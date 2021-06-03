#include <QPixmap>
#include <QQuickView>
#include <QString>

#include "screencapture.h"

screenCapture::screenCapture(QQuickView *currentView) : QObject(0), currentView_(currentView) {}

void screenCapture::capture(QString const &path) const {
    QImage img = currentView_->grabWindow();
    img.save(path);
}
