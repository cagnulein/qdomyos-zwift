#ifndef CHARTS_H
#define CHARTS_H

#include <QDialog>
#include <QtCharts>
#include "mainwindow.h"

namespace Ui {
class charts;
}

class charts : public QDialog
{
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
    MainWindow* parent = 0;

    QtCharts::QChart* chart = 0;
    QtCharts::QChartView* chart_view = 0;
    QtCharts::QLineSeries* chart_series_speed = 0;
    QtCharts::QLineSeries* chart_series_inclination = 0;
    QtCharts::QLineSeries* chart_series_heart = 0;
    QtCharts::QLineSeries* chart_series_watt = 0;
    QtCharts::QLineSeries* chart_series_resistance = 0;
};

#endif // CHARTS_H
