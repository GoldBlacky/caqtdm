include (../../../caQtDM_Viewer/qtdefs.pri)
QT += core gui
contains(QT_VER_MAJ, 5) {
    QT     += widgets
}

CONFIG += warn_on
CONFIG += release
CONFIG += bsread_Plugin
include (../../../caQtDM.pri)

TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += .
INCLUDEPATH    += ../
INCLUDEPATH    += ../../src
HEADERS         = bsread_Plugin.h bsreadExternals.h ../controlsinterface.h
SOURCES         = bsread_Plugin.cpp md5.cc
TARGET          = bsread_Plugin
DESTDIR         = $(CAQTDM_COLLECT)/controlsystems

