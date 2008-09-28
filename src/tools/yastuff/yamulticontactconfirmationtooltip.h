/*
 * yamulticontactconfirmationtooltip.h
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

#ifndef YAMULTICONTACTCONFIRMATIONTOOLTIP_H
#define YAMULTICONTACTCONFIRMATIONTOOLTIP_H

#include "yarostertooltip.h"

class YaMultiContactConfirmationTipLabel;
class YaContactListModel;

class YaMultiContactConfirmationToolTipPosition : public YaRosterToolTipPosition
{
public:
	YaMultiContactConfirmationToolTipPosition(QRect rect, const QWidget* parentWidget)
		: YaRosterToolTipPosition(rect, parentWidget)
	{}

protected:
	// reimplemented
	virtual QPoint initialPoint(const QWidget* label) const;
};

class YaMultiContactConfirmationToolTip : public YaRosterToolTip
{
public:
	static YaMultiContactConfirmationToolTip* instance();

	void show(QRect rect, YaContactListModel* model, QMimeData* contactSelection, const QWidget* widget);

	// reimplemented
	virtual bool isVisible() const;

protected:
	YaMultiContactConfirmationToolTip() {}

	YaContactListModel* model_;

private:
	// reimplemented
	virtual ToolTipPosition* createTipPosition(const QPoint& cursorPos, const QWidget* parentWidget);
	virtual PsiTipLabel* createTipLabel(const QString& text, QWidget* parent);
	virtual void updateTipLabel(PsiTipLabel* label, const QString& text, const ToolTipPosition* pos);

	static YaMultiContactConfirmationToolTip* instance_;
};

#endif
