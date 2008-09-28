/* vim:tabstop=4:shiftwidth=4:expandtab:cinoptions=(s,U1,m1
 * Copyright (C) 2005 Dmitry Poplavsky <dima@thekompany.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "tins.h"
#include "sdp.h"

#include "../src/common.h"
#include "callslog.h"

#include "mediastream.h"


#include "udp.h"
#include "stun.h"

#include <qpushbutton.h>
#include <qprogressbar.h>
#include <qmap.h>

#include <stdlib.h>
#include <time.h>

#ifdef TINS

class TINSSession::Private
{
public:
    State state;
    QString localSDP, remoteSDP;
    Jid localJid, remoteJid;
    PsiAccount *account;
    QString thread;

    MediaStream *ms;
    CallEvent *ce;

    QMap<int,QString> responseStrings;

    bool rtp_active;

    CallItem logItem;
};


TINSSession::TINSSession(QObject * parent, PsiAccount *account, Jid &remoteJid)
: QObject( parent )
{
    d = new Private;
    d->state = Initial;
    d->account = account;
    d->localJid = account->jid();
    d->remoteJid = remoteJid;
    d->ms = new MediaStream();
    d->rtp_active = false;
    d->ce = 0L;

    d->logItem.callTime = QDateTime::currentDateTime();
    d->logItem.jid = remoteJid.full();

    d->responseStrings[200] = "OK";
    d->responseStrings[100] = "Trying";
    d->responseStrings[180] = "Trying";
    d->responseStrings[182] = "Queued";

    
    connect(this, SIGNAL(startRTPTransfer(QString, QString)), 
            d->ms,  SLOT(start(QString, QString)));
    connect(this, SIGNAL(stopRTPTransfer()), 
            d->ms,  SLOT(stop()));
}

TINSSession::~TINSSession()
{
    delete d->ms;
    delete d;
}

// <thread> child of <message>, used to track session related
// messages.
QString TINSSession::thread()
{
    return d->thread;
}

void TINSSession::setThread(QString thread)
{
    d->thread = thread;
}

MediaStream * TINSSession::mediaStream()
{
    return d->ms;
}

TINSSession::State TINSSession::state()
{
    return d->state;
}


void TINSSession::processMessage(const Message &message )
{
    Message m(message);
    TinsParams params = m.tinsParams();
    QString method = params.method;
    int result = params.result;

    if ( result > 0 )
        emit logReceive( QString("Received: result:%1 %2").arg(result).arg( d->responseStrings[result] ) );
    else
        emit logReceive( QString("Received: %1").arg(method) );
    
    //  ********** Initial state  ***********
    if ( state() == Initial ) {
        // only INVITE possible
        if ( method == "INVITE" ) {
            d->remoteSDP = params.SDPData;
            setState(Ringing);
            sendMessage(180); // Ringing

            d->logItem.direction = CallItem::Incoming;

            d->ce = new CallEvent(this, d->remoteJid, d->account);
            d->account->handleEvent(d->ce);
            return;
        }

        logUnexpected("Initial", method, result);
        return;
    }

    //  ********** Inviting state  ***********
    if ( state() == Inviting ) {
        // result and BYE possible
        if ( method == "BYE" ) {
            setState(Finished);
            sendMessage(200); // Ok
            emit remoteSentBye();
            return;
        }

        if ( result == 100 ) {
            // Trying
            emit stateMessage( "Trying" );
            emit logMessage( "Trying" );
            return;
        }

        if ( result == 180 ) {
            // Ringing
            emit stateMessage( "Ringing" );
            emit logMessage( "Ringing" );
            return;
        }

        if ( result/100 == 1 ) {
            // other 1xx results
            emit logMessage("result: "+QString::number(result));
            return;
        }


        if ( result == 200 ) {
            // Ok
            emit stateMessage( "Accepted" );
            emit logMessage( "Accepted" );
            sendMessage("ACK");
            
            d->remoteSDP = params.SDPData;
            SDP remoteSDP( d->remoteSDP );
            emit detectedRemoteHost(remoteSDP.host, remoteSDP.port);

            setState(Talking);
            emit logMessage("start session\nLocal SDP:\n"+d->localSDP+"\nRemote SDP:\n"+d->remoteSDP+"\n\n");
            emit startRTPTransfer( d->localSDP, d->remoteSDP );
            d->rtp_active = true;
            return;
        }

        //TODO process another result codes
        
        
        logUnexpected("Inviting", method, result);
        return;
    }

    //  ********** Ringing state  ***********
    if ( state() == Ringing ) {
        // only BYE possible
        if ( method == "BYE" ) {
            setState(Finished);
            sendMessage(200); // Ok
            emit remoteSentBye();
            if (d->ce) {
                d->account->dequeueEvent(d->ce);
                d->ce->stopTimer();
                d->ce = 0L;
            }
            return;
        }

        logUnexpected("Ringing", method, result);
        return;
    }

    //  ********** WaitingForACK state  ***********
    if ( state() == WaitingForACK ) {
        // only ACK possible
        if ( method == "ACK" ) {
            setState(Talking);
            emit logMessage("start session\nLocal SDP:\n"+d->localSDP+"\nRemote SDP:\n"+d->remoteSDP+"\n\n");
            emit startRTPTransfer( d->localSDP, d->remoteSDP );
            d->rtp_active = true;
            return;
        }

        logUnexpected("WaitingForACK", method, result);
        return;
    }

    //  ********** Talking state  ***********
    if ( state() == Talking ) {
        // only BYE possible
        if ( method == "BYE" ) {
            setState(Finished);
            if (d->rtp_active) {
                emit stopRTPTransfer();
                d->rtp_active = false;
            }
            sendMessage(200); // Ok
            emit remoteSentBye();
            return;
        }

        logUnexpected("Talking", method, result);
        return;
    }

    //  ********** SentBye state  ***********
    if ( state() == SentBye ) {
        // only 200 result is possible
        if ( result == 200 ) {
            setState(Finished);
            return;
        }

        return;
    }


}

void TINSSession::logUnexpected(QString state, QString method, int result)
{
    emit logMessage(
        QString("%3: Unexpected TINS message in %1 state, mode:%2").arg(state).arg(method).arg(result)
    );
}

/*! one of accept()/reject() should be called in the Ringing state */
void TINSSession::accept()
{
    d->localSDP = TINSSessionManager::instance()->localSDP( d->account );
    TinsParams params;
    params.method = "result";
    params.result = 200;
    params.SDPData = d->localSDP;

    sendMessage( params );
    setState( WaitingForACK );
}

