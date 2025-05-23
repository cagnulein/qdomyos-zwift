#include "mainwindow.h"
#include "charts.h"
#include "gpx.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <chrono>

using namespace std::chrono_literals;

charts *Charts = nullptr;

void MainWindow::load(bluetooth *b) {
    ui->setupUi(this);

    this->bluetoothManager = b;
    connect(this->bluetoothManager, &bluetooth::deviceConnected, this, &MainWindow::trainProgramSignals);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::update);
    timer->start(1s);

#ifdef Q_OS_ANDROID
    ui->groupTrain->setVisible(false);
    ui->fanBar->setVisible(false);
    ui->fanSpeedMinus->setVisible(false);
    ui->fanSpeedPlus->setVisible(false);
    ui->weight->setVisible(false);
    ui->weightDescription->setVisible(false);
    ui->weightIcon->setVisible(false);
    ui->load->setVisible(false);
    ui->reset->setVisible(false);
    ui->save->setVisible(false);
#endif

    update();
}

MainWindow::MainWindow(bluetooth *b) : QDialog(nullptr), ui(new Ui::MainWindow) {

    load(b);
    this->trainProgram = new trainprogram(QList<trainrow>(), b);
}

MainWindow::MainWindow(bluetooth *b, const QString &trainProgram) : QDialog(nullptr), ui(new Ui::MainWindow) {

    load(b);
    loadTrainProgram(trainProgram);
}

