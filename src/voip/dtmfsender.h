// vim:tabstop=4:shiftwidth=4:expandtab:cinoptions=(s,U1,m1
// Copyright (C) 2005
// Author Dmitry Poplavsky <dmitry.poplavsky@gmail.com>

#ifndef DTMFSENDER_H
#define DTMFSENDER_H

#include "qobject.h"
#include "im.h"


/*!
 \class DTMFSender dtmfsender.h
 \brief stub
 */

class DTMFSender : public QObject {
Q_OBJECT
public:
	DTMFSender(QObject * parent, QString sid, QString from, QString to, QString initiator);
	virtual ~DTMFSender();

public slots:
    void sendDTMF(QString dtmfCode, XMPP::Client *client);

private slots:
    void sendNextCode();
private:
	class Private;
	Private *d;
}; 

#endif // DTMFSENDER_H


