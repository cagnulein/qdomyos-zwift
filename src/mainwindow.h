#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDialog>
#include <QTimer>
#include <QTime>
#include <QDebug>
#include <QTableWidgetItem>
#include <trainprogram.h>
#include "domyostreadmill.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MainWindow(treadmill* t);
    ~MainWindow();

private:
    void addEmptyRow();
    void createTrainProgram(QList<trainrow> rows);
    void trainProgramSignals();
    bool editing = false;

    Ui::MainWindow *ui;
    QTimer *timer;

    treadmill* treadMill;

private slots:
    void update();
    void on_tableWidget_cellChanged(int row, int column);
    void on_tableWidget_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);
    void on_save_clicked();
    void on_load_clicked();
    void on_reset_clicked();
    void on_stop_clicked();
    void on_start_clicked();
    void on_groupBox_2_clicked();
    void on_fanSpeedMinus_clicked();
    void on_fanSpeedPlus_clicked();
};

#endif // MAINWINDOW_H
