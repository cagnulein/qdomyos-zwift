#ifndef SCREENCAPTURE_H
#define SCREENCAPTURE_H

#include <QObject>

class QString;
class QQuickView;

class screenCapture : public QObject {
    Q_OBJECT
  public:
    explicit screenCapture(QQuickView *parent = 0);

  public slots:
    void capture(QString const &path) const;

  private:
    QQuickView *currentView_;
};

#endif // SCREENCAPTURE_H
