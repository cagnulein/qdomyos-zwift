#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include "gpx.h"

void MainWindow::load(treadmill* t)
{
    ui->setupUi(this);

    this->treadMill = t;
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::update);
    timer->start(1000);

    update();
}

MainWindow::MainWindow(treadmill* treadmill) :
    QDialog(nullptr),
    ui(new Ui::MainWindow)
{    
    load(treadmill);
}

MainWindow::MainWindow(treadmill* treadmill, QString trainProgram) :
    QDialog(nullptr),
    ui(new Ui::MainWindow)
{
    load(treadmill);
    loadTrainProgram(trainProgram);
}

void MainWindow::update()
{
    if(treadMill)
    {
        ui->speed->setText(QString::number(treadMill->currentSpeed()));
        ui->inclination->setText(QString::number(treadMill->currentInclination()));
        ui->heartrate->setText(QString::number(treadMill->currentHeart()));
        ui->odometer->setText(QString::number(treadMill->odometer()));
        ui->elevationGain->setText(QString::number(treadMill->elevationGain()));
        ui->calories->setText(QString::number(treadMill->calories()));
        ui->fanBar->setValue(treadMill->fanSpeed());
        ui->watt->setText(QString::number(treadMill->watts(ui->weight->text().toFloat())));

        if(treadMill->trainProgram)
        {
            ui->trainProgramElapsedTime->setText(treadMill->trainProgram->totalElapsedTime().toString("hh:mm:ss"));
            ui->trainProgramCurrentRowElapsedTime->setText(treadMill->trainProgram->currentRowElapsedTime().toString("hh:mm:ss"));
            ui->trainProgramDuration->setText(treadMill->trainProgram->duration().toString("hh:mm:ss"));

            double distance = treadMill->trainProgram->totalDistance();
            if(distance > 0)
            {
                ui->trainProgramTotalDistance->setText(QString::number(distance));
            }
            else
                ui->trainProgramTotalDistance->setText("N/A");
        }

        if(treadMill->connected())
        {
            ui->connectionToTreadmill->setEnabled(true);
            if(treadMill->VirtualTreadMill())
            {
                if(((virtualtreadmill*)treadMill->VirtualTreadMill())->connected())
                    ui->connectionToZwift->setEnabled(true);
                else
                    ui->connectionToZwift->setEnabled(false);
            }
            else
                ui->connectionToZwift->setEnabled(false);
        }
        else
            ui->connectionToTreadmill->setEnabled(false);
    }
    else
    {
        ui->connectionToTreadmill->setEnabled(false);
        ui->connectionToZwift->setEnabled(false);
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
    connect(treadMill->trainProgram, SIGNAL(start()), treadMill, SLOT(start()));
    connect(treadMill->trainProgram, SIGNAL(stop()), treadMill, SLOT(stop()));
    connect(treadMill->trainProgram, SIGNAL(changeSpeed(double)), treadMill, SLOT(changeSpeed(double)));
    connect(treadMill->trainProgram, SIGNAL(changeInclination(double)), treadMill, SLOT(changeInclination(double)));
    connect(treadMill->trainProgram, SIGNAL(changeSpeedAndInclination(double, double)), treadMill, SLOT(changeSpeedAndInclination(double, double)));
    connect(treadMill, SIGNAL(tapeStarted()), treadMill->trainProgram, SLOT(onTapeStarted()));
}

void MainWindow::createTrainProgram(QList<trainrow> rows)
{
    if(treadMill->trainProgram) delete treadMill->trainProgram;
    treadMill->trainProgram = new trainprogram(rows);
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
    if(!fileName.isEmpty() && treadMill->trainProgram)
        treadMill->trainProgram->save(fileName);
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
            if(treadMill->trainProgram)
                delete treadMill->trainProgram;
            treadMill->trainProgram = trainprogram::load(fileName);
        }
        else if(fileName.endsWith("gpx"))
        {
            if(treadMill->trainProgram)
                delete treadMill->trainProgram;
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
            treadMill->trainProgram = new trainprogram(list);
        }
        else
        {
            return;
        }
        int countRow = 0;
        foreach(trainrow row, treadMill->trainProgram->rows)
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
        ui->groupBox_2->setChecked(true);
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
    if(treadMill->currentSpeed() > 0) return;

    int countRow = 0;
    foreach(trainrow row, treadMill->trainProgram->rows)
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
    treadMill->stop();
}

void MainWindow::on_start_clicked()
{
    treadMill->trainProgram->restart();
    treadMill->start();
}

void MainWindow::on_groupBox_2_clicked()
{
    if(!treadMill->trainProgram)
        createTrainProgram(QList<trainrow>());
    treadMill->trainProgram->enabled = ui->groupBox_2->isChecked();
}

void MainWindow::on_fanSpeedMinus_clicked()
{
    if(treadMill)
        treadMill->changeFanSpeed(treadMill->fanSpeed() - 1);
}

void MainWindow::on_fanSpeedPlus_clicked()
{
    if(treadMill)
        treadMill->changeFanSpeed(treadMill->fanSpeed() + 1);
}

void MainWindow::on_difficulty_valueChanged(int value)
{
    if(editing) return;

    for(int i=0;i<treadMill->trainProgram->rows.count(); i++)
    {
        treadMill->trainProgram->rows[i].speed = treadMill->trainProgram->loadedRows[i].speed +
                (treadMill->trainProgram->loadedRows[i].speed * (0.02 * (value - 50)));
        treadMill->trainProgram->rows[i].inclination = treadMill->trainProgram->loadedRows[i].inclination +
                (treadMill->trainProgram->loadedRows[i].inclination * (0.02 * (value - 50)));
    }

    int countRow = 0;
    foreach(trainrow row, treadMill->trainProgram->rows)
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
