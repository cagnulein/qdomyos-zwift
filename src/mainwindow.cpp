#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include "gpx.h"
#include "charts.h"

charts* Charts = 0;

void MainWindow::load(bluetooth* b)
{
    ui->setupUi(this);

    this->bluetoothManager = b;
    connect(this->bluetoothManager, SIGNAL(deviceConnected()), this, SLOT(trainProgramSignals()));
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::update);
    timer->start(1000);

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

MainWindow::MainWindow(bluetooth* b) :
    QDialog(nullptr),
    ui(new Ui::MainWindow)
{    
     load(b);
     this->trainProgram = new trainprogram(QList<trainrow>(), b);
}

MainWindow::MainWindow(bluetooth* b, QString trainProgram) :
    QDialog(nullptr),
    ui(new Ui::MainWindow)
{
     load(b);
    loadTrainProgram(trainProgram);
}

void MainWindow::update()
{
    if(bluetoothManager->device())
    {
        double inclination = 0;
        double resistance = 0;
        double watts = 0;
        double pace = 0;

        ui->speed->setText(QString::number(bluetoothManager->device()->currentSpeed(), 'f', 2));
        ui->heartrate->setText(QString::number(bluetoothManager->device()->currentHeart()));
        ui->odometer->setText(QString::number(bluetoothManager->device()->odometer(), 'f', 2));
        ui->calories->setText(QString::number(bluetoothManager->device()->calories(), 'f', 0));
        ui->fanBar->setValue(bluetoothManager->device()->fanSpeed());

        if(bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL)
        {
            pace = 10000 / (((treadmill*)bluetoothManager->device())->currentPace().second() + (((treadmill*)bluetoothManager->device())->currentPace().minute() * 60));
            if(pace < 0) pace = 0;
            watts = ((treadmill*)bluetoothManager->device())->watts(ui->weight->text().toFloat());
            inclination = ((treadmill*)bluetoothManager->device())->currentInclination();
            ui->pace->setText(((treadmill*)bluetoothManager->device())->currentPace().toString("m:ss"));
            ui->watt->setText(QString::number(watts, 'f', 0));
            ui->inclination->setText(QString::number(inclination, 'f', 1));
            ui->elevationGain->setText(QString::number(((treadmill*)bluetoothManager->device())->elevationGain(), 'f', 1));
        }
        else if(bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
        {
            resistance = ((bike*)bluetoothManager->device())->currentResistance();
            watts = ((bike*)bluetoothManager->device())->watts();
            ui->watt->setText(QString::number(watts));
            ui->resistance->setText(QString::number(resistance));
        }

        if(trainProgram)
        {
            ui->trainProgramElapsedTime->setText(trainProgram->totalElapsedTime().toString("hh:mm:ss"));
            ui->trainProgramCurrentRowElapsedTime->setText(trainProgram->currentRowElapsedTime().toString("hh:mm:ss"));
            ui->trainProgramDuration->setText(trainProgram->duration().toString("hh:mm:ss"));

            double distance = trainProgram->totalDistance();
            if(distance > 0)
            {
                ui->trainProgramTotalDistance->setText(QString::number(distance));
            }
            else
                ui->trainProgramTotalDistance->setText("N/A");
        }

        if(bluetoothManager->device()->connected())
        {
            ui->connectionToTreadmill->setEnabled(true);
            if(bluetoothManager->device()->VirtualDevice())
            {
                if(bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL &&
                        ((virtualtreadmill*)((treadmill*)bluetoothManager->device())->VirtualDevice())->connected())
                {
                    ui->connectionToZwift->setEnabled(true);
                }
                else if(bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE &&
                        ((virtualbike*)((bike*)bluetoothManager->device())->VirtualDevice())->connected())
                {
                    ui->connectionToZwift->setEnabled(true);
                }
                else
                    ui->connectionToZwift->setEnabled(false);
            }
            else
                ui->connectionToZwift->setEnabled(false);
        }
        else
            ui->connectionToTreadmill->setEnabled(false);

        SessionLine s(
                      bluetoothManager->device()->currentSpeed(),
                      inclination,
                      bluetoothManager->device()->odometer(),
                      watts,
                      resistance,
                      bluetoothManager->device()->currentHeart(),
                      pace);

        Session.append(s);

        if(ui->chart->isChecked())
        {
            if(!Charts)
            {
                Charts = new charts(this);
                Charts->show();
            }
            Charts->update();
        }
    }
    else
    {
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

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addEmptyRow()
{
    int row = ui->tableWidget->rowCount();
    editing = true;
    ui->tableWidget->insertRow(row);
    ui->tableWidget->setItem(row, 0, new QTableWidgetItem("00:00:00"));
    ui->tableWidget->setItem(row, 1, new QTableWidgetItem("10"));
    ui->tableWidget->setItem(row, 2, new QTableWidgetItem("0"));
    ui->tableWidget->setItem(row, 3, new QTableWidgetItem(""));
    ui->tableWidget->item(row, 0)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidget->item(row, 1)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidget->item(row, 2)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidget->item(row, 3)->setCheckState(Qt::CheckState::Checked);
    editing = false;
}

void MainWindow::on_tableWidget_cellChanged(int row, int column)
{    
    if(editing) return;
    if(column == 0)
    {
        switch(ui->tableWidget->currentItem()->text().length())
        {
        case 4:
            ui->tableWidget->currentItem()->setText("00:0" + ui->tableWidget->currentItem()->text());
            break;
        case 5:
            ui->tableWidget->currentItem()->setText("00:" + ui->tableWidget->currentItem()->text());
            break;
        case 7:
            ui->tableWidget->currentItem()->setText("0" + ui->tableWidget->currentItem()->text());
            break;
        }
        QString fmt = "hh:mm:ss";
        QTime dt = QTime::fromString(ui->tableWidget->currentItem()->text());
        QString timeStr = dt.toString("hh:mm:ss");
        ui->tableWidget->currentItem()->setText(timeStr);
    }

    if(row + 1 == ui->tableWidget->rowCount() && ui->tableWidget->currentItem()->text().length() )
        addEmptyRow();

    QList<trainrow> rows;
    for(int i = 0; i < ui->tableWidget->rowCount(); i++)
    {
        if(!ui->tableWidget->item(i, 0)->text().contains("00:00:00"))
        {
            trainrow t;
            t.duration = QTime::fromString(ui->tableWidget->item(i, 0)->text(), "hh:mm:ss");
            t.speed = ui->tableWidget->item(i, 1)->text().toFloat();
            t.inclination = ui->tableWidget->item(i, 2)->text().toFloat();
            t.forcespeed = ui->tableWidget->item(i, 3)->checkState() == Qt::CheckState::Checked;
            rows.append(t);
        }
        else
        {
            break;
        }
        createTrainProgram(rows);
    }
}

void MainWindow::trainProgramSignals()
{
     if(bluetoothManager->device())
     {
         disconnect(trainProgram, SIGNAL(start()), bluetoothManager->device(), SLOT(start()));
         disconnect(trainProgram, SIGNAL(stop()), bluetoothManager->device(), SLOT(stop()));
         disconnect(trainProgram, SIGNAL(changeSpeed(double)), ((treadmill*)bluetoothManager->device()), SLOT(changeSpeed(double)));
         disconnect(trainProgram, SIGNAL(changeInclination(double)), ((treadmill*)bluetoothManager->device()), SLOT(changeInclination(double)));
         disconnect(trainProgram, SIGNAL(changeSpeedAndInclination(double, double)), ((treadmill*)bluetoothManager->device()), SLOT(changeSpeedAndInclination(double, double)));
         disconnect(trainProgram, SIGNAL(changeResistance(double)), ((bike*)bluetoothManager->device()), SLOT(changeResistance(double)));
         disconnect(((treadmill*)bluetoothManager->device()), SIGNAL(tapeStarted()), trainProgram, SLOT(onTapeStarted()));
         disconnect(((bike*)bluetoothManager->device()), SIGNAL(bikeStarted()), trainProgram, SLOT(onTapeStarted()));

         connect(trainProgram, SIGNAL(start()), bluetoothManager->device(), SLOT(start()));
         connect(trainProgram, SIGNAL(stop()), bluetoothManager->device(), SLOT(stop()));
         connect(trainProgram, SIGNAL(changeSpeed(double)), ((treadmill*)bluetoothManager->device()), SLOT(changeSpeed(double)));
         connect(trainProgram, SIGNAL(changeInclination(double)), ((treadmill*)bluetoothManager->device()), SLOT(changeInclination(double)));
         connect(trainProgram, SIGNAL(changeSpeedAndInclination(double, double)), ((treadmill*)bluetoothManager->device()), SLOT(changeSpeedAndInclination(double, double)));
         connect(trainProgram, SIGNAL(changeResistance(double)), ((bike*)bluetoothManager->device()), SLOT(changeResistance(double)));
         connect(((treadmill*)bluetoothManager->device()), SIGNAL(tapeStarted()), trainProgram, SLOT(onTapeStarted()));
         connect(((bike*)bluetoothManager->device()), SIGNAL(bikeStarted()), trainProgram, SLOT(onTapeStarted()));

         qDebug() << "trainProgram associated to a device";
     }
     else
     {
         qDebug() << "trainProgram NOT associated to a device";
     }
}

void MainWindow::createTrainProgram(QList<trainrow> rows)
{
     if(trainProgram) delete trainProgram;
     trainProgram = new trainprogram(rows, bluetoothManager);
    if(rows.length() == 0)
        addEmptyRow();
    trainProgramSignals();    
}

void MainWindow::on_tableWidget_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous)
{
    Q_UNUSED(current);
    Q_UNUSED(previous);
}

void MainWindow::on_save_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                               "train.xml",
                               tr("Train Program (*.xml)"));
     if(!fileName.isEmpty() && trainProgram)
          trainProgram->save(fileName);
}

