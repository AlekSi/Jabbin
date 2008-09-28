/*
 * yamanageaccounts.cpp - account editor
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

#include "yamanageaccounts.h"

#include <QContextMenuEvent>
#include <QMenu>
#include <QPainter>

#include "psicon.h"
#include "yastyle.h"
#include "accountinformermodel.h"
#include "psiaccount.h"
#include "yaremoveconfirmationmessagebox.h"
#include "shortcutmanager.h"
#include "yapushbutton.h"

YaManageAccounts::YaManageAccounts(QWidget* parent)
	: QWidget(parent)
	, controller_(0)
	, removeAction_(0)
{
	// setStyle(YaStyle::defaultStyle());

	model_ = new AccountInformerModel(this);
	connect(model_, SIGNAL(rowsInserted(const QModelIndex&, int, int)), SLOT(updateRemoveAction()));
	connect(model_, SIGNAL(rowsRemoved(const QModelIndex&, int, int)), SLOT(updateRemoveAction()));

	xmlConsoleAction_ = new QAction(tr("XML Console"), this);
	connect(xmlConsoleAction_, SIGNAL(triggered()), SLOT(xmlConsole()));

	ui_.setupUi(this);
	ui_.treeView->installEventFilter(this);
	connect(ui_.treeView, SIGNAL(deleteAccount()), SLOT(deleteAccount()));

	background_ = YaWindowBackground(YaWindowBackground::Roster);

	removeAction_ = new QAction(this);
	removeAction_->setShortcuts(ShortcutManager::instance()->shortcuts("contactlist.delete"));
	connect(removeAction_, SIGNAL(activated()), SLOT(deleteAccount()));
	ui_.treeView->addAction(removeAction_);

	ui_.treeView->setModel(model_);
	connect(ui_.treeView, SIGNAL(selectionChanged(const QItemSelection&)), SLOT(updateRemoveAction()));

	YaPushButton* closeButton = new YaPushButton(this);
	closeButton->setText(tr("Close"));
	ui_.buttonBox->addButton(closeButton, QDialogButtonBox::RejectRole);
	connect(closeButton, SIGNAL(clicked()), window(), SLOT(close()));

	connect(ui_.addButton, SIGNAL(clicked()), SLOT(addAccount()));
	connect(ui_.deleteButton, SIGNAL(clicked()), SLOT(deleteAccount()));
	ui_.deleteButton->setButtonStyle(YaPushButton::ButtonStyle_Destructive);
	updateRemoveAction();

	resize(482, 340);

	ui_.treeView->setFocus();
	if (model_->rowCount(QModelIndex()) > 0) {
		QModelIndex index = model_->index(0, 0, QModelIndex());
		if (index.isValid()) {
			ui_.treeView->setCurrentIndex(index);
		}
	}

	YaPushButton::initAllButtons(this);
}

YaManageAccounts::~YaManageAccounts()
{
}

void YaManageAccounts::setController(PsiCon* controller)
{
	controller_ = controller;
	Q_ASSERT(controller_);
	model_->setContactList(controller_->contactList());
}

bool YaManageAccounts::eventFilter(QObject* obj, QEvent* e)
{
	if (e->type() == QEvent::ContextMenu && obj == ui_.treeView) {
		QContextMenuEvent* contextMenuEvent = static_cast<QContextMenuEvent*>(e);

		// this is necessary as the actions operate on the selected item
		QModelIndex index = ui_.treeView->indexAt(contextMenuEvent->pos());
		ui_.treeView->setCurrentIndex(index);

		if (!contextMenuEvent->modifiers().testFlag(Qt::AltModifier))
			return true;

		QMenu menu;
		menu.addAction(xmlConsoleAction_);
		menu.exec(ui_.treeView->mapToGlobal(contextMenuEvent->pos()));

		ui_.treeView->repairMouseTracking();
		return true;
	}

	return QWidget::eventFilter(obj, e);
}

void YaManageAccounts::addAccount()
{
	Q_ASSERT(controller_);
	PsiAccount* account = controller_->createAccount();
	account->setEnabled(false);
	Q_ASSERT(account);
	Q_UNUSED(account);

	ui_.treeView->editNewAccount();
}

void YaManageAccounts::deleteAccount()
{
	QModelIndex index = ui_.treeView->selectedIndex();
	if (index.isValid()) {
		selectedAccount_ = index.data(Qt::DisplayRole).toString();
		if (model_->editMode(index) != AccountInformerModel::EditPassword) {
			confirmationDelete();
		}
		else {
			YaRemoveConfirmationMessageBox msgBox(
			    tr("Deleting account"),
			    tr("Do you really want to delete <b>%1</b>?")
			    .arg(index.data(Qt::DisplayRole).toString()),
			    this);
			if (msgBox.confirmRemoval()) {
				confirmationDelete();
			}
		}
	}
}

void YaManageAccounts::updateRemoveAction()
{
	QModelIndexList selected = ui_.treeView->selectedIndexes();
	bool enableDelete = !selected.isEmpty() && (model_->rowCount(QModelIndex()) > 1);
#ifdef YAPSI_ACTIVEX_SERVER
	if (!selected.isEmpty()) {
		PsiAccount* account = model_->accountForIndex(selected.first());
		enableDelete = account->userAccount().saveable;
	}
#endif
	ui_.deleteButton->setEnabled(enableDelete);
	removeAction_->setEnabled(enableDelete);
	xmlConsoleAction_->setEnabled(!selected.isEmpty());
}

void YaManageAccounts::xmlConsole()
{
	QModelIndex index = ui_.treeView->selectedIndex();
	PsiAccount* account = index.isValid() ? model_->accountForIndex(index) : 0;
	if (account) {
		account->showXmlConsole();
	}
}

void YaManageAccounts::confirmationDelete()
{
	bool updatesEnabled = this->updatesEnabled();
	setUpdatesEnabled(false);
	QModelIndex index = ui_.treeView->selectedIndex();
	if (index.isValid()) {
		if (selectedAccount_ == index.data(Qt::DisplayRole).toString()) {
			PsiAccount* account = model_->accountForIndex(index);
#ifdef YAPSI_ACTIVEX_SERVER
			Q_ASSERT(account->userAccount().saveable);
#endif
			Q_ASSERT(account);
			delete account;
		}
	}
	setUpdatesEnabled(updatesEnabled);
}

void YaManageAccounts::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	background_.paint(&p, rect(), isActiveWindow());
}

void YaManageAccounts::selectFirstAccount()
{
	if (model_->rowCount() > 0) {
		ui_.treeView->setCurrentIndex(model_->index(0, 0));
	}
}
