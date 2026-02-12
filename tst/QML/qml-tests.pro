CONFIG += qmltestcase
QT += qml quick
TARGET = qml-tests

SOURCES += qml_test_runner.cpp

# Define test data directory
DEFINES += QUICK_TEST_SOURCE_DIR=\\\"$$PWD\\\"

# Other test data files
OTHER_FILES += \
    tst_trainingprogramslist.qml

# Set output directory
DESTDIR = $$OUT_PWD
