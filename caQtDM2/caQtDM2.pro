include (../caQtDM.pri)

TARGET = caQtDM_Hipa
TEMPLATE = app

LIBS += -L$${QTCONTROLS} -Wl,-rpath,$${QTCONTROLS} -lcaQtDM_Lib_Hipa

OBJECTS_DIR = ../obj2
DEFINES += SUPPORT=\\\"EPICS_ACS\\\"