void MainWindow::loadTrainProgram(QString fileName)
{       
    if(!fileName.isEmpty())
    {
        ui->difficulty->setValue(50);
        int rows = ui->tableWidget->rowCount();
        for(int i = 0; i<rows; i++)
            ui->tableWidget->removeRow(ui->tableWidget->rowCount() - 1);

        if(fileName.endsWith("xml"))
        {
               if(trainProgram)
                     delete trainProgram;
                trainProgram = trainprogram::load(fileName, bluetoothManager);
        }
        else if(fileName.endsWith("gpx"))
        {
               if(trainProgram)
                     delete trainProgram;
            gpx g;
            QList<trainrow> list;
            foreach(gpx_altitude_point_for_treadmill p, g.open(fileName))
            {
                trainrow r;
                r.speed = p.speed;
                r.duration = QTime(0,0,0,0);
                r.duration = r.duration.addSecs(p.seconds);
                r.inclination = p.inclination;
                r.forcespeed = true;
                list.append(r);
            }
                trainProgram = new trainprogram(list, bluetoothManager);
        }
        else
        {
            return;
        }
        int countRow = 0;
          foreach(trainrow row, trainProgram->rows)
        {
            if(ui->tableWidget->rowCount() <= countRow)
                addEmptyRow();

            QTableWidgetItem* i;
            editing = true;
            i = ui->tableWidget->takeItem(countRow, 0);
            i->setText(row.duration.toString("hh:mm:ss"));
            ui->tableWidget->setItem(countRow, 0, i);

            i = ui->tableWidget->takeItem(countRow, 1);
            i->setText(QString::number(row.speed));
            ui->tableWidget->setItem(countRow, 1, i);

            i = ui->tableWidget->takeItem(countRow, 2);
            i->setText(QString::number(row.inclination));
            ui->tableWidget->setItem(countRow, 2, i);

            i = ui->tableWidget->takeItem(countRow, 3);
            i->setCheckState(row.forcespeed?Qt::CheckState::Checked:Qt::CheckState::Unchecked);
            ui->tableWidget->setItem(countRow, 3, i);

            editing = false;

            countRow++;
        }

        trainProgramSignals();
        ui->groupTrain->setChecked(true);
    }
}

