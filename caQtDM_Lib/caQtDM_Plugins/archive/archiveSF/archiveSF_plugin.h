/*
 *  This file is part of the caQtDM Framework, developed at the Paul Scherrer Institut,
 *  Villigen, Switzerland
 *
 *  The caQtDM Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The caQtDM Framework is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the caQtDM Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright (c) 2010 - 2014
 *
 *  Author:
 *    Anton Mezger
 *  Contact details:
 *    anton.mezger@psi.ch
 */
#ifndef ArchiveSF_Plugin_H
#define ArchiveSF_Plugin_H

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QList>
#include <QTimer>
#include <QThread>
#include <qwt.h>
#include "cacartesianplot.h"
#include "controlsinterface.h"
#include "archiveSF_plugin.h"
#include "archiverCommon.h"
#include "sfRetrieval.h"


class Q_DECL_EXPORT WorkerSF : public QObject
{
    Q_OBJECT

public:
    WorkerSF() {
        //qDebug() << "WorkerSF::WorkerSF()";
        qRegisterMetaType<indexes>("indexes");
        qRegisterMetaType<QVector<float> >("QVector<float>");
        fromArchive =  (sfRetrieval *)0;
    }

    ~WorkerSF() {
        //qDebug() << "WorkerSF::~WorkerSF()";
    }

private:
    QVector<float>  TimerN, YValsN;

public slots:

    void workerFinish() {
        deleteLater();
    }

    sfRetrieval* getArchive() {
        return fromArchive;
    }

    void getFromArchive(QWidget *w, indexes indexNew,  QString index_name, MessageWindow * messageWindow) {

        Q_UNUSED(w);

        bool timeAxis = false;

        QMutex *mutex = indexNew.mutexP;
        mutex->lock();

        struct timeb now;
        QUrl url = QUrl(index_name);
        QString fields, agg;
        bool isBinned;

        QString key = indexNew.pv;
        int nbVal = 0;

        ftime(&now);
        double endSeconds = (double) now.time + (double) now.millitm / (double)1000;
        double startSeconds = endSeconds - indexNew.secondsPast;
#ifdef CSV
        QString response ="'response':{'format':'csv'}";
#else
        QString response ="'response':{'format':'json'}";
#endif
        QString channels;
        if(indexNew.backend.size() > 0) {
           channels = "'channels': [ {'name':'" + key + "', 'backend' : '" + indexNew.backend + "' }]";
        } else {
           channels = "'channels': [ {'name':'" + key + "' }]";
        }

        QString range = "'range': { 'startSeconds' : '" + QString::number(startSeconds, 'g', 10) + "', 'endSeconds' : '" + QString::number(endSeconds, 'g', 10) + "'}";
        fields = "'fields':['channel','globalSeconds','value']";

        if(indexNew.nrOfBins != -1) {
            isBinned = true;
            agg = tr(", 'aggregation': {'aggregationType':'value', 'aggregations':['min','mean','max'], 'nrOfBins' : %1}").arg(indexNew.nrOfBins);
        } else {
            isBinned = true;
            agg = ", 'aggregation': {'aggregationType':'value', 'aggregations':['min','mean','max'], 'durationPerBin' : 'PT1S'}";
            //agg = "";
        }
        QString total = "{" + response + "," + range + "," + channels + "," + fields + agg + "}";
        total = total.replace("'", "\"");
        QByteArray json_str = total.toUtf8();

        fromArchive = new sfRetrieval();

        //qDebug() << "fromArchive pointer=" << fromArchive << indexNew.timeAxis;

        if(fromArchive->requestUrl(url, json_str, indexNew.secondsPast, isBinned, indexNew.timeAxis, key)) {
            if((nbVal = fromArchive->getCount()) > 0) {
                //qDebug() << nbVal << total;
                TimerN.resize(fromArchive->getCount());
                YValsN.resize(fromArchive->getCount());
                fromArchive->getData(TimerN, YValsN);
            }
        } else {
            if(messageWindow != (MessageWindow *) 0) {
                QString mess("ArchiveSF plugin -- lastError: ");
                mess.append(fromArchive->lastError());
                mess.append(" for pv: ");
                mess.append(key);
#if QT_VERSION > 0x050000
                mess=QString(mess.toHtmlEscaped());
#else
                mess = (Qt::escape(mess));
#endif
                messageWindow->postMsgEvent(QtFatalMsg, (char*) qasc(mess));
            }
        }

        //qDebug() << QTime::currentTime().toString() << "number of values received" << nbVal << fromArchive << "for" << key;

        emit resultReady(indexNew, nbVal, TimerN, YValsN, fromArchive->getBackend());

        mutex->unlock();
    }

signals:
    void resultReady(indexes indexNew, int nbVal, QVector<float> TimerN, QVector<float> YValsN, QString backend);

public:

private:
    sfRetrieval *fromArchive;

};

class Q_DECL_EXPORT myThread : public QThread
{
    Q_OBJECT

public:
    myThread(WorkerSF *worker) {
        pworker = worker;
        //qDebug() << "myThread::myThread()";
    }
    ~myThread() {
        //qDebug() << "myThread::~myThread()";
    }
    WorkerSF *workersf() {
        return pworker;
    }

    sfRetrieval *getArchive() {
        if(pworker != (WorkerSF *) 0) {
            return pworker->getArchive();
        } else {
            return (sfRetrieval *) 0;
        }
    }

private:
    WorkerSF *pworker;
};

class Q_DECL_EXPORT ArchiveSF_Plugin : public QObject, ControlsInterface
{
    Q_OBJECT

    Q_INTERFACES(ControlsInterface)
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "ch.psi.caqtdm.Plugin.ControlsInterface/1.0.democontrols")
#endif

public:
    QString pluginName();
    ArchiveSF_Plugin();
    ~ArchiveSF_Plugin();

    int initCommunicationLayer(MutexKnobData *data, MessageWindow *messageWindow, QMap<QString, QString> options);
    int pvAddMonitor(int index, knobData *kData, int rate, int skip);
    int pvClearMonitor(knobData *kData);
    int pvFreeAllocatedData(knobData *kData);
    int pvSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType);
    int pvSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess);
    int pvGetTimeStamp(char *pv, char *timestamp);
    int pvGetDescription(char *pv, char *description);
    int pvClearEvent(void * ptr);
    int pvAddEvent(void * ptr);
    int pvReconnect(knobData *kData);
    int pvDisconnect(knobData *kData);
    int FlushIO();
    int TerminateIO();

public slots:
    void handleResults(indexes, int, QVector<float>, QVector<float>, QString);

signals:
    void operate(QWidget*, const indexes, const QString, MessageWindow *);
    void Signal_StopUpdateInterface();

private slots:
    void Callback_UpdateInterface( QMap<QString, indexes> listOfIndexes);
    void Callback_AbortOutstandingRequests(QString key);
    void closeEvent();

private:
    QMutex mutex;
    MutexKnobData *mutexknobdataP;
    MessageWindow *messagewindowP;
    ArchiverCommon *archiverCommon;
    QMap<QString, myThread*> listOfThreads;
    bool suspend;
};

#endif
