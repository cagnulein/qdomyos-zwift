#include "charts.h"
#include "ui_charts.h"

charts::charts(MainWindow *parent) :
    QDialog(parent),
    ui(new Ui::charts)
{
    ui->setupUi(this);
    this->parent = parent;

    chart = new QtCharts::QChart();
    chart_view = new QtCharts::QChartView(chart, ui->widget);
    ui->widget->setVisible(false);

    chart_series_speed = new QtCharts::QLineSeries();
    chart_series_inclination = new QtCharts::QLineSeries();
    chart_series_heart = new QtCharts::QLineSeries();
    chart_series_watt = new QtCharts::QLineSeries();
    chart_series_resistance = new QtCharts::QLineSeries();

    chart_series_speed->setPointLabelsVisible(true);    // is false by default
    chart_series_speed->setPointLabelsColor(Qt::black);
    chart_series_speed->setPointLabelsFormat("@yPoint km/h");
    chart_series_inclination->setPointLabelsVisible(true);    // is false by default
    chart_series_inclination->setPointLabelsColor(Qt::black);
    chart_series_inclination->setPointLabelsFormat("@yPoint%");
    chart_series_speed->setName("Speed (km/h)");
    chart_series_inclination->setName("Inclination (%)");

    chart->legend()->setAlignment(Qt::AlignBottom);
    chart_view->setRenderHint(QPainter::Antialiasing);
    chart_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    ui->frame->layout()->addWidget(chart_view);
}

void charts::update()
{
    for(int i=0; i<parent->Session.length(); i++)
    {
        if(chart->series().count())
        {
            chart->removeSeries(chart_series_speed);
            chart->removeSeries(chart_series_inclination);
        }
        chart_series_inclination->clear();
        chart_series_speed->clear();
        const int maxQueue = 100;
        for(int g=0; g<(parent->Session.count() > maxQueue ? maxQueue : parent->Session.count()); g++)
        {
            int index = g + (parent->Session.count() > maxQueue ? parent->Session.count() % maxQueue : 0);
            chart_series_inclination->append(g, static_cast<double>(parent->Session[index].inclination));
        }

        for(int g=0; g<(parent->Session.count() > maxQueue ? maxQueue : parent->Session.count()); g++)
        {
            int index = g + (parent->Session.count() > maxQueue ? parent->Session.count() % maxQueue : 0);

            chart_series_speed->append(g, static_cast<qreal>(parent->Session[index].speed));
        }

        chart->addSeries(chart_series_inclination);
        chart->addSeries(chart_series_speed);
        chart->createDefaultAxes();
    }
}

charts::~charts()
{
    delete ui;
}
