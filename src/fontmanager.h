#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFontDatabase>
#include <QStandardPaths>
#include <QDir>
#include <QFile>

class FontManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool emojiFontReady READ isEmojiFontReady NOTIFY emojiFontReadyChanged)
    Q_PROPERTY(QString emojiFontFamily READ emojiFontFamily NOTIFY emojiFontFamilyChanged)

public:
    explicit FontManager(QObject *parent = nullptr);
    
    bool isEmojiFontReady() const { return m_emojiFontReady; }
    QString emojiFontFamily() const { return m_emojiFontFamily; }
    
    Q_INVOKABLE void initializeEmojiFont();

signals:
    void emojiFontReadyChanged();
    void emojiFontFamilyChanged();

private slots:
    void onFontDownloadFinished();

private:
    void loadLocalEmojiFont();
    void downloadEmojiFont();
    QString getCacheFilePath() const;
    
    QNetworkAccessManager *m_networkManager;
    bool m_emojiFontReady;
    QString m_emojiFontFamily;
    
    static const QString EMOJI_FONT_URL;
    static const QString EMOJI_FONT_FILENAME;
};

#endif // FONTMANAGER_H