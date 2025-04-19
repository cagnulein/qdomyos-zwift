#ifndef CHARTS_H
#define CHARTS_H

#include "mainwindow.h"
#include <QDialog>
#include <QtCharts>

namespace Ui {
class charts;
}

class charts : public QDialog {
    Q_OBJECT

  public:
    explicit charts(MainWindow *parent = nullptr);
    void update();
    ~charts();

  private slots:
    void on_valueOnChart_stateChanged(int arg1);

    void on_speed_clicked();

    void on_Inclination_clicked();

    void on_watt_clicked();

    void on_resistance_clicked();

    void on_heart_clicked();

  private:
    Ui::charts *ui;
    MainWindow *parent = nullptr;

    QChart *chart = nullptr;
    QChartView *chart_view = nullptr;
    QLineSeries *chart_series_speed = nullptr;
    QLineSeries *chart_series_inclination = nullptr;
    QLineSeries *chart_series_heart = nullptr;
    QLineSeries *chart_series_watt = nullptr;
    QLineSeries *chart_series_resistance = nullptr;
    QLineSeries *chart_series_pace = nullptr;
};

#endif // CHARTS_H
