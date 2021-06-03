#include "templateinfosender.h"
#include <QDebug>

TemplateInfoSender::TemplateInfoSender(const QString &id, QObject *parent) : QObject(parent), templateId(id) {
    connect(&retryTimer, &QTimer::timeout, this, [this]() {
        Q_UNUSED(this);
        init();
    });
    retryTimer.setSingleShot(true);
}

TemplateInfoSender::~TemplateInfoSender() { stop(); }

bool TemplateInfoSender::init(const QString &script) {
    jscript = script;
    stop();
    return init();
}

bool TemplateInfoSender::update(QJSEngine *eng) {
    if (!jscript.isEmpty()) {
        QJSValue jsv = eng->evaluate(jscript);
        if (!jsv.isError()) {
            QString evalres = jsv.toString();
            qDebug() << "eval res " << evalres;
            return send(evalres);
        } else {
#if (QT_VERSION < QT_VERSION_CHECK(5, 12, 0))
            int errorType = 255;
#else
            int errorType = jsv.errorType();
#endif
            qDebug() << "Scripts contains an error:" << jscript << "error" << errorType;
            return false;
        }
    } else
        return false;
}

QString TemplateInfoSender::js() const { return jscript; }

QString TemplateInfoSender::getId() const { return templateId; }

void TemplateInfoSender::stop() {
    retryTimer.stop();
    innerStop();
}

void TemplateInfoSender::innerStop() {}

void TemplateInfoSender::reinit() {
    stop();
    retryTimer.start(5000);
}
