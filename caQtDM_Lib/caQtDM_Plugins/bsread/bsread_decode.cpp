#include <QThread>
#include "zmq.h"

#include "bsread_decode.h"
#include "knobData.h"
#include "JSON.h"
#include "JSONValue.h"
#include "bsread_channeldata.h"

//Q_DECLARE_METATYPE(bsread_channeldata)

bsread_Decode::bsread_Decode(void * Context,QString ConnectionPoint)
{
    int rc;
    zmqsocket=zmq_socket(Context, ZMQ_PULL);
    if (!zmqsocket) {
        printf ("error in zmq_socket: %s\n", zmq_strerror (errno));
    }
    rc = zmq_connect (zmqsocket, ConnectionPoint.toLatin1().constData());
    if (rc != 0) {
        printf ("error in zmq_bind: %s(%s)\n", zmq_strerror (errno),ConnectionPoint.toLatin1().constData());
        running_decode=false;
    }else{
        running_decode=true;
        channelcounter=0;
    }


}
void bsread_Decode::run()
{
    int rc;
    zmq_msg_t msg;
    int64_t more;
    size_t more_size = sizeof (more);
    rc = zmq_msg_init (&msg);

    while (1){
        //printf("Message\n");
        rc = zmq_recvmsg (zmqsocket, &msg, 0);
        if (rc < 0) {
            printf ("error in zmq_recvmsg: %s\n", zmq_strerror (errno));

        }
        //        if (zmq_msg_size (&msg) != message_size) {
        //            printf ("message of incorrect size received\n");

        //        }
        setMainHeader((char*)zmq_msg_data(&msg));

        zmq_getsockopt (zmqsocket, ZMQ_RCVMORE, &more, &more_size);
        if (more){

            rc = zmq_recvmsg (zmqsocket, &msg, 0);
            if (rc < 0) {
                printf ("error in zmq_recvmsg(Header): %s\n", zmq_strerror (errno));
            }
            setHeader((char*)zmq_msg_data(&msg));

            zmq_getsockopt (zmqsocket, ZMQ_RCVMORE, &more, &more_size);
            while(more){
                rc = zmq_recvmsg (zmqsocket, &msg, 0);
                if (rc < 0) {
                    printf ("error in zmq_recvmsg(Data): %s\n", zmq_strerror (errno));
                }

                bsread_SetChannelData(zmq_msg_data(&msg));
                zmq_getsockopt (zmqsocket, ZMQ_RCVMORE, &more, &more_size);
                if (more){
                    rc = zmq_recvmsg (zmqsocket, &msg, 0);
                    if (rc < 0) {
                        printf ("error in zmq_recvmsg(Timestamp): %s\n", zmq_strerror (errno));
                    }
                    bsread_SetChannelTimeStamp(zmq_msg_data(&msg));
                    zmq_getsockopt (zmqsocket, ZMQ_RCVMORE, &more, &more_size);
                }



            }
            bsread_EndofData();

        }




    }


}
size_t bsread_Decode::getMessage_size() const
{
    return message_size;
}
QString bsread_Decode::getMainHeader() const
{
    return MainHeader;
}

