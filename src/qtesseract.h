#ifndef QTESSERACT_H
#define QTESSERACT_H

#include <QApplication>

#ifdef Q_OS_WIN

#include <QQmlContext>
#include <QStyleFactory>
#include <stdio.h>
#include <stdlib.h>

#include "bluetooth.h"
#include "mainwindow.h"
#include <QDir>
#include <QGuiApplication>
#include <QOperatingSystemVersion>
#include <QQmlApplicationEngine>
#include <QSettings>
#include <QStandardPaths>

#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>

class qtesseract {
  public:
    static void captureWindow();

  private:
};
#endif
#endif // QTESSERACT_H
