/*
 * yaremoveconfirmationmessagebox.h - generic confirmation of destructive action
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

#ifndef YAREMOVECONFIRMATIONMESSAGEBOX_H
#define YAREMOVECONFIRMATIONMESSAGEBOX_H

#include <QMessageBox>

class QPushButton;

class YaRemoveConfirmationMessageBox : public QMessageBox
{
	Q_OBJECT
public:
	YaRemoveConfirmationMessageBox(const QString& title, const QString& informativeText, QWidget* parent);

	void setDestructiveActionName(const QString& destructiveAction);

	bool confirmRemoval();

private:
	QPushButton* removeButton_;
	QPushButton* cancelButton_;
};

#endif
