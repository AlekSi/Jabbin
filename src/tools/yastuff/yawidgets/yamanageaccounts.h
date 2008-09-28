/*
 * yamanageaccounts.h - account editor
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

#ifndef YAMANAGEACCOUNTS_H
#define YAMANAGEACCOUNTS_H

#include <QWidget>

#include "ui_yamanageaccounts.h"

#include "yawindowbackground.h"

class PsiCon;
class QAction;

class YaManageAccounts : public QWidget
{
	Q_OBJECT
public:
	YaManageAccounts(QWidget* parent);
	~YaManageAccounts();

	void setController(PsiCon* controller);

	void selectFirstAccount();

protected:
	// reimplemented
	bool eventFilter(QObject* obj, QEvent* e);
	void paintEvent(QPaintEvent*);

private slots:
	void addAccount();
	void deleteAccount();
	void xmlConsole();
	void confirmationDelete();
	void updateRemoveAction();

private:
	PsiCon* controller_;
	AccountInformerModel* model_;
	QAction* xmlConsoleAction_;
	Ui::YaManageAccounts ui_;
	QString selectedAccount_;
	QAction* removeAction_;
	YaWindowBackground background_;
};

#endif
