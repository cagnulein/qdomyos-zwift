#include "charts.h"
#include "ui_charts.h"

charts::charts(MainWindow *parent) : QDialog(parent), ui(new Ui::charts) {
    ui->setupUi(this);
    this->parent = parent;

    chart = new QChart();
    chart_view = new QChartView(chart, ui->widget);
    ui->widget->setVisible(false);

    chart_series_speed = new QLineSeries();
    chart_series_pace = new QLineSeries();
    chart_series_inclination = new QLineSeries();
    chart_series_heart = new QLineSeries();
    chart_series_watt = new QLineSeries();
    chart_series_resistance = new QLineSeries();

    chart_series_speed->setPointLabelsVisible(false); // is false by default
    chart_series_speed->setPointLabelsColor(Qt::black);
    chart_series_speed->setPointLabelsFormat(QStringLiteral("@yPoint km/h"));
    chart_series_pace->setPointLabelsVisible(false); // is false by default
    chart_series_pace->setPointLabelsColor(Qt::black);
    chart_series_pace->setPointLabelsFormat(QStringLiteral("@yPoint min/km"));
    chart_series_inclination->setPointLabelsVisible(false); // is false by default
    chart_series_inclination->setPointLabelsColor(Qt::black);
    chart_series_inclination->setPointLabelsFormat(QStringLiteral("@yPoint%"));
    chart_series_heart->setPointLabelsVisible(false); // is false by default
    chart_series_heart->setPointLabelsColor(Qt::black);
    chart_series_heart->setPointLabelsFormat(QStringLiteral("@yPoint bpm"));
    chart_series_watt->setPointLabelsVisible(false); // is false by default
    chart_series_watt->setPointLabelsColor(Qt::black);
    chart_series_watt->setPointLabelsFormat(QStringLiteral("@yPoint W"));
    chart_series_resistance->setPointLabelsVisible(false); // is false by default
    chart_series_resistance->setPointLabelsColor(Qt::black);
    chart_series_resistance->setPointLabelsFormat(QStringLiteral("@yPoint lvl"));
    chart_series_speed->setName(QStringLiteral("Speed (km/h)"));
    chart_series_pace->setName(QStringLiteral("Pace (min/km)"));
    chart_series_inclination->setName(QStringLiteral("Inclination (%)"));
    chart_series_heart->setName(QStringLiteral("Heart (bpm)"));
    chart_series_watt->setName(QStringLiteral("Watt (W)"));
    chart_series_resistance->setName(QStringLiteral("Resistance (lvl)"));

    chart->legend()->setAlignment(Qt::AlignBottom);
    chart_view->setRenderHint(QPainter::Antialiasing);
    chart_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    ui->frame->layout()->addWidget(chart_view);
}

void charts::update() {
    if (!chart->series().isEmpty()) {
        if (ui->speed->isChecked()) {
            chart->removeSeries(chart_series_speed);
        }
        if (ui->pace->isChecked()) {
            chart->removeSeries(chart_series_pace);
        }
        if (ui->inclination->isChecked()) {
            chart->removeSeries(chart_series_inclination);
        }
        if (ui->heart->isChecked()) {
            chart->removeSeries(chart_series_heart);
        }
        if (ui->watt->isChecked()) {
            chart->removeSeries(chart_series_watt);
        }
        if (ui->resistance->isChecked()) {
            chart->removeSeries(chart_series_resistance);
        }
    }
    chart_series_inclination->clear();
    chart_series_speed->clear();
    chart_series_pace->clear();
    chart_series_heart->clear();
    chart_series_watt->clear();
    chart_series_resistance->clear();
    const int maxQueue = 100;

    for (int g = 0; g < (parent->Session.count() > maxQueue ? maxQueue : parent->Session.count()); g++) {
        int index = g + (parent->Session.count() > maxQueue ? parent->Session.count() % maxQueue : 0);
        if (ui->inclination->isChecked()) {
            chart_series_inclination->append(g, static_cast<double>(parent->Session[index].inclination));
        }
        if (ui->speed->isChecked()) {
            chart_series_speed->append(g, static_cast<qreal>(parent->Session[index].speed));
        }
        if (ui->pace->isChecked()) {
            chart_series_pace->append(g, static_cast<qreal>(parent->Session[index].pace));
        }
        if (ui->heart->isChecked()) {
            chart_series_heart->append(g, static_cast<qreal>(parent->Session[index].heart));
        }
        if (ui->watt->isChecked()) {
            chart_series_watt->append(g, static_cast<qreal>(parent->Session[index].watt));
        }
        if (ui->resistance->isChecked()) {
            chart_series_resistance->append(g, static_cast<qreal>(parent->Session[index].resistance));
        }
    }

    if (ui->inclination->isChecked()) {
        chart->addSeries(chart_series_inclination);
    }
    if (ui->speed->isChecked()) {
        chart->addSeries(chart_series_speed);
    }
    if (ui->pace->isChecked()) {
        chart->addSeries(chart_series_pace);
    }
    if (ui->heart->isChecked()) {
        chart->addSeries(chart_series_heart);
    }
    if (ui->watt->isChecked()) {
        chart->addSeries(chart_series_watt);
    }
    if (ui->resistance->isChecked()) {
        chart->addSeries(chart_series_resistance);
    }
    chart->createDefaultAxes();
}

charts::~charts() { delete ui; }

void charts::on_valueOnChart_stateChanged(int arg1) {
    Q_UNUSED(arg1);

    if (ui->valueOnChart->checkState() == Qt::Checked) {
        chart_series_speed->setPointLabelsVisible(true);
        chart_series_pace->setPointLabelsVisible(true);
        chart_series_inclination->setPointLabelsVisible(true); // is false by default
        chart_series_heart->setPointLabelsVisible(true);       // is false by default
        chart_series_watt->setPointLabelsVisible(true);        // is false by default
        chart_series_resistance->setPointLabelsVisible(true);  // is false by default
    } else {
        chart_series_speed->setPointLabelsVisible(false);
        chart_series_pace->setPointLabelsVisible(false);
        chart_series_inclination->setPointLabelsVisible(false); // is false by default
        chart_series_heart->setPointLabelsVisible(false);       // is false by default
        chart_series_watt->setPointLabelsVisible(false);        // is false by default
        chart_series_resistance->setPointLabelsVisible(false);  // is false by default
    }
}

void charts::on_speed_clicked() {}

void charts::on_Inclination_clicked() {}

void charts::on_watt_clicked() {}

void charts::on_resistance_clicked() {}

void charts::on_heart_clicked() {}
