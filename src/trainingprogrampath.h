#ifndef TRAININGPROGRAMPATH_H
#define TRAININGPROGRAMPATH_H

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>

namespace TrainingProgramPath {

// Preserve existing paths on every platform. Rebase an obsolete iOS sandbox path
// only when its XML still exists below the current Documents/training directory.
// The caller enables recovery only on iOS; other platforms never search elsewhere.
inline QString resolve(const QString &storedPath, const QString &currentDocumentsDir, bool recoverIosContainer) {
    if (storedPath.isEmpty()) {
        return QString();
    }
    if (QFile::exists(storedPath)) {
        return storedPath;
    }
    if (!recoverIosContainer) {
        return QString();
    }

    const QString normalized = QDir::fromNativeSeparators(storedPath);
    const QString marker = QStringLiteral("/Documents/training/");
    const int markerIndex = normalized.lastIndexOf(marker);
    if (!normalized.startsWith(QLatin1Char('/')) || markerIndex < 0 ||
        !normalized.left(markerIndex).contains(QStringLiteral("/Application/"))) {
        // Never reinterpret arbitrary local or non-iOS paths as a training XML.
        return QString();
    }

    const QString relativePath = normalized.mid(markerIndex + QStringLiteral("/Documents/").size());
    const QString currentRoot = QDir(currentDocumentsDir).filePath(QStringLiteral("training"));
    const QString candidate = QDir(currentDocumentsDir).filePath(relativePath);

    // Use canonical paths to reject ../ escapes and symlinks leaving the training
    // directory; do not load a different file merely because its name matches.
    const QString canonicalRoot = QFileInfo(currentRoot).canonicalFilePath();
    const QString canonicalFile = QFileInfo(candidate).canonicalFilePath();
    if (canonicalRoot.isEmpty() || canonicalFile.isEmpty() ||
        !canonicalFile.startsWith(canonicalRoot + QLatin1Char('/')) ||
        !QFileInfo(canonicalFile).isFile()) {
        return QString();
    }
    return canonicalFile;
}

} // namespace TrainingProgramPath

#endif // TRAININGPROGRAMPATH_H
