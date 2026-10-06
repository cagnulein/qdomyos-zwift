TEMPLATE = lib
TARGET = qios
CONFIG += static plugin

isEmpty(QIOS_SOURCE_DIR): error("QIOS_SOURCE_DIR must point to qtbase/src/plugins/platforms/ios")

QT += \
    core-private gui-private \
    clipboard_support-private fontdatabase_support-private graphics_support-private

INCLUDEPATH += $$QIOS_SOURCE_DIR

LIBS += -framework Foundation -framework UIKit -framework QuartzCore -framework AudioToolbox
LIBS += -framework AssetsLibrary

OBJECTIVE_SOURCES = \
    $$QIOS_SOURCE_DIR/plugin.mm \
    $$QIOS_SOURCE_DIR/qiosintegration.mm \
    $$QIOS_SOURCE_DIR/qioseventdispatcher.mm \
    $$QIOS_SOURCE_DIR/qioswindow.mm \
    $$QIOS_SOURCE_DIR/qiosscreen.mm \
    $$QIOS_SOURCE_DIR/qiosbackingstore.mm \
    $$QIOS_SOURCE_DIR/qiosapplicationdelegate.mm \
    $$QIOS_SOURCE_DIR/qiosapplicationstate.mm \
    $$QIOS_SOURCE_DIR/qiosviewcontroller.mm \
    $$QIOS_SOURCE_DIR/qioscontext.mm \
    $$QIOS_SOURCE_DIR/qiosinputcontext.mm \
    $$QIOS_SOURCE_DIR/qiostheme.mm \
    $$QIOS_SOURCE_DIR/qiosglobal.mm \
    $$QIOS_SOURCE_DIR/qiosservices.mm \
    $$QIOS_SOURCE_DIR/quiview.mm \
    $$QIOS_SOURCE_DIR/quiaccessibilityelement.mm \
    $$QIOS_SOURCE_DIR/qiosplatformaccessibility.mm \
    $$QIOS_SOURCE_DIR/qiostextresponder.mm \
    $$QIOS_SOURCE_DIR/qiosclipboard.mm \
    $$QIOS_SOURCE_DIR/qiosmenu.mm \
    $$QIOS_SOURCE_DIR/qiosfiledialog.mm \
    $$QIOS_SOURCE_DIR/qiosmessagedialog.mm \
    $$QIOS_SOURCE_DIR/qiostextinputoverlay.mm \
    $$QIOS_SOURCE_DIR/qiosdocumentpickercontroller.mm

HEADERS = \
    $$QIOS_SOURCE_DIR/qiosintegration.h \
    $$QIOS_SOURCE_DIR/qioseventdispatcher.h \
    $$QIOS_SOURCE_DIR/qioswindow.h \
    $$QIOS_SOURCE_DIR/qiosscreen.h \
    $$QIOS_SOURCE_DIR/qiosbackingstore.h \
    $$QIOS_SOURCE_DIR/qiosapplicationdelegate.h \
    $$QIOS_SOURCE_DIR/qiosapplicationstate.h \
    $$QIOS_SOURCE_DIR/qiosviewcontroller.h \
    $$QIOS_SOURCE_DIR/qioscontext.h \
    $$QIOS_SOURCE_DIR/qiosinputcontext.h \
    $$QIOS_SOURCE_DIR/qiostheme.h \
    $$QIOS_SOURCE_DIR/qiosglobal.h \
    $$QIOS_SOURCE_DIR/qiosservices.h \
    $$QIOS_SOURCE_DIR/quiview.h \
    $$QIOS_SOURCE_DIR/quiaccessibilityelement.h \
    $$QIOS_SOURCE_DIR/qiosplatformaccessibility.h \
    $$QIOS_SOURCE_DIR/qiostextresponder.h \
    $$QIOS_SOURCE_DIR/qiosclipboard.h \
    $$QIOS_SOURCE_DIR/qiosmenu.h \
    $$QIOS_SOURCE_DIR/qiosfiledialog.h \
    $$QIOS_SOURCE_DIR/qiosmessagedialog.h \
    $$QIOS_SOURCE_DIR/qiostextinputoverlay.h \
    $$QIOS_SOURCE_DIR/qiosdocumentpickercontroller.h

OTHER_FILES = \
    $$QIOS_SOURCE_DIR/quiview_textinput.mm \
    $$QIOS_SOURCE_DIR/quiview_accessibility.mm