void MainWindow::on_load_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                               "train.xml",
                               tr("Train Program (*.xml *.gpx)"));
    loadTrainProgram(fileName);
}

void MainWindow::on_reset_clicked()
{
     if(bluetoothManager->device() && bluetoothManager->device()->currentSpeed() > 0) return;

    int countRow = 0;
     foreach(trainrow row, trainProgram->rows)
    {
        QTableWidgetItem* i;
        editing = true;
        i = ui->tableWidget->takeItem(countRow, 0);
        i->setText("00:00:00");
        ui->tableWidget->setItem(countRow, 0, i);

        i = ui->tableWidget->takeItem(countRow, 1);
        i->setText("0");
        ui->tableWidget->setItem(countRow, 1, i);

        i = ui->tableWidget->takeItem(countRow, 2);
        i->setText("0");
        ui->tableWidget->setItem(countRow, 2, i);

        i = ui->tableWidget->takeItem(countRow, 3);
        i->setCheckState(row.forcespeed?Qt::CheckState::Checked:Qt::CheckState::Unchecked);
        ui->tableWidget->setItem(countRow, 3, i);

        editing = false;

        countRow++;
    }

    createTrainProgram(QList<trainrow>());
}

void MainWindow::on_stop_clicked()
{
    if(bluetoothManager->device())
        bluetoothManager->device()->stop();
}

