/*
 * ahcformdlg.cpp - Ad-Hoc Command Form Dialog
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

#include <QLayout>
#include <QPushButton>
#include <QLabel>

#include "ahcformdlg.h"
#include "ahcommand.h"
#include "ahcexecutetask.h"
#include "xdata_widget.h"
#include "xmpp_client.h"
#include "psiaccount.h"
#include "busywidget.h"

AHCFormDlg::AHCFormDlg(const AHCommand& r, const Jid& receiver, XMPP::Client* client, bool final)
	: QDialog(NULL), receiver_(receiver), client_(client)
{
	setAttribute(Qt::WA_DeleteOnClose);

	// Save node
	node_ = r.node();
	sessionId_ = r.sessionId();

	QVBoxLayout *vb = new QVBoxLayout(this, 11, 6);

	// Instructions
	if (!r.data().instructions().isEmpty()) {
		QLabel* lb_instructions = new QLabel(r.data().instructions(),this);
		vb->addWidget(lb_instructions);
	}
	
	// XData form
	xdata_ = new XDataWidget(this);
	xdata_->setFields(r.data().fields());
	vb->addWidget(xdata_);

	vb->addStretch(1);

	// Buttons
	QHBoxLayout *hb = new QHBoxLayout(vb);
	pb_complete = pb_cancel = pb_prev = pb_next = 0;
	if (!final) {

		busy_ = new BusyWidget(this);
		hb->addWidget(busy_);
		hb->addItem(new QSpacerItem(20,0,QSizePolicy::Expanding));

		if (r.actions().empty()) {
			// Single stage dialog
			pb_complete = new QPushButton(tr("Finish"),this);
			connect(pb_complete,SIGNAL(clicked()),SLOT(doExecute()));
			hb->addWidget(pb_complete);
		}
		else {
			// Multi-stage dialog

			// Previous
			pb_prev = new QPushButton(tr("Previous"),this);
			if (r.actions().contains(AHCommand::Prev)) {
				if (r.defaultAction() == AHCommand::Prev) {
					pb_prev->setDefault(true);
					pb_prev->setFocus();
				}
				connect(pb_prev,SIGNAL(clicked()),SLOT(doPrev()));
				pb_prev->setEnabled(true);
			}
			else 
				pb_prev->setEnabled(false);
			hb->addWidget(pb_prev);

			// Next
			pb_next = new QPushButton(tr("Next"),this);
			if (r.actions().contains(AHCommand::Next)) {
				if (r.defaultAction() == AHCommand::Next) {
					connect(pb_next,SIGNAL(clicked()),SLOT(doExecute()));
					pb_next->setDefault(true);
					pb_next->setFocus();
				}
				else
					connect(pb_next,SIGNAL(clicked()),SLOT(doNext()));
				pb_next->setEnabled(true);
			}
			else {
				pb_next->setEnabled(false);
			}
			hb->addWidget(pb_next);
			
			// Complete
			pb_complete = new QPushButton(tr("Finish"),this);
			if (r.actions().contains(AHCommand::Complete)) {
				if (r.defaultAction() == AHCommand::Complete) {
					connect(pb_complete,SIGNAL(clicked()),SLOT(doExecute()));
					pb_complete->setDefault(true);
					pb_complete->setFocus();
				}
				else
					connect(pb_complete,SIGNAL(clicked()),SLOT(doComplete()));
				pb_complete->setEnabled(true);
			}
			else {
				pb_complete->setEnabled(false);
			}
			hb->addWidget(pb_complete);
		}
		pb_cancel = new QPushButton(tr("Cancel"), this);
		connect(pb_cancel, SIGNAL(clicked()),SLOT(doCancel()));
		hb->addWidget(pb_cancel);
	}
	else {
		hb->addItem(new QSpacerItem(20,0,QSizePolicy::Expanding));
		
		pb_complete = new QPushButton(tr("Ok"),this);
		connect(pb_complete,SIGNAL(clicked()),SLOT(close()));
		hb->addWidget(pb_complete);
	}

	if (!r.data().title().isEmpty()) {
		setCaption(QString("%1 (%2)").arg(r.data().title()).arg(receiver.full()));
	}
	else {
		setCaption(QString("%1").arg(receiver.full()));
	}
}

void AHCFormDlg::doPrev()
{
	busy_->start();
	AHCExecuteTask* t = new AHCExecuteTask(receiver_,AHCommand(node_,data(),sessionId_,AHCommand::Prev), client_->rootTask());
	connect(t,SIGNAL(finished()),SLOT(commandExecuted()));
	t->go(true);
}

void AHCFormDlg::doNext()
{
	busy_->start();
	AHCExecuteTask* t = new AHCExecuteTask(receiver_,AHCommand(node_,data(),sessionId_,AHCommand::Next),client_->rootTask());
	connect(t,SIGNAL(finished()),SLOT(commandExecuted()));
	t->go(true);
}

void AHCFormDlg::doExecute()
{
	busy_->start();
	AHCExecuteTask* t = new AHCExecuteTask(receiver_,AHCommand(node_,data(),sessionId_),client_->rootTask());
	connect(t,SIGNAL(finished()),SLOT(commandExecuted()));
	t->go(true);
}

void AHCFormDlg::doComplete()
{
	busy_->start();
	AHCExecuteTask* t = new AHCExecuteTask(receiver_,AHCommand(node_,data(),sessionId_,AHCommand::Complete), client_->rootTask());
	connect(t,SIGNAL(finished()),SLOT(commandExecuted()));
	t->go(true);
}

void AHCFormDlg::doCancel()
{
	busy_->start();
	AHCExecuteTask* t = new AHCExecuteTask(receiver_,AHCommand(node_,sessionId_,AHCommand::Cancel), client_->rootTask());
	connect(t,SIGNAL(finished()),SLOT(commandExecuted()));
	t->go(true);
}

void AHCFormDlg::commandExecuted()
{
	busy_->stop();
	close();
}

XData AHCFormDlg::data() const
{
	XData x;
	x.setFields(xdata_->fields());
	x.setType(XData::Data_Submit);
	return x;
}

