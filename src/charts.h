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