void MainWindow::update() {
    if (bluetoothManager->device()) {

        uint8_t cadence = 0;
        double inclination = 0;
        double resistance = 0;
        double peloton_resistance = 0;
        double watts = 0;
        double pace = 0;
        uint32_t totalStrokes = 0;
        double avgStrokesRate = 0;
        double maxStrokesRate = 0;
        double avgStrokesLength = 0;
        double strideLength = 0;
        double groundContact = 0;
        double verticalOscillation = 0;
        double stepCount = 0;

        ui->speed->setText(QString::number(bluetoothManager->device()->currentSpeed().value(), 'f', 2));
        ui->heartrate->setText(QString::number(bluetoothManager->device()->currentHeart().value()));
        ui->odometer->setText(QString::number(bluetoothManager->device()->odometer(), 'f', 2));
        ui->calories->setText(QString::number(bluetoothManager->device()->calories().value(), 'f', 0));
        ui->fanBar->setValue(bluetoothManager->device()->fanSpeed());

        if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
            if (bluetoothManager->device()->currentSpeed().value()) {
                pace = 10000 / (((treadmill *)bluetoothManager->device())->currentPace().second() +
                                (((treadmill *)bluetoothManager->device())->currentPace().minute() * 60));
                if (pace < 0) {
                    pace = 0;
                }
            } else {

                pace = 0;
            }
            strideLength = ((treadmill *)bluetoothManager->device())->currentStrideLength().value();
            groundContact = ((treadmill *)bluetoothManager->device())->currentGroundContact().value();
            verticalOscillation = ((treadmill *)bluetoothManager->device())->currentVerticalOscillation().value();
            stepCount = ((treadmill *)bluetoothManager->device())->currentStepCount().value();
            watts = ((treadmill *)bluetoothManager->device())->watts(ui->weight->text().toFloat());
            inclination = ((treadmill *)bluetoothManager->device())->currentInclination().value();
            ui->pace->setText(
                ((treadmill *)bluetoothManager->device())->currentPace().toString(QStringLiteral("m:ss")));
            ui->watt->setText(QString::number(watts, 'f', 0));
            ui->inclination->setText(QString::number(inclination, 'f', 1));
            ui->elevationGain->setText(
                QString::number(((treadmill *)bluetoothManager->device())->elevationGain().value(), 'f', 1));
        } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {

            cadence = ((bike *)bluetoothManager->device())->currentCadence().value();
            resistance = ((bike *)bluetoothManager->device())->currentResistance().value();
            watts = ((bike *)bluetoothManager->device())->watts();
            ui->watt->setText(QString::number(watts));
            ui->resistance->setText(QString::number(resistance));
            ui->cadence->setText(QString::number(cadence));
        } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {

            cadence = ((rower *)bluetoothManager->device())->currentCadence().value();
            resistance = ((rower *)bluetoothManager->device())->currentResistance().value();
            watts = ((rower *)bluetoothManager->device())->watts();
            ui->watt->setText(QString::number(watts));
            ui->resistance->setText(QString::number(resistance));
            ui->cadence->setText(QString::number(cadence));
        } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {

            cadence = ((elliptical *)bluetoothManager->device())->currentCadence().value();
            resistance = ((elliptical *)bluetoothManager->device())->currentResistance().value();
            watts = ((elliptical *)bluetoothManager->device())->watts();
            ui->watt->setText(QString::number(watts));
            ui->resistance->setText(QString::number(resistance));
            ui->cadence->setText(QString::number(cadence));
        }

        if (trainProgram) {
            ui->trainProgramElapsedTime->setText(trainProgram->totalElapsedTime().toString(QStringLiteral("hh:mm:ss")));
            ui->trainProgramCurrentRowElapsedTime->setText(
                trainProgram->currentRowElapsedTime().toString(QStringLiteral("hh:mm:ss")));
            ui->trainProgramDuration->setText(trainProgram->duration().toString(QStringLiteral("hh:mm:ss")));

            double distance = trainProgram->totalDistance();
            if (distance > 0) {

                ui->trainProgramTotalDistance->setText(QString::number(distance));
            } else {
                ui->trainProgramTotalDistance->setText(QStringLiteral("N/A"));
            }
        }

        if (bluetoothManager->device()->connected()) {

            ui->connectionToTreadmill->setEnabled(true);
            if (bluetoothManager->device()->VirtualDevice()) {

                if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL &&
                    ((virtualtreadmill *)((treadmill *)bluetoothManager->device())->VirtualDevice())->connected()) {

                    ui->connectionToZwift->setEnabled(true);
                } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE &&
                           ((virtualbike *)((bike *)bluetoothManager->device())->VirtualDevice())->connected()) {

                    ui->connectionToZwift->setEnabled(true);
                } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING &&
                           ((virtualbike *)((rower *)bluetoothManager->device())->VirtualDevice())->connected()) {

                    ui->connectionToZwift->setEnabled(true);
                } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL &&
                           ((virtualtreadmill *)((elliptical *)bluetoothManager->device())->VirtualDevice())
                               ->connected()) {

                    ui->connectionToZwift->setEnabled(true);
                } else {

                    ui->connectionToZwift->setEnabled(false);
                }
            } else {
                ui->connectionToZwift->setEnabled(false);
            }
        } else {
            ui->connectionToTreadmill->setEnabled(false);
        }

        SessionLine s(bluetoothManager->device()->currentSpeed().value(), inclination,
                      bluetoothManager->device()->odometer(), watts, resistance, peloton_resistance,
                      (uint8_t)bluetoothManager->device()->currentHeart().value(), pace, cadence,
                      bluetoothManager->device()->calories().value(),
                      bluetoothManager->device()->elevationGain().value(),
                      bluetoothManager->device()->elapsedTime().second() +
                          (bluetoothManager->device()->elapsedTime().minute() * 60) +
                          (bluetoothManager->device()->elapsedTime().hour() * 3600),

                      false, totalStrokes, avgStrokesRate, maxStrokesRate, avgStrokesLength,
                      bluetoothManager->device()->currentCordinate(), strideLength, groundContact,
                      verticalOscillation, stepCount,
                      bluetoothManager->device()->CoreBodyTemperature.value(), bluetoothManager->device()->SkinTemperature.value(),
                      bluetoothManager->device()->HeatStrainIndex.value() // TODO add lap
        );

        Session.append(s);

        if (ui->chart->isChecked()) {
            if (!Charts) {

                Charts = new charts(this);
                Charts->show();
            }
            Charts->update();
        }
    } else {

        ui->connectionToTreadmill->setEnabled(false);
        ui->connectionToZwift->setEnabled(false);

        /*
         * DEBUG CHARTS
         *

        if(!Charts)
        {
            Charts = new charts(this);
            Charts->show();
        }

        SessionLine s(
                      (double)QRandomGenerator::global()->bounded(22),
                      QRandomGenerator::global()->bounded(15),
                      (double)QRandomGenerator::global()->bounded(15),
                      QRandomGenerator::global()->bounded(150),
                      0,
                      QRandomGenerator::global()->bounded(180));

        Session.append(s);
        Charts->update();*/
    }
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::addEmptyRow() {

    int row = ui->tableWidget->rowCount();
    editing = true;
    ui->tableWidget->insertRow(row);
    ui->tableWidget->setItem(row, 0, new QTableWidgetItem(QStringLiteral("00:00:00")));
    ui->tableWidget->setItem(row, 1, new QTableWidgetItem(QStringLiteral("10")));
    ui->tableWidget->setItem(row, 2, new QTableWidgetItem(QStringLiteral("0")));
    ui->tableWidget->setItem(row, 3, new QTableWidgetItem(QLatin1String("")));
    ui->tableWidget->item(row, 0)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidget->item(row, 1)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidget->item(row, 2)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidget->item(row, 3)->setCheckState(Qt::CheckState::Checked);
    editing = false;
}

