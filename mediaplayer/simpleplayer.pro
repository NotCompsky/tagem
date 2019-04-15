TEMPLATE = app
CONFIG -= app_bundle
QT += widgets
QT += avwidgets

PROJECTROOT = $$PWD/../..
#include($$PROJECTROOT/src/libQtAV.pri)
#include($$PROJECTROOT/widgets/libQtAVWidgets.pri)
#preparePaths($$OUT_PWD/../../out)

include(src.pri)
LIBS += -lmysqlcppconn

DISTFILES += \
    ui/UIForm.ui.qml \
    ui/UI.qml