void MainWindow::on_start_clicked()
{
     trainProgram->restart();
     if(bluetoothManager->device())
         bluetoothManager->device()->start();
}

void MainWindow::on_groupTrain_clicked()
{
     if(!trainProgram)
        createTrainProgram(QList<trainrow>());
     trainProgram->enabled = ui->groupTrain->isChecked();
}

void MainWindow::on_fanSpeedMinus_clicked()
{
     if(bluetoothManager->device())
          bluetoothManager->device()->changeFanSpeed(bluetoothManager->device()->fanSpeed() - 1);
}

void MainWindow::on_fanSpeedPlus_clicked()
{
     if(bluetoothManager->device())
          bluetoothManager->device()->changeFanSpeed(bluetoothManager->device()->fanSpeed() + 1);
}

void MainWindow::on_difficulty_valueChanged(int value)
{
    if(editing) return;

     for(int i=0;i<trainProgram->rows.count(); i++)
    {
          trainProgram->rows[i].speed = trainProgram->loadedRows[i].speed +
                  (trainProgram->loadedRows[i].speed * (0.02 * (value - 50)));
          trainProgram->rows[i].inclination = trainProgram->loadedRows[i].inclination +
                  (trainProgram->loadedRows[i].inclination * (0.02 * (value - 50)));
    }

    int countRow = 0;
     foreach(trainrow row, trainProgram->rows)
    {
        QTableWidgetItem* i;
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
    ui->difficulty->setToolTip(QString::number(value) + "%");
}

void MainWindow::on_speedMinus_clicked()
{
    if(bluetoothManager->device())
    {
        if(bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL)
        {
            ((treadmill*)bluetoothManager->device())->changeSpeed(((treadmill*)bluetoothManager->device())->currentSpeed() - 0.5);
        }
    }
}

void MainWindow::on_speedPlus_clicked()
{
    if(bluetoothManager->device())
    {
        if(bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL)
        {
            ((treadmill*)bluetoothManager->device())->changeSpeed(((treadmill*)bluetoothManager->device())->currentSpeed() + 0.5);
        }
    }
}

void MainWindow::on_inclinationMinus_clicked()
{
    if(bluetoothManager->device())
    {
        if(bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL)
        {
            ((treadmill*)bluetoothManager->device())->changeInclination(((treadmill*)bluetoothManager->device())->currentInclination() - 0.5);
        }
    }
}

void MainWindow::on_inclinationPlus_clicked()
{
    if(bluetoothManager->device())
    {
        if(bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL)
        {
            ((treadmill*)bluetoothManager->device())->changeInclination(((treadmill*)bluetoothManager->device())->currentInclination() + 0.5);
        }
    }
}

void MainWindow::on_resistanceMinus_clicked()
{
    if(bluetoothManager->device())
    {
        if(bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
        {
            ((bike*)bluetoothManager->device())->changeResistance(((bike*)bluetoothManager->device())->currentResistance() - 1);
        }
    }
}

void MainWindow::on_resistancePlus_clicked()
{
    if(bluetoothManager->device())
    {
        if(bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
        {
            ((bike*)bluetoothManager->device())->changeResistance(((bike*)bluetoothManager->device())->currentResistance() + 1);
        }
    }
}

void MainWindow::on_chart_clicked()
{

}