void TINSSession::reject()
{
    bye();
}

/*! can be called in Inviting, Ringing, Talking state to terminate connection */
void TINSSession::bye()
{
    sendMessage( QString("BYE") );
    if (d->rtp_active) {
        emit stopRTPTransfer();
        d->rtp_active = false;
    }
    setState( SentBye );
}

void TINSSession::sendInvite()
{
    d->localSDP = TINSSessionManager::instance()->localSDP( d->account );

    d->logItem.direction = CallItem::Outgoing;
    
    TinsParams params;
    params.method = "INVITE";

    params.SDPData = d->localSDP;
    
    sendMessage( params );
    setState( Inviting );
}

void TINSSession::sendACK()
{
    sendMessage( QString("ACK") );
    emit logMessage("start session\nLocal SDP:\n"+d->localSDP+"\n\nRemote SDP:\n"+d->remoteSDP+"\n\n");
    emit startRTPTransfer( d->localSDP, d->remoteSDP );
    d->rtp_active = true;
    setState( Talking );
}

void TINSSession::sendMessage( TinsParams &params )
{
    Message m( d->remoteJid );
    m.setTinsParams(params);
    m.setFrom( d->localJid );
    m.setThread( thread() );
    d->account->dj_sendMessage(m, false);
    
    if ( params.result > 0 ) 
        emit logSend(QString("Sent: result:%1").arg(params.result));
    else
        emit logSend(QString("Sent: TINS: %1").arg(params.method));
}
    
void TINSSession::sendMessage( QString method )
{
    TinsParams params;
    params.method = method;
    params.result = 0;
    sendMessage(params);
}

void TINSSession::sendMessage( int result )
{
    TinsParams params;
    params.method = "result";
    params.result = result;
    sendMessage(params);
}

void TINSSession::setState( TINSSession::State state )
{
    d->state = state;
    emit stateChanged(state);

    // call is accepted, let's save this for logs
    if ( state == Talking ) {
        d->logItem.callTime = QDateTime::currentDateTime();
        d->logItem.isAccepted = true;
    }
    
    if ( state == Finished ) {
        if ( d->logItem.isAccepted ) {
            // calculate call duration
            d->logItem.duration = d->logItem.callTime.secsTo( 
                    QDateTime::currentDateTime() );
        } else 
            d->logItem.duration = 0;

        emit logDataReady(d->logItem);
    }
    

    QStringList stateNames;
    stateNames << "Initial" << "Inviting" << "Ringing";
    stateNames << "WaitingForACK" << "Talking" << "SentBye" << "Finished";
    
    emit logMessage("setState:"+stateNames[state]+" "+d->localJid.full());
}
 

/*****************************************************************************/
/*                    TINSSessionManager implementation                      */
/*****************************************************************************/

TINSSessionManager *tinsSessionManagerInstance = 0;

struct StunResult {
    QString host;
    int port;
    NatType natType;
};

class TINSSessionManager::Private
{
public:
    QDict<TINSSession> sessions;
    QString stunServer;