void MainWindow::on_tableWidget_cellChanged(int row, int column) {
    if (editing)
        return;
    if (column == 0) {
        switch (ui->tableWidget->currentItem()->text().length()) {

        case 4:
            ui->tableWidget->currentItem()->setText(QStringLiteral("00:0") + ui->tableWidget->currentItem()->text());
            break;
        case 5:
            ui->tableWidget->currentItem()->setText(QStringLiteral("00:") + ui->tableWidget->currentItem()->text());
            break;
        case 7:
            ui->tableWidget->currentItem()->setText(QStringLiteral("0") + ui->tableWidget->currentItem()->text());
            break;
        }
        // QString fmt = QStringLiteral("hh:mm:ss");//NOTE: clazy-unused-non-trivial-variable
        QTime dt = QTime::fromString(ui->tableWidget->currentItem()->text());
        QString timeStr = dt.toString(QStringLiteral("hh:mm:ss"));
        ui->tableWidget->currentItem()->setText(timeStr);
    }

    if (row + 1 == ui->tableWidget->rowCount() && !ui->tableWidget->currentItem()->text().isEmpty()) {
        addEmptyRow();
    }

    QList<trainrow> rows;
    for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        if (!ui->tableWidget->item(i, 0)->text().contains(QStringLiteral("00:00:00"))) {

            trainrow t;
            t.duration = QTime::fromString(ui->tableWidget->item(i, 0)->text(), QStringLiteral("hh:mm:ss"));
            t.speed = ui->tableWidget->item(i, 1)->text().toFloat();
            t.inclination = ui->tableWidget->item(i, 2)->text().toFloat();
            t.forcespeed = ui->tableWidget->item(i, 3)->checkState() == Qt::CheckState::Checked;
            rows.append(t);
        } else {

            break;
        }
        createTrainProgram(rows);
    }
}

void MainWindow::trainProgramSignals() {
    if (bluetoothManager->device()) {
        disconnect(trainProgram, &trainprogram::start, bluetoothManager->device(), &bluetoothdevice::start);
        disconnect(trainProgram, &trainprogram::stop, bluetoothManager->device(), &bluetoothdevice::stop);
        disconnect(trainProgram, &trainprogram::changeSpeed, ((treadmill *)bluetoothManager->device()),
                   &treadmill::changeSpeed);
        disconnect(trainProgram, &trainprogram::changeInclination, ((treadmill *)bluetoothManager->device()),
                   &treadmill::changeInclination);
        disconnect(trainProgram, &trainprogram::changeSpeedAndInclination, ((treadmill *)bluetoothManager->device()),
                   &treadmill::changeSpeedAndInclination);
        disconnect(trainProgram, &trainprogram::changeResistance, ((bike *)bluetoothManager->device()),
                   &bike::changeResistance);
        disconnect(((treadmill *)bluetoothManager->device()), &treadmill::tapeStarted, trainProgram,
                   &trainprogram::onTapeStarted);
        disconnect(((bike *)bluetoothManager->device()), &bike::bikeStarted, trainProgram,
                   &trainprogram::onTapeStarted);

        connect(trainProgram, &trainprogram::start, bluetoothManager->device(), &bluetoothdevice::start);
        connect(trainProgram, &trainprogram::stop, bluetoothManager->device(), &bluetoothdevice::stop);
        connect(trainProgram, &trainprogram::changeSpeed, ((treadmill *)bluetoothManager->device()),
                &treadmill::changeSpeed);
        connect(trainProgram, &trainprogram::changeInclination, ((treadmill *)bluetoothManager->device()),
                &treadmill::changeInclination);
        connect(trainProgram, &trainprogram::changeSpeedAndInclination, ((treadmill *)bluetoothManager->device()),
                &treadmill::changeSpeedAndInclination);
        connect(trainProgram, &trainprogram::changeResistance, ((bike *)bluetoothManager->device()),
                &bike::changeResistance);
        connect(((treadmill *)bluetoothManager->device()), &treadmill::tapeStarted, trainProgram,
                &trainprogram::onTapeStarted);
        connect(((bike *)bluetoothManager->device()), &bike::bikeStarted, trainProgram, &trainprogram::onTapeStarted);

        qDebug() << QStringLiteral("trainProgram associated to a device");
    } else {
        qDebug() << QStringLiteral("trainProgram NOT associated to a device");
    }
}