bool bsread_Decode::setMainHeader(char *value)
{
    JSONObject jsonobj;
    MainHeader = QString(value);

    JSONValue *MainMessageJ = JSON::Parse(value);
    if (MainMessageJ!=NULL){
        if(!MainMessageJ->IsObject()) {
            delete(MainMessageJ);
        } else {
            jsonobj=MainMessageJ->AsObject();
            if (jsonobj.find(L"hash") != jsonobj.end() && jsonobj[L"hash"]->IsString()) {
                hash=QString::fromWCharArray(jsonobj[L"hash"]->AsString().c_str());
                //printf("hType :%s\n",hash.toLatin1().constData());
            }

            if (jsonobj.find(L"pulse_id") != jsonobj.end() && jsonobj[L"pulse_id"]->IsNumber()) {
                pulse_id=jsonobj[L"pulse_id"]->AsNumber();
            }
            if (jsonobj.find(L"htype") != jsonobj.end() && jsonobj[L"htype"]->IsString()) {
                main_htype=QString::fromWCharArray(jsonobj[L"htype"]->AsString().c_str());
            }
            if (jsonobj.find(L"global_timestamp") != jsonobj.end() && jsonobj[L"global_timestamp"]->IsObject())
            {
                JSONObject jsonobj2=jsonobj[L"global_timestamp"]->AsObject();
                if (jsonobj2.find(L"epoch") != jsonobj2.end() && jsonobj2[L"epoch"]->IsNumber()) {
                    global_timestamp_epoch=jsonobj2[L"epoch"]->AsNumber();
                }
                if (jsonobj2.find(L"ns") != jsonobj2.end() && jsonobj2[L"ns"]->IsNumber()) {
                    global_timestamp_ns=jsonobj2[L"ns"]->AsNumber();
                }
            }
            delete(MainMessageJ);

        }
    }

}
void bsread_Decode::setHeader(char *value){

    ChannelHeader = QString(value);

    Channels.clear();
    ChannelSearch.clear();
    JSONValue *HeaderMessageJ = JSON::Parse(value);
    if (HeaderMessageJ!=NULL){
        if(!HeaderMessageJ->IsObject()) {
            delete(HeaderMessageJ);
        } else {
            JSONObject jsonobj=HeaderMessageJ->AsObject();
            if (jsonobj.find(L"channels") != jsonobj.end() && jsonobj[L"channels"]->IsArray()) {


                JSONArray jsonobj2=jsonobj[L"channels"]->AsArray();

                for (unsigned int i = 0; i < jsonobj2.size(); i++)
                {

                    bsread_channeldata *chdata=new bsread_channeldata();
                    Channels.append(chdata);
                    JSONObject jsonobj3=jsonobj2[i]->AsObject();
                    if (jsonobj3.find(L"type") != jsonobj3.end() && jsonobj3[L"type"]->IsString()) {

                        QString value=QString::fromWCharArray(jsonobj3[L"type"]->AsString().c_str());
                        if (value=="double"){
                            chdata->type=bs_double;
                        }else if(value=="string"){
                            chdata->type=bs_string;
                        }else if(value=="integer"){
                            chdata->type=bs_integer;
                        }else if(value=="long"){
                            chdata->type=bs_long;
                        }else if(value=="short"){
                            chdata->type=bs_short;
                        }


                    }
                    if (jsonobj3.find(L"name") != jsonobj3.end() && jsonobj3[L"name"]->IsString()) {
                        chdata->name=QString::fromWCharArray(jsonobj3[L"name"]->AsString().c_str());
                        ChannelSearch.insert(chdata->name, chdata);
                        // printf("Ch-Name :%s\n",chdata->name.toLatin1().constData());
                    }
                    if (jsonobj3.find(L"offset") != jsonobj3.end() && jsonobj3[L"offset"]->IsNumber()) {
                        chdata->offset=jsonobj3[L"offset"]->AsNumber();
                    }
                    if (jsonobj3.find(L"modulo") != jsonobj3.end() && jsonobj3[L"modulo"]->IsNumber()) {
                        chdata->modulo=jsonobj3[L"modulo"]->AsNumber();
                    }

                    if (jsonobj3.find(L"encoding") != jsonobj3.end() && jsonobj3[L"encoding"]->IsString()) {
                        if (QString::fromWCharArray(jsonobj3[L"encoding"]->AsString().c_str())=="big"){
                            chdata->endianess=bs_big;
                        }

                        if (jsonobj3.find(L"shape") != jsonobj3.end() && jsonobj3[L"shape"]->IsArray()) {
                            chdata->shape.clear();
                            JSONArray jsonobj4=jsonobj3[L"shape"]->AsArray();
                            for (unsigned int j = 0; j < jsonobj4.size(); j++){
                                int value=(int)jsonobj4[j]->AsNumber();
                                chdata->shape.append(value);
                            }

                        }
                    }

                }

                if (jsonobj.find(L"pulse_id") != jsonobj.end() && jsonobj[L"pulse_id"]->IsNumber()) {
                    pulse_id=jsonobj[L"pulse_id"]->AsNumber();
                }
                if (jsonobj.find(L"htype") != jsonobj.end() && jsonobj[L"htype"]->IsNumber()) {
                    main_htype=QString::fromWCharArray(jsonobj[L"htype"]->AsString().c_str());
                }
                if (jsonobj.find(L"global_timestamp") != jsonobj.end() && jsonobj[L"global_timestamp"]->IsObject())
                {
                    JSONObject jsonobj2=jsonobj[L"global_timestamp"]->AsObject();
                    if (jsonobj2.find(L"epoch") != jsonobj2.end() && jsonobj2[L"epoch"]->IsNumber()) {
                        global_timestamp_epoch=jsonobj2[L"epoch"]->AsNumber();
                    }
                    if (jsonobj2.find(L"ns") != jsonobj2.end() && jsonobj2[L"ns"]->IsNumber()) {
                        global_timestamp_ns=jsonobj2[L"ns"]->AsNumber();
                    }
                }
                delete(HeaderMessageJ);

            }
        }



    }

}
void bsread_Decode::bsread_SetChannelData(void *message)
{
    if ((message)&&(Channels.size()>channelcounter)){
        switch (Channels.at(channelcounter)->type){
        case bs_double:{
            Channels.at(channelcounter)->bsdata.bs_double=*(double*) message;
            //printf("Data :%s %f\n",Channels.at(channelcounter)->name.toLatin1().constData(),Channels.at(channelcounter)->bsdata.bs_double);
            break;
        }
        case bs_string:{
            Channels.at(channelcounter)->bsdata.bs_string=QString((char*)message);
            break;
        }
        case bs_integer:{
            Channels.at(channelcounter)->bsdata.bs_double=*(int*) message;
            break;
        }
        case bs_long:{
            Channels.at(channelcounter)->bsdata.bs_double=*(long*) message;
            break;
        }
        case bs_short:{
            Channels.at(channelcounter)->bsdata.bs_double=*(short*) message;
            break;
        }
        }
    }
}

