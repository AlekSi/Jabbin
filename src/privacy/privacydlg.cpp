/*
 * privacydlg.cpp
 * Copyright (C) 2006  Remko Troncon
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
 
#include <QListView>
#include <QInputDialog>
#include <QMessageBox>

#include "privacydlg.h"
#include "privacylist.h"
#include "privacymanager.h"
#include "privacylistmodel.h"

// fixme: subscribe on the destroyed() signal of the manager

PrivacyDlg::PrivacyDlg(const QString& account_name, PrivacyManager* manager, QWidget* parent) : QDialog(parent), manager_(manager)
{
	ui_.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle(tr("%1: Privacy Lists").arg(account_name));

	connect(manager_,SIGNAL(listsReceived(const QString&, const QString&, const QStringList&)),SLOT(updateLists(const QString&, const QString&, const QStringList&)));
	connect(manager_,SIGNAL(listReceived(const PrivacyList&)),SLOT(refreshList(const PrivacyList&)));
	connect(manager_,SIGNAL(listError()),SLOT(list_failed()));
	//connect(manager_,SIGNAL(listNamesError()),SLOT(listNamesError()));
	//connect(manager_,SIGNAL(listReceiveError()),SLOT(listReceiveError()));

	connect(ui_.cb_active,SIGNAL(activated(int)),SLOT(active_selected(int)));
	connect(ui_.cb_default,SIGNAL(activated(int)),SLOT(default_selected(int)));
	connect(ui_.cb_lists,SIGNAL(activated(int)),SLOT(list_selected(int)));
	connect(ui_.cb_lists,SIGNAL(currentIndexChanged(int)),SLOT(list_changed(int)));
	connect(manager_,SIGNAL(changeActiveList_success()),SLOT(change_succeeded()));
	connect(manager_,SIGNAL(changeActiveList_error()),SLOT(change_failed()));
	connect(manager_,SIGNAL(changeDefaultList_success()),SLOT(change_succeeded()));
	connect(manager_,SIGNAL(changeDefaultList_error()),SLOT(change_failed()));
	connect(manager_,SIGNAL(changeList_success()),SLOT(changeList_succeeded()));
	connect(manager_,SIGNAL(changeList_error()),SLOT(changeList_failed()));

	connect(ui_.pb_newList,SIGNAL(clicked()),SLOT(newList()));
	connect(ui_.pb_deleteList,SIGNAL(clicked()),SLOT(removeList()));
	
	connect(ui_.pb_add,SIGNAL(clicked()),SLOT(addRule()));
	connect(ui_.pb_edit,SIGNAL(clicked()),SLOT(editCurrentRule()));
	connect(ui_.pb_remove,SIGNAL(clicked()),SLOT(removeCurrentRule()));
	connect(ui_.pb_up,SIGNAL(clicked()),SLOT(moveCurrentRuleUp()));
	connect(ui_.pb_down,SIGNAL(clicked()),SLOT(moveCurrentRuleDown()));
	connect(ui_.pb_apply,SIGNAL(clicked()),SLOT(applyList()));

	connect(ui_.pb_close,SIGNAL(clicked()),SLOT(close()));
	setWidgetsEnabled(false);

	// Disable all buttons
	ui_.pb_deleteList->setEnabled(false);
	setEditRuleEnabled(false);
	ui_.pb_add->setEnabled(false);
	ui_.pb_apply->setEnabled(false);

	// FIXME: Temporarily disabling auto-activate
	ui_.ck_autoActivate->hide();
	
	manager_->requestListNames();
}

void PrivacyDlg::setWidgetsEnabled(bool b)
{
	ui_.gb_settings->setEnabled(b);
	ui_.gb_listSettings->setEnabled(b);
}

void PrivacyDlg::setEditRuleEnabled(bool b)
{
	ui_.pb_up->setEnabled(b);
	ui_.pb_down->setEnabled(b);
	ui_.pb_edit->setEnabled(b);
	ui_.pb_remove->setEnabled(b);
}

void PrivacyDlg::addRule()
{
	model_.add();
}

void PrivacyDlg::editCurrentRule()
{
	// Maybe should use MVC setData here
	model_.edit(ui_.lv_rules->currentIndex());
}

void PrivacyDlg::removeCurrentRule()
{
	if (ui_.lv_rules->currentIndex().isValid()) {
		model_.removeRow(ui_.lv_rules->currentIndex().row(),ui_.lv_rules->currentIndex().parent());
	}
}

void PrivacyDlg::moveCurrentRuleUp()
{
	int row = ui_.lv_rules->currentIndex().row();
	if (model_.moveUp(ui_.lv_rules->currentIndex())) {
		ui_.lv_rules->setCurrentIndex(model_.index(row-1,0));
	}
}

void PrivacyDlg::moveCurrentRuleDown()
{
	int row = ui_.lv_rules->currentIndex().row();
	if (model_.moveDown(ui_.lv_rules->currentIndex())) {
		ui_.lv_rules->setCurrentIndex(model_.index(row+1,0));
	}
}

void PrivacyDlg::applyList()
{
	if (!model_.list().isEmpty()) {
		setWidgetsEnabled(false);
		manager_->changeList(model_.list());
		if (newList_)
			manager_->requestListNames();
	}
}

void PrivacyDlg::close()
{
	done(0);
}

void PrivacyDlg::updateLists(const QString& defaultList, const QString& activeList, const QStringList& names)
{
	// Active list
	ui_.cb_active->clear();
	ui_.cb_active->insertItem(tr("<None>"));
	ui_.cb_active->insertStringList(names);
	if (!activeList.isEmpty()) {
		ui_.cb_active->setCurrentItem(names.findIndex(activeList)+1);
	}
	else {
		ui_.cb_active->setCurrentItem(0);
	}
	previousActive_ = ui_.cb_active->currentItem();
	
	// Default list
	ui_.cb_default->clear();
	ui_.cb_default->insertItem(tr("<None>"));
	ui_.cb_default->insertStringList(names);
	if (!defaultList.isEmpty()) {
		ui_.cb_default->setCurrentItem(names.findIndex(defaultList)+1);
	}
	else {
		ui_.cb_default->setCurrentItem(0);
	}
	previousDefault_ = ui_.cb_default->currentItem();
	
	// All lists
	QString previousList = ui_.cb_lists->currentText();
	ui_.cb_lists->clear();
	ui_.cb_lists->insertStringList(names);
	if (ui_.cb_lists->count() > 0) {
		if (!previousList.isEmpty() && ui_.cb_lists->findText(previousList) != -1) {
			ui_.cb_lists->setCurrentItem(ui_.cb_lists->findText(previousList));
		}
		else {
			QString currentList = (activeList.isEmpty() ? activeList : defaultList);
			if (!currentList.isEmpty()) {
				ui_.cb_lists->setCurrentItem(names.findIndex(currentList));
			}
		}
		manager_->requestList(ui_.cb_lists->currentText());
	}
	else {
		setWidgetsEnabled(true);
	}
	
	ui_.lv_rules->setModel(&model_);
}

void PrivacyDlg::listChanged()
{
	if (model_.list().isEmpty()) {
		ui_.cb_lists->removeItem(previousList_);
		rememberSettings();
	}
	setWidgetsEnabled(false);
	manager_->requestList(ui_.cb_lists->currentText());
}

void PrivacyDlg::refreshList(const PrivacyList& list)
{
	if (list.name() == ui_.cb_lists->currentText()) {
		rememberSettings();
		model_.setList(list);
		setWidgetsEnabled(true);
	}
}

void PrivacyDlg::active_selected(int i)
{
	if (i != previousActive_) {
		setWidgetsEnabled(false);
		manager_->changeActiveList((i == 0 ? "" : ui_.cb_active->text(i)));
	}
}

void PrivacyDlg::default_selected(int i)
{
	if (i != previousDefault_) {
		setWidgetsEnabled(false);
		manager_->changeDefaultList((i == 0 ? "" : ui_.cb_active->text(i)));
	}
}

void PrivacyDlg::list_selected(int i)
{
	if (i != previousList_) {
		listChanged();
	}
}

void PrivacyDlg::list_changed(int i)
{
	ui_.pb_deleteList->setEnabled(i != -1);
	ui_.pb_add->setEnabled(i != -1);
	setEditRuleEnabled(i != -1);
	ui_.pb_apply->setEnabled(i != -1);
	//setEditRuleEnabled(false);
	newList_ = false;
}

void PrivacyDlg::list_failed()
{
	revertSettings();
	setWidgetsEnabled(true);
}

void PrivacyDlg::changeList_succeeded()
{
	// If we just deleted a list, select the first list
	if (model_.list().isEmpty()) {
		ui_.cb_lists->setCurrentIndex(0);
		listChanged();
	}
	else {
		setWidgetsEnabled(true);
	}
}

void PrivacyDlg::changeList_failed()
{
	QMessageBox::critical(0, QObject::tr("Error"), QObject::tr("There was an error changing the list.")); 
	setWidgetsEnabled(true);
}

void PrivacyDlg::change_succeeded()
{
	rememberSettings();
	setWidgetsEnabled(true);
}

void PrivacyDlg::change_failed()
{
	revertSettings();
	QMessageBox::critical(0, QObject::tr("Error"), QObject::tr("There was an error processing your request.")); 
	setWidgetsEnabled(true);
}

void PrivacyDlg::rememberSettings()
{
	previousDefault_ = ui_.cb_default->currentItem();
	previousActive_ = ui_.cb_active->currentItem();
	previousList_ = ui_.cb_lists->currentItem();
}

void PrivacyDlg::revertSettings()
{
	ui_.cb_default->setCurrentItem(previousDefault_);
	ui_.cb_active->setCurrentItem(previousActive_);
	ui_.cb_lists->setCurrentItem(previousList_);
}


void PrivacyDlg::newList()
{
	bool done = false;
	bool ok = false;
	QString name;
	while (!done) {
		name = QInputDialog::getText(this, tr("New List"), tr("Enter the name of the new list:"), QLineEdit::Normal, "", &ok);
		if (!ok) {
			done = true;
		}
		else if (ui_.cb_lists->findText(name) != -1) {
			QMessageBox::critical(this, tr("Error"), tr("A list with this name already exists."));
		}
		else if (!name.isEmpty()) {
			done = true;
		}
	}
	
	if (ok) {
		if (ui_.cb_lists->currentIndex() != -1 && model_.list().isEmpty()) {
			ui_.cb_lists->removeItem(ui_.cb_lists->currentIndex());
		}
		ui_.cb_lists->addItem(name);
		ui_.cb_lists->setCurrentItem(ui_.cb_lists->findText(name));
		model_.setList(PrivacyList(name));
		newList_ = true;
		rememberSettings();
	}
}

void PrivacyDlg::removeList()
{
	model_.list().clear();
	manager_->changeList(model_.list());
	manager_->requestListNames();
}
