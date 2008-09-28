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

#ifndef TINSSESSION_H
#define TINSSESSION_H

#include "im.h"
#include "psievent.h"
#include "psiaccount.h"
#include "mediastream.h"
#include "callslog.h"
using namespace XMPP;


/*!
 \class TINSSession tins.h
 
 Client calls to server.

    

       Client                         Server
    State:Inviting                 State:Ringing
       ||                                 ||
       ||------- Invite + SDP ----------->||
       ||<------- 100: Trying ------------|| // optional
       ||<------- 180: Ringing -----------||
       ||<------- 200: Ok + SDP ----------||
       ||                          State:WaitingForACK
       ||--------     ACK      ---------->||
    State:Talking                  State:Talking
       ||<----- RTP Voice data ---------->||
           ............................
       ||----------- BYE ---------------->||
       ||<------- 200: Ok ----------------||
       ||            or                   ||
       ||<---------- BYE -----------------||
       ||                          State:SentBye
       ||-------- 200: Ok --------------->||
    State:Finished                 State:Finished
         
        
 */

#ifdef TINS

class TINSSession : public QObject {
Q_OBJECT
public:
    typedef enum {  Initial=0, Inviting=1, Ringing=2, 
                    WaitingForACK=3, Talking=4, SentBye=5, Finished=6 } State;  
public:
	TINSSession(QObject * parent, PsiAccount *account, Jid &remoteJid );
	virtual ~TINSSession();

    State state();
    PsiAccount *account();

    // <thread> child of <message>, used to track session related
    // messages.
    QString thread();
    void setThread(QString thread);

    //mediastream
    MediaStream * mediaStream();

public slots:
    void processMessage( const Message &m );

    /*! one of accept()/reject() should be called in the Ringing state */
    void accept();
    void reject();
    
    /*! can be called in Inviting, Ringing, Talking state to terminate connection */
    void bye();
    
    void sendInvite();
    void sendACK();
    
    /*
    void hideLevelsDialog();
    void showLevelsDialog();
    void showMicLevel( int mid, int max );
    void showDspLevel( int mid, int max );
    */

signals:
    void stateChanged( TINSSession::State );
    void stateMessage( QString );

    void startRTPTransfer( QString localSDP, QString remoteSDP );
    void stopRTPTransfer();

    void detectedRemoteHost(QString host, int port);
    void remoteSentBye();

    void logSend(const QString &);
    void logReceive(const QString &);
    void logMessage(const QString &);

    void logDataReady( CallItem& );
    
private:
    void logUnexpected(QString, QString, int);
    void sendMessage( TinsParams & );
    void sendMessage( QString method );
    void sendMessage( int result );

    void setState( State );
	
    
    class Private;
	Private *d;
}; 


/*!
 \class TINSSessionManager tinssessionmanager.h
 \brief stub
 */

class TINSSessionManager : public QObject {
Q_OBJECT
public:
	TINSSessionManager(QObject * parent = 0L, const char * name = 0L);
	virtual ~TINSSessionManager();

    static TINSSessionManager * instance();

    //TINSSession *tinsSession( Jid &localJid, Jid &remoteJid );
    //TINSSession *createSession( PsiAccount *account, Jid &remoteJid );
    TINSSession *tinsSession( QString thread );
    TINSSession *createSession( PsiAccount *account, Jid &remoteJid );

    QString localSDP( PsiAccount* );
    
    // pass message to according session, and create session if needed
    void processMessage( PsiAccount *account, const Message &m );

private:
	class Private;
	Private *d;
}; 

#endif // TINS

#endif // TINSSESSION_H


