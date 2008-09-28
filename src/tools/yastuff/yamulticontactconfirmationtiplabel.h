/*
 * yamulticontactconfirmationtiplabel.h
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

#ifndef YAMULTICONTACTCONFIRMATIONTIPLABEL_H
#define YAMULTICONTACTCONFIRMATIONTIPLABEL_H

#include "psitiplabel.h"

#include "ui_yamulticontactconfirmationtiplabel.h"

class PsiContact;
class ToolTipPosition;
class LastActivityTask;
class QMimeData;
class QStandardItemModel;
class YaRosterTipLabelLeaveTimer;

namespace XMPP {
	class VCard;
	class JT_VCard;
};

using namespace XMPP;

class YaMultiContactConfirmationTipLabel : public PsiTipLabel
{
	Q_OBJECT
public:
	YaMultiContactConfirmationTipLabel(QWidget* parent);
	~YaMultiContactConfirmationTipLabel();

	void setTipPosition(ToolTipPosition* tipPosition);
	void setContactSelection(QMimeData* contactSelection);
	void setRealModel(YaContactListModel* model);
	void setListView(QWidget* listView);

	// reimplemented
	QSize sizeHint() const;
	bool eventFilter(QObject *, QEvent *);

signals:
	void removeContactConfirmation(QMimeData* contactSelection);

protected:
	// reimplemented
	void enterEvent(QEvent*);
	void timerEvent(QTimerEvent* e);
	void paintEvent(QPaintEvent* e);
	void resizeEvent(QResizeEvent*);

private slots:
	void confirmationDelete();
	void confirmationCancel();
	void updateSize();
	void deleteSelection();

protected:
	Ui::YaMultiContactConfirmationTipLabel ui_;

	// reimplemented
	void initUi();
	virtual void hideTip();

private:
	ToolTipPosition* tipPosition_;
	QMimeData* contactSelection_;
	bool initialized_;
	QPushButton* confirmationDeleteButton_;
	QPushButton* confirmationCancelButton_;
	QStandardItemModel* model_;
	YaContactListModel* realModel_;
	YaRosterTipLabelLeaveTimer* leaveTimer_;

	void invalidateData();
	void setRemoveContactConfirmationEnabled(bool removeContactConfirmationEnabled);

	// reimplemented
	void startHideTimer();
};

#endif
