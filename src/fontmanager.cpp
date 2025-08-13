#include "fontmanager.h"
#include <QDebug>
#include <QCoreApplication>

const QString FontManager::EMOJI_FONT_URL = "https://fonts.gstatic.com/s/notocoloremoji/v15/Yq6P-KqIXTD0t4D9z1ESnKM3-HpFabsE4tq3luCC7p-aXxcn.woff2";
const QString FontManager::EMOJI_FONT_FILENAME = "NotoColorEmoji.woff2";

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
    request.setHeader(QNetworkRequest::UserAgentHeader, "QDomyos-Zwift/1.0");
    
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
            file.write(reply->readAll());
            file.close();
            
            qDebug() << "Emoji font downloaded and cached to:" << cacheFile;
            loadLocalEmojiFont();
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
    int fontId = QFontDatabase::addApplicationFont(fontPath);
    
    if (fontId != -1) {
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
        if (!fontFamilies.isEmpty()) {
            m_emojiFontFamily = fontFamilies.first();
            m_emojiFontReady = true;
            emit emojiFontReadyChanged();
            emit emojiFontFamilyChanged();
            qDebug() << "Cached emoji font loaded:" << m_emojiFontFamily;
            return;
        }
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