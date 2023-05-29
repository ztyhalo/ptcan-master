#include "netprint.h"
#include <assert.h>
#include <stdarg.h>
#include <time.h>
#include "zprint.h"

using namespace std;

Print_Server * gNetP=NULL;

UPDATE_SOCKET::UPDATE_SOCKET(quint16 sendp , quint16 recvp)
    :upsendport(sendp), uprecport(recvp)
{
    upsocket =  new QUdpSocket(this);
    if(upsocket == NULL)
    {
        qDebug("update socket create failed!");
    }

    qDebug("bind 0x%x %d", uprecport, upsocket->bind(uprecport, QUdpSocket::ShareAddress));
    connect(upsocket, SIGNAL(readyRead()), this, SLOT(updateDataProcess()));
}


void UPDATE_SOCKET::updateDataProcess()
{

    QByteArray datagram;
    while (upsocket->hasPendingDatagrams()) {

        QHostAddress sender;
        quint16 port;
        datagram.resize(upsocket->pendingDatagramSize());
        upsocket->readDatagram(datagram.data(), datagram.size(), &sender, &port);
//        qDebug() << "udp rev callback " << port;
         up_data_process(datagram, sender);
//        emit data_receive( datagram, sender);
    }
}
int UPDATE_SOCKET::up_data_process(QByteArray data, QHostAddress host)
{
    qDebug() << data;
    return 0;
}


int UPDATE_SOCKET::send_data(QString msg)
{
    return upsocket->writeDatagram(msg.toUtf8(), QHostAddress::Broadcast, upsendport);
}

int UPDATE_SOCKET::send_data(QString msg, QString ip)
{
    return upsocket->writeDatagram(msg.toUtf8(), QHostAddress(ip), upsendport);
}

int UPDATE_SOCKET::send_data(QString msg, QHostAddress ip)
{
    return upsocket->writeDatagram(msg.toUtf8(), ip, upsendport);
}

Print_Server::Print_Server(quint16 sendp, quint16 recvp):
    UPDATE_SOCKET(sendp, recvp)
{
    constate = 0;
    sem_init(&psem, 0, 0);
}


int Print_Server::up_data_process(QByteArray data, QHostAddress host)
{
    qDebug() << "data receive!";
    QString msg = data;
    if(msg == "start printf!")
    {
        zprintf1("net printf start!\n");
        constate = 1;
        printhost = host;
        this->start();
    }
    else if (msg == "stop printf!")
    {
        zprintf1("net printf stop!\n");
        constate = 0;
    }
    return 0;
}

void  Print_Server::netprintf(const char * format, ...)
{

    if(constate == 0) return;
    va_list args;
//     lock();

    struct timeval tv;
    struct Print_Time tm;
    gettimeofday(&tv, NULL);
    tm.tv = tv;
    va_start(args, format);

    char buf[1024];
    memset(buf, 0, 1024);
    (void) vsprintf(buf, format, args);
    va_end(args);
    QString mesg(buf);
    lock();
    info.insert(tm, mesg);
    unlock();
    sem_post(&psem);


//    unlock();

}

void Print_Server::run()
{
    char buf[1024];
    struct timeval tv;
    QString        mg;

    while (1) {

       sem_wait(&(this->psem));

       lock();
       QMap<struct Print_Time, QString >::iterator iter = info.begin();

      if(iter != info.end())
      {
//           struct timeval
            tv = iter.key().tv;
//            QString mesg(buf);
            mg = iter.value();
            info.erase(iter);
      }
      unlock();
       struct tm *p;
      p = localtime(&tv.tv_sec);
      memset(buf, 0, 1024);

      sprintf(buf,"%d-%02d-%02d %02d:%02d:%02d.%06ld ",
             1900+p->tm_year, 1+p->tm_mon, p->tm_mday,
             p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec);
      QString mesg(buf);
      send_data(mesg + " " + mg);
    }
}

Print_Client::Print_Client(quint16 sendp , quint16 recvp)
    :UPDATE_SOCKET(sendp, recvp)
{
    send_data("start printf!");
}


int Print_Client::up_data_process(QByteArray data, QHostAddress host)
{
    QString msg = data;
    qDebug() << msg;
    return 0;
}


//void  nprintf(const char * format, ...)
//{
//    if(gNetP != NULL)
//    {

//        va_list args;
//        va_start(args, format);
//        gNetP->netprintf(format, args);
//        va_end(args);
//    }
//}
