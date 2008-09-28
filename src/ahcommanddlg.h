/*
 * ahcommanddlg.h - Ad-Hoc Command Dialog
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
 
#ifndef AHCOMMANDDLG_H
#define AHCOMMANDDLG_H

#include <QDialog>
#include <QList>

#include "busywidget.h"
#include "xmpp_jid.h"

namespace XMPP {
	class Client;
}

class PsiAccount;
class QObject;
class QComboBox;
class QPushButton;

typedef struct { QString jid, node, name; } AHCommandItem;

class AHCommandDlg : public QDialog
{
	Q_OBJECT

public:
	AHCommandDlg(PsiAccount*, const XMPP::Jid& receiver);
	static void executeCommand(XMPP::Client*, const XMPP::Jid& j, const QString &node);

protected slots:
	void refreshCommands();
	void listReceived();
	void executeCommand();
	void commandExecuted();

private:
	PsiAccount* pa_;
	QPushButton *pb_execute, *pb_close, *pb_refresh /*,*pb_info*/;
	XMPP::Jid receiver_;
	QComboBox* cb_commands;
	QList<AHCommandItem> commands_;
	BusyWidget* busy_;
};

#endif
