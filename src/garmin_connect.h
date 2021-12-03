#ifndef GARMIN_CONNECT_H
#define GARMIN_CONNECT_H

#include <QNetworkAccessManager>

class garmin_connect : public QObject {

    Q_OBJECT
  public:
    garmin_connect();

  private:
    QNetworkAccessManager *manager = nullptr;
};

#endif // GARMIN_CONNECT_H