    CallsLog *callsLog;

    StunResult requestStunServer( PsiAccount *account, QString stunServer );
    bool isBehindNat( QString host );
};

TINSSessionManager::TINSSessionManager(QObject * parent, const char * name)
: QObject( parent, name )
{
    d = new Private;
    tinsSessionManagerInstance = this;
    d->stunServer = "stunserver.org";
    d->callsLog = new CallsLog( getHistoryDir() + "/tins_calls.log", this );
}

TINSSessionManager::~TINSSessionManager()
{
    delete d;
}


TINSSessionManager* TINSSessionManager::instance()
{
    return tinsSessionManagerInstance;
}


TINSSession *TINSSessionManager::tinsSession( QString thread )
{
    //QString key = localJid.full() + '-' + remoteJid.full();
    QString key = thread;
    return d->sessions[key];
}

TINSSession *TINSSessionManager::createSession( PsiAccount *account, Jid &remoteJid )
{
    Jid localJid = account->jid();
    //QString key = localJid.full() + '-' + remoteJid.full();
    srand( time(0) );
    QString thread;
    thread.setNum( rand() % 10000000 );

    qDebug( QString("create tins session: ")+thread);
    
    TINSSession *session = new TINSSession(this, account, remoteJid);
    session->setThread( thread );
    d->sessions.insert(thread, session );

    connect( session, SIGNAL(logDataReady(CallItem&)),
             d->callsLog, SLOT(addItem(CallItem&)) );

    //connect( session, SIGNAL(aSend(const Message &, bool)), 
    //         account, SLOT(dj_sendMessage(const Message &, bool)) );

    return session;
}

QString TINSSessionManager::localSDP( PsiAccount *account )
{
    SDP sdp;
    sdp.sessionName = "Tins session";
    sdp.owner = account->jid().full();
    sdp.host= account->localAddress()->toString();
    sdp.port= 8082;

    if ( d->isBehindNat( sdp.host ) ) {
        StunResult result = d->requestStunServer( account, d->stunServer );
        sdp.host = result.host;
        sdp.port = result.port;
    }

    sdp.formats.append(97);
    sdp.formats.append(0);
    
    return sdp.toString();
}


void TINSSessionManager::processMessage( PsiAccount *account, const Message &m )
{
    Jid localJid = m.to();
    Jid remoteJid = m.from();
    QString thread = m.thread();
    
    TINSSession *session = tinsSession( thread );

    if ( !session ) {
        session = new TINSSession(this, account, remoteJid);
        session->setThread( thread );
        d->sessions.insert(thread, session );
        connect( session, SIGNAL(logDataReady(CallItem&)),
                 d->callsLog, SLOT(addItem(CallItem&)) );
    }

    //connect( session, SIGNAL(aSend(const Message &, bool)), 
    //         account, SLOT(dj_sendMessage(const Message &, bool)) );

    session->processMessage(m);
}

bool TINSSessionManager::Private::isBehindNat( QString host )
{
    if ( host.startsWith("10.") )
        return true;

    if ( host.startsWith("192.168.") )
        return true;
    Q_UINT32 addr = QHostAddress(host).toIPv4Address();

    if ( addr > QHostAddress("172.16.0.0").toIPv4Address() &&
         addr < QHostAddress("172.31.0.0").toIPv4Address() )
        return true;
    
    return false;
}

StunResult TINSSessionManager::Private::requestStunServer( PsiAccount *, QString stunServer )
{
    StunResult result;
    result.host = "";
    result.port = 0;
    result.natType = StunTypeBlocked;

    initNetwork();
    StunAddress4 stunServerAddr;
    stunServerAddr.addr=0;

    int srcPort=8082;
    StunAddress4 sAddr;

    //stunParseServerName( (char*)host->latin1(), sAddr);

    sAddr.addr = 0;
    sAddr.port = srcPort;

    qDebug("stun request:");
    bool ret = stunParseServerName( (char*)stunServer.latin1() , stunServerAddr);
    if ( ret != true ) {
        qDebug( stunServer + " is not a valid host name ");
        return result;
    }

    bool presPort = false;
    bool hairpin = false;
    bool verbose = true;
    
    result.natType = stunNatType( stunServerAddr, verbose, &presPort, &hairpin, 
                                  srcPort, &sAddr);

    result.port = sAddr.port;

    UInt32 ip = sAddr.addr;
    //qDebug("addr:%i", ip );
    qDebug("nat type:%i", result.natType);
    result.host = QString("%1.%2.%3.%4").arg((ip>>24)&0xFF) \
                                  .arg((ip>>16)&0xFF) \
                                  .arg((ip>>8)&0xFF) \
                                  .arg((ip>>0)&0xFF);
    
    qDebug("external address %s:%d", result.host.latin1(), result.port );
    return result;
}

#endif

