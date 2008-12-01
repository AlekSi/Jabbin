//
// ivan: This file is no longer used
//
#error This file should not be used: calldlg.h

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

#ifndef CALLSLOG_H
#define CALLSLOG_H

#include <qobject.h>
#include <qdatetime.h>

#include "callslogdialogbase.h"

class CallItem {
public:
    typedef enum { Incoming, Outgoing } Direction;

    CallItem();
    CallItem( QString data ); // load from serialized string
    CallItem( QString jid, QDateTime callTime, Direction direction ); // for rejected calls
    CallItem( QString jid, QDateTime callTime, Direction direction,
              int duration ); // for accepted calls

    QString toString();

    QString jid;
    QDateTime callTime;
    Direction direction;
    bool isAccepted;
    int duration; // in seconds

    bool isValid;
};

/*!
 \class CallsLog callslog.h
 \brief stub
 */

class CallsLog : public QObject {
Q_OBJECT
public:
	CallsLog( QString fileName, QObject * parent = 0L, const char * name = 0L );
	virtual ~CallsLog();

    static CallsLog* instance();

// changed for QT4 compability     QValueList<CallItem> items();
	QList<CallItem> items();

public slots:
    void addItem( CallItem& );

private:
	class Private;
	Private *d;
};


class CallsLogDialog : public CallsLogDialogBase {
Q_OBJECT
public:
    CallsLogDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE );
    ~CallsLogDialog();

public slots:
    virtual void showOnlyAccepted(bool);
    virtual void showCallsWith( const QString& );

protected:
    void updateItems();

};

#endif // CALLSLOG_H


