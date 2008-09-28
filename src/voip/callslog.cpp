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

/* FIX!!!!!!!!!! THE WHOLE CODE HAS TO BE FIXED JUST MADE IT WORKING TO BE ABLE TO COMPILE AND TEST LIBJINGLE */

#include "callslog.h"

#include <qstringlist.h>
#include <qmap.h>
#include <qfile.h>
#include <qlistview.h>
#include <qcheckbox.h>
#include <qcombobox.h>

#include "psiiconset.h"

CallItem::CallItem() 
    :isAccepted(false), isValid(true)
{
    jid = "";
    direction = Incoming;
    duration = 0;
}


CallItem::CallItem( QString data )
{
    QStringList items = QStringList::split( " | ", data );

    if ( items.size() != 5 ) {
        isValid = false;
        return;
    }

    // format:  Jid | callTime | Direction | isAccepted | duration
    jid = items[0];
    callTime.setTime_t( items[1].toInt() );

    if ( items[2] == "Incoming" ) {
        direction = Incoming;
    } else if ( items[2] == "Outgoing" ) {
        direction = Outgoing;
    } else
        isValid = false;

    isAccepted = (items[3] == "Accepted");
    duration = items[4].toInt();

}

// for rejected calls
CallItem::CallItem( QString jid, QDateTime callTime, Direction direction )
    :isAccepted(false),duration(0), isValid(true)
{
    this->jid = jid;
    this->callTime = callTime;
    this->direction = direction;
}

// for accepted calls
CallItem::CallItem( QString jid, QDateTime callTime, Direction direction, int duration ) 
    :isAccepted(true), isValid(true)
{
    this->jid = jid;
    this->callTime = callTime;
    this->direction = direction;
    this->duration = duration;
}

QString CallItem::toString()
{
    if ( !isValid )
        return "";
    
    QString res = jid+" | ";
    res += QString().number( callTime.toTime_t() )+" | ";
    res += direction==Incoming ? "Incoming | ":"Outgoing | ";
    res += isAccepted ? "Accepted | ":"Rejected | ";
    res += QString().number( duration );

    return res;
}


/************************************************************************/
/********************         CallsLog           ************************/
/************************************************************************/


class CallsLog::Private
{
public:
    QString fileName;

};

CallsLog *callsLogInstance = 0;

CallsLog::CallsLog(QString fileName, QObject * parent, const char * name)
: QObject( parent, name )
{
    d = new Private;
    d->fileName = fileName;
    callsLogInstance = this;
}

CallsLog::~CallsLog()
{
    delete d;
}

CallsLog* CallsLog::instance()
{
    return callsLogInstance;
}

// changed due to compability issue with QT4 QViewList<CallItem> CallsLog::items() but still not working
QList<CallItem> CallsLog::items()
{
    QList<CallItem> res;
    
    QFile file(d->fileName);
    if ( !file.open( IO_ReadOnly ) ) {
        return res; // no log
    }
/* FIX THIS!!!!!!!!!!!!!!! We are not logging calls
    QString line;

    while ( file.readLine(line,1024) != -1 ) {
        CallItem item(line);
        res.append(item);
    }

    file.close();
*/
    return res;
    
}

void CallsLog::addItem( CallItem &item ) 
{
    QString itemString = item.toString();
    qDebug( "Log call: "+itemString );

    itemString += "\n";
    QByteArray itemData = itemString.utf8();
    itemData.resize( itemData.size()-1 );

    QFile file(d->fileName);
    file.open( IO_WriteOnly | IO_Append  );
    file.writeBlock( itemData );
    file.close();
}


/************************************************************************/
/********************         CallsLogDialog     ************************/
/************************************************************************/

/*
class CallItemView : public QListViewItem
{
public:
    CallItem item;

    
    CallItemView( QListView *parent, CallItem item )
        :QListViewItem(parent)
    {
        this->item = item;
        setMultiLinesEnabled(true);
        QString text = item.jid+"\n" + item.callTime.toString(LocalDate);
        setText(0, text );

        
//        setText(0, "   "+item.callTime.toString(LocalDate)+" " );
//        setText(1, item.jid+" " );
//        setText(2, item.direction == CallItem::Incoming? "From":"To" );
//        setText(3, item.isAccepted ? "Accepted":"Rejected" );
        
        //if ( item.isAccepted )
        //    setText(4, QString().number(item.duration) );
        
        setPixmap(0, IconsetFactory::iconPixmap("psi/calllogAccepted"));

        if (item.isAccepted) {
            if (item.direction == CallItem::Incoming)
                setPixmap(0, IconsetFactory::iconPixmap("psi/calllogAccepted"));
            else
                setPixmap(0, IconsetFactory::iconPixmap("psi/calllogToAccepted"));
        }
        else 
            setPixmap(0, IconsetFactory::iconPixmap("psi/calllogRejected"));
    }
    
};
*/
CallsLogDialog::CallsLogDialog( QWidget* parent, const char* name, bool modal )
    : CallsLogDialogBase( parent, name, modal )
{
    QMap<QString,int> jids; // list of available jids

/*    comboJids->insertItem("");*/

    listLogs->header()->hide();
    QList<CallItem> items = CallsLog::instance()->items();

    QList<CallItem>::iterator it;
    for ( it = items.begin(); it != items.end(); ++it ) {
/*        new CallItemView( listLogs, *it );*/

        QString jid = (*it).jid;
        if ( !jids.contains( jid ) ) {
            jids[jid] = 1;
//          comboJids->insertItem(jid);
        }
    }
}

CallsLogDialog::~CallsLogDialog()
{
}


void CallsLogDialog::showOnlyAccepted(bool)
{
    updateItems();
}

void CallsLogDialog::showCallsWith(const QString&)
{
    updateItems();
}


void CallsLogDialog::updateItems()
{
    bool filterAccepted = checkAccepted->isChecked(); // hide rejected calls if true
    QString filtercalls = combocalls->currentText(); // hide other jids if not empty
    
/*    CallItemView* viewItem = (CallItemView*)listLogs->firstChild();
    while ( viewItem ) {

        if ( filterAccepted && !viewItem->item.isAccepted ) {
            viewItem->setVisible(false);
        } else 
        if ( filtercalls != "All calls" ) {
            if (filtercalls == "Outgoing calls" && viewItem->item.direction == CallItem::Outgoing && viewItem->item.isAccepted) {
	            viewItem->setVisible(true);
			} else if (filtercalls == "Incoming calls" && viewItem->item.direction == CallItem::Incoming && viewItem->item.isAccepted) {
	            viewItem->setVisible(true);
			} else if (filtercalls == "Missed calls" && !viewItem->item.isAccepted) {
	            viewItem->setVisible(true);
			} else
            viewItem->setVisible(false);
        } else
            viewItem->setVisible(true);

        viewItem = (CallItemView*)viewItem->nextSibling();
    }
*/        
}

