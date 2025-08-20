#include "fontmanager.h"
#include <QDebug>
#include <QCoreApplication>

const QString FontManager::EMOJI_FONT_URL = "https://github.com/googlefonts/noto-emoji/raw/main/fonts/NotoColorEmoji.ttf";
const QString FontManager::EMOJI_FONT_FILENAME = "NotoColorEmoji.ttf";

FontManager::FontManager(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_emojiFontReady(false)
    , m_emojiFontFamily("Arial") // fallback
{
}

void FontManager::initializeEmojiFont()
{
#ifdef Q_OS_ANDROID
    QString cacheFile = getCacheFilePath();
    QFile file(cacheFile);
    
    if (file.exists()) {
        // Font già in cache, caricalo
        loadLocalEmojiFont();
    } else {
        // Scarica il font
        downloadEmojiFont();
    }
#else
    // Su desktop/iOS usa il font locale
    QString localFontPath = QCoreApplication::applicationDirPath() + "/fonts/NotoColorEmoji_WindowsCompatible.ttf";
    QFile file(localFontPath);
    
    if (file.exists()) {
        int fontId = QFontDatabase::addApplicationFont(localFontPath);
        if (fontId != -1) {
            QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
            if (!fontFamilies.isEmpty()) {
                m_emojiFontFamily = fontFamilies.first();
                m_emojiFontReady = true;
                emit emojiFontReadyChanged();
                emit emojiFontFamilyChanged();
                qDebug() << "Local emoji font loaded:" << m_emojiFontFamily;
                return;
            }
        }
    }
    
    qWarning() << "Failed to load local emoji font, using fallback";
    m_emojiFontReady = true; // Use fallback
    emit emojiFontReadyChanged();
#endif
}

void FontManager::downloadEmojiFont()
{
    qDebug() << "Downloading emoji font from:" << EMOJI_FONT_URL;
    
    QNetworkRequest request(EMOJI_FONT_URL);
    // Headers per simulare un browser
    request.setHeader(QNetworkRequest::UserAgentHeader, 
                     "Mozilla/5.0 (Linux; Android 10; SM-G960F) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.120 Mobile Safari/537.36");
    request.setRawHeader("Accept", "*/*");
    request.setRawHeader("Accept-Language", "en-US,en;q=0.9");
    request.setRawHeader("Accept-Encoding", "identity"); // No compression
    request.setRawHeader("Connection", "keep-alive");
    request.setRawHeader("Cache-Control", "no-cache");
    
    // Abilita redirect automatici
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &FontManager::onFontDownloadFinished);
}

void FontManager::onFontDownloadFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    reply->deleteLater();
    
    if (reply->error() == QNetworkReply::NoError) {
        QString cacheFile = getCacheFilePath();
        QFile file(cacheFile);
        
        // Crea directory se non esiste
        QDir().mkpath(QFileInfo(cacheFile).dir().absolutePath());
        
        if (file.open(QIODevice::WriteOnly)) {
            QByteArray fontData = reply->readAll();
            
            qDebug() << "Downloaded data size:" << fontData.size() << "bytes";
            qDebug() << "First 100 chars:" << fontData.left(100);
            qDebug() << "First 16 bytes hex:" << fontData.left(16).toHex();
            
            // Validate TTF file (starts with 0x00010000 or "OTTO")
            if (fontData.size() > 4 && 
                (fontData.startsWith(QByteArray::fromHex("00010000")) || fontData.startsWith("OTTO"))) {
                file.write(fontData);
                file.close();
                
                qDebug() << "Emoji font downloaded and cached to:" << cacheFile;
                loadLocalEmojiFont();
            } else {
                file.close();
                // Cancella il file invalido per forzare ri-download
                if (file.exists()) {
                    file.remove();
                    qDebug() << "Removed invalid font file:" << cacheFile;
                }
                qWarning() << "Downloaded file is not a valid TTF font. Size:" << fontData.size();
                if (fontData.size() > 0) {
                    qWarning() << "Content starts with:" << fontData.left(50);
                }
                m_emojiFontReady = true; // Use fallback
                emit emojiFontReadyChanged();
            }
        } else {
            qWarning() << "Failed to save font to cache:" << file.errorString();
            m_emojiFontReady = true; // Use fallback
            emit emojiFontReadyChanged();
        }
    } else {
        qWarning() << "Font download failed:" << reply->errorString();
        m_emojiFontReady = true; // Use fallback
        emit emojiFontReadyChanged();
    }
}

void FontManager::loadLocalEmojiFont()
{
    QString fontPath = getCacheFilePath();
    
    // Debug info sul file
    QFileInfo fileInfo(fontPath);
    qDebug() << "Font file path:" << fontPath;
    qDebug() << "Font file exists:" << fileInfo.exists();
    qDebug() << "Font file size:" << fileInfo.size() << "bytes";
    qDebug() << "Font file readable:" << fileInfo.isReadable();
    
    if (!fileInfo.exists() || fileInfo.size() == 0) {
        qWarning() << "Font file is missing or empty";
        m_emojiFontReady = true;
        emit emojiFontReadyChanged();
        return;
    }
    
    // Verifica i primi bytes del file
    QFile file(fontPath);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray header = file.read(16);
        qDebug() << "Font file header (hex):" << header.toHex();
        qDebug() << "Font file header (text):" << header;
        file.close();
    }
    
    int fontId = QFontDatabase::addApplicationFont(fontPath);
    qDebug() << "QFontDatabase::addApplicationFont returned:" << fontId;
    
    if (fontId != -1) {
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
        qDebug() << "Available font families:" << fontFamilies;
        if (!fontFamilies.isEmpty()) {
            m_emojiFontFamily = fontFamilies.first();
            m_emojiFontReady = true;
            emit emojiFontReadyChanged();
            emit emojiFontFamilyChanged();
            qDebug() << "Cached emoji font loaded:" << m_emojiFontFamily;
            return;
        } else {
            qWarning() << "Font loaded but no families found";
        }
    } else {
        qWarning() << "QFontDatabase::addApplicationFont failed";
        // Cancella il file corrotto
        QFile::remove(fontPath);
        qDebug() << "Removed corrupted font file:" << fontPath;
    }
    
    qWarning() << "Failed to load cached emoji font";
    m_emojiFontReady = true; // Use fallback
    emit emojiFontReadyChanged();
}

QString FontManager::getCacheFilePath() const
{
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    return cacheDir + "/fonts/" + EMOJI_FONT_FILENAME;
}