void MainWindow::createTrainProgram(const QList<trainrow> &rows) {
    if (trainProgram) {
        delete trainProgram;
    }
    trainProgram = new trainprogram(rows, bluetoothManager);
    if (rows.length() == 0) {
        addEmptyRow();
    }
    trainProgramSignals();
}

void MainWindow::on_tableWidget_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous) {

    Q_UNUSED(current);
    Q_UNUSED(previous);
}

void MainWindow::on_save_clicked() {
    QString fileName =
        QFileDialog::getSaveFileName(this, tr("Save File"), QStringLiteral("train.xml"), tr("Train Program (*.xml)"));
    if (!fileName.isEmpty() && trainProgram) {

        trainProgram->save(fileName);
    }
}

void MainWindow::loadTrainProgram(const QString &fileName) {
    if (!fileName.isEmpty()) {

        ui->difficulty->setValue(50);
        int rows = ui->tableWidget->rowCount();
        for (int i = 0; i < rows; i++) {
            ui->tableWidget->removeRow(ui->tableWidget->rowCount() - 1);
        }

        if (fileName.endsWith(QStringLiteral("xml"))) {
            if (trainProgram) {

                delete trainProgram;
            }
            trainProgram = trainprogram::load(fileName, bluetoothManager, fileName.right(3).toUpper());
        } else if (fileName.endsWith(QStringLiteral("gpx"))) {
            if (trainProgram) {
                delete trainProgram;
            }
            gpx g;
            QList<trainrow> list;
            auto g_list = g.open(fileName, bluetoothManager->device() ? bluetoothManager->device()->deviceType() : bluetoothdevice::BIKE);
            list.reserve(g_list.count() + 1);
            for (const auto &p : qAsConst(g_list)) {
                trainrow r;
                r.speed = p.speed;
                r.duration = QTime(0, 0, 0, 0);
                r.duration = r.duration.addSecs(p.seconds);
                r.inclination = p.inclination;
                r.forcespeed = true;
                list.append(r);
            }
            trainProgram = new trainprogram(list, bluetoothManager);
        } else {

            return;
        }
        int countRow = 0;
        for (const auto &row : qAsConst(trainProgram->rows)) {
            if (ui->tableWidget->rowCount() <= countRow) {

                addEmptyRow();
            }

            QTableWidgetItem *i;
            editing = true;
            i = ui->tableWidget->takeItem(countRow, 0);
            i->setText(row.duration.toString(QStringLiteral("hh:mm:ss")));
            ui->tableWidget->setItem(countRow, 0, i);

            i = ui->tableWidget->takeItem(countRow, 1);
            i->setText(QString::number(row.speed));
            ui->tableWidget->setItem(countRow, 1, i);

            i = ui->tableWidget->takeItem(countRow, 2);
            i->setText(QString::number(row.inclination));
            ui->tableWidget->setItem(countRow, 2, i);

            i = ui->tableWidget->takeItem(countRow, 3);
            i->setCheckState(row.forcespeed ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
            ui->tableWidget->setItem(countRow, 3, i);

            editing = false;

            countRow++;
        }

        trainProgramSignals();
        ui->groupTrain->setChecked(true);
    }
}

void MainWindow::on_load_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QStringLiteral("train.xml"),

                                                    tr("Train Program (*.xml *.gpx)"));
    loadTrainProgram(fileName);
}

