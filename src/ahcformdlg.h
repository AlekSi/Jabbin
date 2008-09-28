/*
 * ahcformdlg.h - Ad-Hoc Command Form Dialog
 * Copyright (C) 2005  Remko Troncon
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
 
#ifndef AHCFORMDLG_H
#define AHCFORMDLG_H

#include <QObject>
#include <QDialog>
#include <QString>

#include "xmpp_xdata.h"
#include "xmpp_jid.h"
#include "busywidget.h"

class QPushButton;
class AHCommand;
class XDataWidget;
namespace XMPP {
	class Client;
}

class AHCFormDlg : public QDialog
{
	Q_OBJECT

public:
	AHCFormDlg(const AHCommand& r, const XMPP::Jid& receiver, XMPP::Client* client, bool final = false);

protected:
	XMPP::XData data() const;

protected slots:
	void doPrev();
	void doNext();
	void doComplete();
	void doExecute();
	void doCancel();
	void commandExecuted();

private:
	QPushButton *pb_prev, *pb_next, *pb_complete, *pb_cancel;
	XDataWidget *xdata_;
	XMPP::Jid receiver_;
	QString node_;
	XMPP::Client* client_;
	QString sessionId_;
	BusyWidget* busy_;
};

#endif
