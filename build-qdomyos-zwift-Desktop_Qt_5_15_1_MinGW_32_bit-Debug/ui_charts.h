/********************************************************************************
** Form generated from reading UI file 'charts.ui'
**
** Created by: Qt User Interface Compiler version 5.15.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHARTS_H
#define UI_CHARTS_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_charts
{
public:
    QVBoxLayout *verticalLayout;
    QFrame *frame;
    QHBoxLayout *horizontalLayout;
    QWidget *widget;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *speed;
    QPushButton *inclination;
    QPushButton *watt;
    QPushButton *resistance;
    QPushButton *heart;
    QPushButton *pace;
    QCheckBox *valueOnChart;

    void setupUi(QDialog *charts)
    {
        if (charts->objectName().isEmpty())
            charts->setObjectName(QString::fromUtf8("charts"));
        charts->resize(640, 480);
        verticalLayout = new QVBoxLayout(charts);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        frame = new QFrame(charts);
        frame->setObjectName(QString::fromUtf8("frame"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(frame->sizePolicy().hasHeightForWidth());
        frame->setSizePolicy(sizePolicy);
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        horizontalLayout = new QHBoxLayout(frame);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        widget = new QWidget(frame);
        widget->setObjectName(QString::fromUtf8("widget"));
        sizePolicy.setHeightForWidth(widget->sizePolicy().hasHeightForWidth());
        widget->setSizePolicy(sizePolicy);
        widget->setMaximumSize(QSize(16777215, 16777215));
        widget->setAutoFillBackground(false);

        horizontalLayout->addWidget(widget);


        verticalLayout->addWidget(frame);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        speed = new QPushButton(charts);
        speed->setObjectName(QString::fromUtf8("speed"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/icons/speed.png"), QSize(), QIcon::Normal, QIcon::On);
        speed->setIcon(icon);
        speed->setIconSize(QSize(32, 32));
        speed->setCheckable(true);
        speed->setChecked(true);

        horizontalLayout_2->addWidget(speed);

        inclination = new QPushButton(charts);
        inclination->setObjectName(QString::fromUtf8("inclination"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/icons/inclination.png"), QSize(), QIcon::Normal, QIcon::On);
        inclination->setIcon(icon1);
        inclination->setIconSize(QSize(32, 32));
        inclination->setCheckable(true);
        inclination->setChecked(true);

        horizontalLayout_2->addWidget(inclination);

        watt = new QPushButton(charts);
        watt->setObjectName(QString::fromUtf8("watt"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icons/icons/watt.png"), QSize(), QIcon::Normal, QIcon::On);
        watt->setIcon(icon2);
        watt->setIconSize(QSize(32, 32));
        watt->setCheckable(true);
        watt->setChecked(true);

        horizontalLayout_2->addWidget(watt);

        resistance = new QPushButton(charts);
        resistance->setObjectName(QString::fromUtf8("resistance"));
        resistance->setIcon(icon1);
        resistance->setIconSize(QSize(32, 32));
        resistance->setCheckable(true);

        horizontalLayout_2->addWidget(resistance);

        heart = new QPushButton(charts);
        heart->setObjectName(QString::fromUtf8("heart"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icons/icons/heart_red.png"), QSize(), QIcon::Normal, QIcon::On);
        heart->setIcon(icon3);
        heart->setIconSize(QSize(32, 32));
        heart->setCheckable(true);
        heart->setChecked(true);

        horizontalLayout_2->addWidget(heart);

        pace = new QPushButton(charts);
        pace->setObjectName(QString::fromUtf8("pace"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/icons/icons/pace.png"), QSize(), QIcon::Normal, QIcon::On);
        pace->setIcon(icon4);
        pace->setIconSize(QSize(32, 32));
        pace->setCheckable(true);
        pace->setChecked(true);

        horizontalLayout_2->addWidget(pace);


        verticalLayout->addLayout(horizontalLayout_2);

        valueOnChart = new QCheckBox(charts);
        valueOnChart->setObjectName(QString::fromUtf8("valueOnChart"));

        verticalLayout->addWidget(valueOnChart);


        retranslateUi(charts);

        QMetaObject::connectSlotsByName(charts);
    } // setupUi

    void retranslateUi(QDialog *charts)
    {
        charts->setWindowTitle(QCoreApplication::translate("charts", "Charts", nullptr));
        speed->setText(QCoreApplication::translate("charts", "Speed", nullptr));
        inclination->setText(QCoreApplication::translate("charts", "Inclination", nullptr));
        watt->setText(QCoreApplication::translate("charts", "Watt", nullptr));
        resistance->setText(QCoreApplication::translate("charts", "Resistance", nullptr));
        heart->setText(QCoreApplication::translate("charts", "Heart", nullptr));
        pace->setText(QCoreApplication::translate("charts", "Pace", nullptr));
        valueOnChart->setText(QCoreApplication::translate("charts", "Value on Chart", nullptr));
    } // retranslateUi

};

namespace Ui {
    class charts: public Ui_charts {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHARTS_H