void MainWindow::on_reset_clicked() {
    if (bluetoothManager->device() && bluetoothManager->device()->currentSpeed().value() > 0) {
        return;
    }

    int countRow = 0;
    for (const auto &row : qAsConst(trainProgram->rows)) {

        QTableWidgetItem *i;
        editing = true;
        i = ui->tableWidget->takeItem(countRow, 0);
        i->setText(QStringLiteral("00:00:00"));
        ui->tableWidget->setItem(countRow, 0, i);

        i = ui->tableWidget->takeItem(countRow, 1);
        i->setText(QStringLiteral("0"));
        ui->tableWidget->setItem(countRow, 1, i);

        i = ui->tableWidget->takeItem(countRow, 2);
        i->setText(QStringLiteral("0"));
        ui->tableWidget->setItem(countRow, 2, i);

        i = ui->tableWidget->takeItem(countRow, 3);
        i->setCheckState(row.forcespeed ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
        ui->tableWidget->setItem(countRow, 3, i);

        editing = false;

        countRow++;
    }

    createTrainProgram(QList<trainrow>());
}

void MainWindow::on_stop_clicked() {
    if (bluetoothManager->device()) {

        bluetoothManager->device()->stop(false);
    }
}

void MainWindow::on_start_clicked() {

    trainProgram->restart();
    if (bluetoothManager->device()) {
        bluetoothManager->device()->start();
    }
}

void MainWindow::on_groupTrain_clicked() {
    if (!trainProgram) {

        createTrainProgram(QList<trainrow>());
    }
    trainProgram->enabled = ui->groupTrain->isChecked();
}

void MainWindow::on_fanSpeedMinus_clicked() {
    if (bluetoothManager->device()) {

        bluetoothManager->device()->changeFanSpeed(bluetoothManager->device()->fanSpeed() - 1);
    }
}

void MainWindow::on_fanSpeedPlus_clicked() {
    if (bluetoothManager->device()) {

        bluetoothManager->device()->changeFanSpeed(bluetoothManager->device()->fanSpeed() + 1);
    }
}

void MainWindow::on_difficulty_valueChanged(int value) {
    if (editing) {
        return;
    }

    for (int i = 0; i < trainProgram->rows.count(); i++) {
        trainProgram->rows[i].speed =
            trainProgram->loadedRows.at(i).speed + (trainProgram->loadedRows.at(i).speed * (0.02 * (value - 50)));
        trainProgram->rows[i].inclination = trainProgram->loadedRows.at(i).inclination +
                                            (trainProgram->loadedRows.at(i).inclination * (0.02 * (value - 50)));
    }

    int countRow = 0;
    for (const auto &row : qAsConst(trainProgram->rows)) {

        QTableWidgetItem *i;
        editing = true;

        i = ui->tableWidget->takeItem(countRow, 1);
        i->setText(QString::number(row.speed));
        ui->tableWidget->setItem(countRow, 1, i);

        i = ui->tableWidget->takeItem(countRow, 2);
        i->setText(QString::number(row.inclination));
        ui->tableWidget->setItem(countRow, 2, i);

        editing = false;

        countRow++;
    }
    ui->difficulty->setToolTip(QString::number(value) + QStringLiteral("%"));
}

void MainWindow::on_speedMinus_clicked() {
    if (bluetoothManager->device()) {
        if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
            ((treadmill *)bluetoothManager->device())
                ->changeSpeed(((treadmill *)bluetoothManager->device())->currentSpeed().value() - 0.5);
        }
    }
}

void MainWindow::on_speedPlus_clicked() {
    if (bluetoothManager->device()) {
        if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
            ((treadmill *)bluetoothManager->device())
                ->changeSpeed(((treadmill *)bluetoothManager->device())->currentSpeed().value() + 0.5);
        }
    }
}

void MainWindow::on_inclinationMinus_clicked() {
    if (bluetoothManager->device()) {
        if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
            double perc = ((treadmill *)bluetoothManager->device())->currentInclination().value() - 0.5;
            ((treadmill *)bluetoothManager->device())->changeInclination(perc, perc);
        }
    }
}

void MainWindow::on_inclinationPlus_clicked() {
    if (bluetoothManager->device()) {
        if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
            double perc = ((treadmill *)bluetoothManager->device())->currentInclination().value() + 0.5;
            ((treadmill *)bluetoothManager->device())->changeInclination(perc, perc);
        }
    }
}

void MainWindow::on_resistanceMinus_clicked() {
    if (bluetoothManager->device()) {
        if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
            ((bike *)bluetoothManager->device())
                ->changeResistance(((bike *)bluetoothManager->device())->currentResistance().value() - 1);
        } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
            ((rower *)bluetoothManager->device())
                ->changeResistance(((rower *)bluetoothManager->device())->currentResistance().value() - 1);
        } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {
            ((elliptical *)bluetoothManager->device())
                ->changeResistance(((elliptical *)bluetoothManager->device())->currentResistance().value() - 1);
        }
    }
}

void MainWindow::on_resistancePlus_clicked() {
    if (bluetoothManager->device()) {
        if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
            ((bike *)bluetoothManager->device())
                ->changeResistance(((bike *)bluetoothManager->device())->currentResistance().value() + 1);
        } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
            ((rower *)bluetoothManager->device())
                ->changeResistance(((rower *)bluetoothManager->device())->currentResistance().value() + 1);
        } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {
            ((elliptical *)bluetoothManager->device())
                ->changeResistance(((elliptical *)bluetoothManager->device())->currentResistance().value() + 1);
        }
    }
}

void MainWindow::on_chart_clicked() {}
