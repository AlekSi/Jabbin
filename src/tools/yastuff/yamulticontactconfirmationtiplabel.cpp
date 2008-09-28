/*
 * yamulticontactconfirmationtiplabel.cpp
 * Copyright (C) 2008  Yandex LLC (Michail Pishchagin)
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

#include "yamulticontactconfirmationtiplabel.h"

#include <QEvent>
#include <QDesktopServices>
#include <QTimer>
#include <QPainter>
#include <QTextDocument>
#include <QKeyEvent>
#include <QStandardItemModel>

#include "yaprofile.h"
#include "psiaccount.h"
#include "psicontact.h"
#include "psiiconset.h"
#include "xmpp_vcard.h"
#include "xmpp_tasks.h"
#include "vcardfactory.h"
#include "userlist.h"
#include "psioptions.h"
#include "yacommon.h"
#include "yacontactlabel.h"
#include "yavisualutil.h"
#include "lastactivitytask.h"
#include "yarostertooltip.h"
#include "yacontactlistmodel.h"
#include "yarostertiplabel.h"

YaMultiContactConfirmationTipLabel::YaMultiContactConfirmationTipLabel(QWidget* parent)
	: PsiTipLabel(parent)
	, tipPosition_(0)
	, contactSelection_(0)
	, model_(0)
	, leaveTimer_(new YaRosterTipLabelLeaveTimer(this))
{
	connect(leaveTimer_, SIGNAL(hideTip()), SLOT(hideTip()));
	installEventFilter(this);

	model_ = new QStandardItemModel(this);
}

YaMultiContactConfirmationTipLabel::~YaMultiContactConfirmationTipLabel()
{
	setContactSelection(0);
	setTipPosition(0);
}

void YaMultiContactConfirmationTipLabel::setTipPosition(ToolTipPosition* tipPosition)
{
	if (tipPosition_)
		delete tipPosition_;
	tipPosition_ = tipPosition;
}

void YaMultiContactConfirmationTipLabel::setRealModel(YaContactListModel* model)
{
	realModel_ = model;
}

void YaMultiContactConfirmationTipLabel::setContactSelection(QMimeData* contactSelection)
{
	if (contactSelection_)
		delete contactSelection_;
	contactSelection_ = contactSelection;
	invalidateData();
}

void YaMultiContactConfirmationTipLabel::setListView(QWidget* listView)
{
	setRemoveContactConfirmationEnabled(!listView);
	leaveTimer_->setListView(listView);
}

void YaMultiContactConfirmationTipLabel::invalidateData()
{
	if (!initialized_ || !contactSelection_)
		return;

	setUpdatesEnabled(false);

	model_->clear();

	QList<int> roles;
	roles
		<< Qt::DisplayRole
		<< ContactListModel::TypeRole
		<< ContactListModel::ActivateRole
		<< ContactListModel::JidRole
		<< ContactListModel::PictureRole
		<< ContactListModel::StatusTextRole
		<< ContactListModel::StatusTypeRole;

	foreach(PsiContact* contact, realModel_->contactsLostByRemove(contactSelection_)) {
		QStandardItem* item = new QStandardItem();
		foreach(int role, roles) {
			QVariant data = realModel_->contactData(contact, role);
			// TODO: or maybe display jid instead of StatusTextRole?
			if (role == Qt::DisplayRole) {
				QString name = data.toString();
				if (name != contact->jid().full())
					name = tr("%1 (%2)").arg(name, contact->jid().full());
				data = QVariant(name);
			}
			item->setData(data, role);
		}
		model_->invisibleRootItem()->appendRow(item);
	}

	updateSize();
	setUpdatesEnabled(true);
}

void YaMultiContactConfirmationTipLabel::initUi()
{
	ui_.setupUi(this);
	ui_.actionButtonsStackedWidget->init();
	ui_.actionButtonsStackedWidget->setMainWidget(ui_.actionButtonsPage);
	ui_.actionButtonsPage->installEventFilter(this);
	ui_.deleteConfirmationPage->installEventFilter(this);

	ui_.contactListView->setModel(model_);

	connect(ui_.deleteButton, SIGNAL(clicked()), SLOT(deleteSelection()));

	QDialogButtonBox* box = ui_.deleteConfirmationButtonBox;
	confirmationDeleteButton_ = box->addButton(tr("Delete"), QDialogButtonBox::DestructiveRole);
	confirmationCancelButton_ = box->addButton(QDialogButtonBox::Cancel);
	connect(confirmationDeleteButton_, SIGNAL(clicked()), SLOT(confirmationDelete()));
	connect(confirmationCancelButton_, SIGNAL(clicked()), SLOT(confirmationCancel()));
	confirmationDeleteButton_->setDefault(true);

	initialized_ = true;
}

void YaMultiContactConfirmationTipLabel::setRemoveContactConfirmationEnabled(bool removeContactConfirmationEnabled)
{
	ui_.actionButtonsStackedWidget->setCurrentWidget(removeContactConfirmationEnabled ?
	        ui_.deleteConfirmationPage : ui_.actionButtonsPage);
	if (removeContactConfirmationEnabled)
		ui_.deleteConfirmationButtonBox->setFocus();
}

void YaMultiContactConfirmationTipLabel::deleteSelection()
{
	setRemoveContactConfirmationEnabled(true);
}

void YaMultiContactConfirmationTipLabel::confirmationDelete()
{
	Q_ASSERT(contactSelection_);
	emit removeContactConfirmation(contactSelection_);
	hideTip();
}

void YaMultiContactConfirmationTipLabel::confirmationCancel()
{
	setRemoveContactConfirmationEnabled(false);
	if (!underMouse())
		hideTip();
}

void YaMultiContactConfirmationTipLabel::updateSize()
{
	setUpdatesEnabled(false);
	layout()->activate();
	// FIXME: think of something more graceful
	resize(sizeHint());
	if (tipPosition_)
		move(tipPosition_->calculateTipPosition(this, size()));
	setUpdatesEnabled(true);
}

QSize YaMultiContactConfirmationTipLabel::sizeHint() const
{
	return QFrame::sizeHint();
}

bool YaMultiContactConfirmationTipLabel::eventFilter(QObject* obj, QEvent* e)
{
	if (e->type() == QEvent::Paint && (obj == ui_.actionButtonsPage || obj == ui_.deleteConfirmationPage)) {
		QWidget* w = static_cast<QWidget*>(obj);
		QPainter p(w);
		p.fillRect(w->rect(), obj == ui_.actionButtonsPage ? Ya::VisualUtil::highlightBackgroundColor() : Ya::VisualUtil::deleteConfirmationBackgroundColor());
		return true;
	}

	if (e->type() == QEvent::KeyPress && ui_.actionButtonsStackedWidget->currentWidget() == ui_.deleteConfirmationPage) {
		QKeyEvent* ke = static_cast<QKeyEvent*>(e);
		if (ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return){
			confirmationDeleteButton_->animateClick();
		}
		if (ke->key() == Qt::Key_Escape) {
			confirmationCancelButton_->animateClick();
		}
		return true;
	}

	if (!leaveTimer_->listView()) {
		switch (e->type()) {
		case QEvent::KeyRelease:
		case QEvent::ContextMenu:
		case QEvent::FocusIn:
		case QEvent::FocusOut:
		case QEvent::WindowActivate:
		case QEvent::WindowDeactivate:
		case QEvent::MouseButtonPress:
		case QEvent::MouseButtonRelease:
		case QEvent::MouseButtonDblClick:
		case QEvent::Wheel:
		case QEvent::Leave:
		case QEvent::Enter:
			return false;
		default:
			;
		}
	}
	else {
		bool result;
		if (leaveTimer_->processEvent(obj, e, &result))
			return result;
	}
	return PsiTipLabel::eventFilter(obj, e);
}

void YaMultiContactConfirmationTipLabel::timerEvent(QTimerEvent* e)
{
	PsiTipLabel::timerEvent(e);
}

void YaMultiContactConfirmationTipLabel::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	p.fillRect(rect(), Ya::VisualUtil::highlightBackgroundColor());
}

void YaMultiContactConfirmationTipLabel::enterEvent(QEvent*)
{
	// we don't want our tooltip hidden as result of enterEvent,
	// so we don't call parent's enterEvent
}

void YaMultiContactConfirmationTipLabel::startHideTimer()
{
	// we don't want to be hidden so we do nothing
}

void YaMultiContactConfirmationTipLabel::resizeEvent(QResizeEvent*)
{
	int corner = 10;
	setMask(Ya::VisualUtil::roundedMask(size(), corner, Ya::VisualUtil::AllBorders));
}

void YaMultiContactConfirmationTipLabel::hideTip()
{
	PsiTipLabel::hideTip();
}
