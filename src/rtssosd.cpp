#include "rtssosd.h"

#ifdef Q_OS_WIN

#include <QByteArray>
#include <QDateTime>
#include <QDebug>
#include <windows.h>

namespace {

const wchar_t *kMapName = L"RTSSSharedMemoryV2";

// RTSS_SHARED_MEMORY header, nine DWORDs:
//   dwSignature, dwVersion, dwAppEntrySize, dwAppArrOffset, dwAppArrSize,
//   dwOSDEntrySize, dwOSDArrOffset, dwOSDArrSize, dwOSDFrame
constexpr int kHdrSignature = 0;
constexpr int kHdrVersion = 1;
constexpr int kHdrOsdEntrySize = 5;
constexpr int kHdrOsdArrOffset = 6;
constexpr int kHdrOsdArrSize = 7;
constexpr int kHdrOsdFrame = 8;

// 'RTSS' as an MSVC multi-char constant, and the order it actually lands in
// memory. Accepting both avoids depending on how the constant was formed.
constexpr quint32 kSignatureA = 0x52545353;
constexpr quint32 kSignatureB = 0x53535452;

constexpr quint32 kMinVersion = 0x00020000;

// RTSS_SHARED_MEMORY_OSD_ENTRY
constexpr int kOsdTextLen = 256;   // szOSD
constexpr int kOsdOwnerOff = 256;  // szOSDOwner
constexpr int kOsdOwnerLen = 256;
constexpr int kOsdExOff = 512;     // szOSDEx, present once entries are big enough
constexpr int kOsdExLen = 4096;

// Only worth one attempt every few seconds: RTSS is usually started after QZ.
constexpr qint64 kRetryMs = 5000;

const char *kOwner = "QZ";

quint32 readDword(const unsigned char *base, int index) {
    quint32 v = 0;
    memcpy(&v, base + index * sizeof(quint32), sizeof(quint32));
    return v;
}

void writeDword(unsigned char *base, int index, quint32 value) {
    memcpy(base + index * sizeof(quint32), &value, sizeof(quint32));
}

} // namespace

RtssOsd::~RtssOsd() { release(); }

bool RtssOsd::headerLooksValid() const {
    if (!view) {
        return false;
    }
    const quint32 signature = readDword(view, kHdrSignature);
    return signature == kSignatureA || signature == kSignatureB;
}

bool RtssOsd::attach() {
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now - lastAttachAttemptMs < kRetryMs) {
        return false;
    }
    lastAttachAttemptMs = now;

    HANDLE h = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, kMapName);
    if (!h) {
        return false; // RTSS not running; nothing to say about it
    }

    auto *v = static_cast<unsigned char *>(MapViewOfFile(h, FILE_MAP_ALL_ACCESS, 0, 0, 0));
    if (!v) {
        CloseHandle(h);
        return false;
    }

    handle = h;
    view = v;

    if (!headerLooksValid()) {
        qDebug() << "RtssOsd: shared memory signature not recognised, ignoring RTSS";
        detach();
        return false;
    }

    const quint32 version = readDword(view, kHdrVersion);
    if (version < kMinVersion) {
        qDebug() << "RtssOsd: shared memory version" << Qt::hex << version << "is older than 2.0, ignoring RTSS";
        detach();
        return false;
    }

    osdEntrySize = readDword(view, kHdrOsdEntrySize);
    osdArrOffset = readDword(view, kHdrOsdArrOffset);
    osdArrSize = readDword(view, kHdrOsdArrSize);

    // An entry has to at least hold szOSD and szOSDOwner for any of this to mean
    // anything, and the array has to sit somewhere sane.
    if (osdEntrySize < static_cast<quint32>(kOsdOwnerOff + kOsdOwnerLen) || osdArrSize == 0 || osdArrOffset == 0) {
        qDebug() << "RtssOsd: implausible OSD array geometry" << osdEntrySize << osdArrOffset << osdArrSize;
        detach();
        return false;
    }

    hasOsdEx = osdEntrySize >= static_cast<quint32>(kOsdExOff + kOsdExLen);

    // Reuse our own slot if one is already there - a previous run that was killed
    // rather than closed leaves it claimed - otherwise take the first free one.
    int freeSlot = -1;
    for (quint32 i = 0; i < osdArrSize; ++i) {
        unsigned char *owner = view + osdArrOffset + i * osdEntrySize + kOsdOwnerOff;
        const QByteArray name = QByteArray(reinterpret_cast<const char *>(owner),
                                           qstrnlen(reinterpret_cast<const char *>(owner), kOsdOwnerLen));
        if (name == kOwner) {
            slot = static_cast<int>(i);
            break;
        }
        if (name.isEmpty() && freeSlot < 0) {
            freeSlot = static_cast<int>(i);
        }
    }

    if (slot < 0) {
        if (freeSlot < 0) {
            qDebug() << "RtssOsd: every RTSS OSD slot is taken";
            detach();
            return false;
        }
        slot = freeSlot;
        unsigned char *owner = view + osdArrOffset + slot * osdEntrySize + kOsdOwnerOff;
        SecureZeroMemory(owner, kOsdOwnerLen);
        memcpy(owner, kOwner, strlen(kOwner));
    }

    qDebug() << "RtssOsd: attached to RTSS, slot" << slot << "entry size" << osdEntrySize << "szOSDEx" << hasOsdEx;
    return true;
}

void RtssOsd::detach() {
    if (view) {
        UnmapViewOfFile(view);
        view = nullptr;
    }
    if (handle) {
        CloseHandle(handle);
        handle = nullptr;
    }
    slot = -1;
    lastPublished.clear();
}

void RtssOsd::publish(const QString &text) {
    if (!view && !attach()) {
        return;
    }

    // RTSS closing and restarting leaves us mapped onto a block it no longer
    // owns, so the signature is checked every time rather than only on attach.
    if (!headerLooksValid()) {
        qDebug() << "RtssOsd: RTSS shared memory went away, will re-attach";
        detach();
        return;
    }

    if (text == lastPublished) {
        return; // nothing changed, leave dwOSDFrame alone
    }
    lastPublished = text;

    const QByteArray payload = text.toLatin1();
    unsigned char *entry = view + osdArrOffset + slot * osdEntrySize;

    SecureZeroMemory(entry, kOsdTextLen);
    memcpy(entry, payload.constData(), qMin<int>(payload.size(), kOsdTextLen - 1));

    if (hasOsdEx) {
        SecureZeroMemory(entry + kOsdExOff, kOsdExLen);
        memcpy(entry + kOsdExOff, payload.constData(), qMin<int>(payload.size(), kOsdExLen - 1));
    }

    writeDword(view, kHdrOsdFrame, readDword(view, kHdrOsdFrame) + 1);
}

void RtssOsd::release() {
    if (view && slot >= 0 && headerLooksValid()) {
        // Clear the whole entry, owner included: leaving it claimed would keep
        // the last gear frozen on screen and burn a slot for the next app.
        unsigned char *entry = view + osdArrOffset + slot * osdEntrySize;
        SecureZeroMemory(entry, osdEntrySize);
        writeDword(view, kHdrOsdFrame, readDword(view, kHdrOsdFrame) + 1);
    }
    detach();
}

#else

RtssOsd::~RtssOsd() = default;
void RtssOsd::publish(const QString &text) { Q_UNUSED(text) }
void RtssOsd::release() {}

#endif // Q_OS_WIN
