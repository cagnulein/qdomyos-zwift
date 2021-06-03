#ifndef TEMPLATEINFOSENDER_H
#define TEMPLATEINFOSENDER_H
#include <QJSEngine>
#include <QObject>
#include <QSettings>
#include <QTimer>

class TemplateInfoSender : public QObject {
    Q_OBJECT
  public:
    TemplateInfoSender(const QString &id, QObject *parent = 0);
    virtual ~TemplateInfoSender();
    virtual bool isRunning() const = 0;
    virtual bool send(const QString &data) = 0;
    bool init(const QString &script);
    void stop();
    bool update(QJSEngine *eng);
    QString js() const;
    QString getId() const;
  signals:
    void onDataReceived(QByteArray data);

  protected:
    virtual bool init() = 0;
    virtual void innerStop();
    QString templateId;
    QSettings settings;
    QString jscript;
  protected slots:
    void reinit();

  private:
    QTimer retryTimer;
};

#endif // TEMPLATEINFOSENDER_H
