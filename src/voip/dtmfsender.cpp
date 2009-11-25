// vim:tabstop=4:shiftwidth=4:foldmethod=marker:expandtab:cinoptions=(s,U1,m1
// Copyright (C) 2005
// Author Dmitry Poplavsky <dmitry.poplavsky@gmail.com>

//DKZM TEMP:for Sleep
//We need cross-platform sleep here
#ifdef WIN32
#include <windows.h>
#else
//this is Linux or Mac OS X (i.e.FreeBSD userspace)
#define _XOPEN_SOURCE 500
#include <unistd.h>
#endif

#include "dtmfsender.h"
#include <qtimer.h>



class DTMFSender::Private
{
public:
    QString sid;
    QString from;
    QString to;
    QString initiator;

    QString dtmfCode;
    XMPP::Client *client;



};

DTMFSender::DTMFSender(QObject * parent, QString sid, QString from, QString to, QString initiator)
: QObject( parent )
{
    d = new Private;
    d->sid = sid;
    d->from = from;
    d->to = to;
    d->initiator = initiator;
}

DTMFSender::~DTMFSender()
{
    delete d;
}

 
void DTMFSender::sendDTMF(QString dtmfCode, XMPP::Client *client)
{
    d->dtmfCode = dtmfCode;
    d->client = client;
    sendNextCode();
}

void DTMFSender::sendNextCode()
{
    if ( !d->dtmfCode.length() ) {
        deleteLater();
        return;
    }

    static int idNum = 0;
    idNum++;
    QString id = "dtmf"+QString::number(idNum);


    QString currentCode = d->dtmfCode.mid(0,1);
    d->dtmfCode = d->dtmfCode.mid(1);
    
	qDebug( "sending "+currentCode+" left "+d->dtmfCode );

    QString stanza = \
"<iq from='%1'\
  to='%2'\
  id='%3'\
  type='set'>\n\
  <gtalk xmlns='http://jabber.org/protocol/gtalk'\n\
          action='session-info'\n\
          initiator='%4'\n\
          sid='%5'>\n\
    <dtmf xmlns='http://jabber.org/protocol/gtalk/info/dtmf'\n\
        code='%6' action='%7' />\n\
  </gtalk>\n\
</iq>";

    
    stanza = stanza.arg(d->from)    //1
                .arg(d->to)         //2  
                .arg(id)            //3
                .arg(d->initiator)  //4
                .arg(d->sid)        //5 
                .arg(currentCode)   //6
                .arg( QString("button-down") ); // 7

   qDebug("Send DTMF stanza:") ;
    qDebug(stanza);

    d->client->send( stanza );
	//DKZM Temp Hack,Win32-only, linux support will be added later
#ifdef WIN32
	::Sleep(100);
#else
	::usleep(100*1000);//microseconds
#endif

	sendNextCode();
//  QTimer::singleShot( 100, this, SLOT(sendNextCode()) );
    
}