void bsread_Decode::bsread_SetChannelTimeStamp(void * timestamp)
{
    if ((timestamp)&&(Channels.size()>channelcounter)){
        Channels.at(channelcounter)->timestamp=*(double*) timestamp;
        channelcounter++;
    }
}

void bsread_Decode::bsread_EndofData()
{
    bsread_channeldata * bsreadPV;
    channelcounter=0;
    //Update Knobdata
    foreach(int index, listOfIndexes) {
        knobData* kData = KnobData->GetMutexKnobDataPtr(index);
        if((kData != (knobData *) 0) && (kData->index != -1)) {
            QString key = kData->pv;

            // find this pv in our internal double values list (assume for now we are only treating doubles)
            // and increment its value
            QMap<QString,bsread_channeldata*>::iterator i = ChannelSearch.find(key);
            while (i !=ChannelSearch.end() && i.key() == key) {
                bsreadPV = i.value();
                break;//?????
            }
            // update some data
            switch (bsreadPV->type){
            case bs_double:{
                kData->edata.rvalue=bsreadPV->bsdata.bs_double;
                kData->edata.fieldtype = caDOUBLE;
                break;
            }
            case bs_string:{
                kData->edata.fieldtype = caSTRING;
                kData->edata.dataSize= bsreadPV->bsdata.bs_string.length();
                if (!kData->edata.dataB){
                  kData->edata.dataB = (void*) malloc((size_t) bsreadPV->bsdata.bs_string.length());
                }
                if (kData->edata.dataSize!= bsreadPV->bsdata.bs_string.length()){
                 free(kData->edata.dataB);
                 kData->edata.dataB = (void*) malloc((size_t) bsreadPV->bsdata.bs_string.length());
                }

                memcpy(kData->edata.dataB, (char*) bsreadPV->bsdata.bs_string.toLatin1().constData()
                       ,(size_t) bsreadPV->bsdata.bs_string.length());

                break;
            }
            case bs_integer:{
                kData->edata.ivalue=bsreadPV->bsdata.bs_integer;
                kData->edata.fieldtype = caINT;
                break;
            }
            case bs_long:{
                kData->edata.ivalue=bsreadPV->bsdata.bs_long;
                kData->edata.fieldtype = caLONG;
                break;
            }
            case bs_short:{

                kData->edata.ivalue=bsreadPV->bsdata.bs_short;
                kData->edata.fieldtype = caINT;
                break;
            }
            }



            kData->edata.connected = true;
            kData->edata.accessR = true;
            kData->edata.accessW = false;
            kData->edata.monitorCount++;
            KnobData->SetMutexKnobData(kData->index, *kData);
            KnobData->SetMutexKnobDataReceived(kData);
        }
    }



}

bool bsread_Decode::bsread_DataMonitorConnection(knobData *kData){
    QString key = kData->pv;
    bool result=false;

    QMap<QString,bsread_channeldata*>::const_iterator i=ChannelSearch.find(key);
    while (i != ChannelSearch.end() && i.key() == key) {
        listOfIndexes.append(kData->index);
        result= true;
        ++i;
    }
    return result;
}

void *bsread_Decode::getZmqsocket() const
{
    return zmqsocket;
}
QString bsread_Decode::getConnectionPoint() const
{
    return ConnectionPoint;
}
bool bsread_Decode::getRunning_decode() const
{
    return running_decode;
}
MutexKnobData *bsread_Decode::getKnobData() const
{
    return KnobData;
}

void bsread_Decode::setKnobData(MutexKnobData *value)
{
    KnobData = value;
}





