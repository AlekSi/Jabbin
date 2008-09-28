/*
 * yamulticontactconfirmationtooltip.cpp
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

#include "yamulticontactconfirmationtooltip.h"

#include <QMimeData>

#include "yamulticontactconfirmationtiplabel.h"

//----------------------------------------------------------------------------
// YaMultiContactConfirmationToolTipPosition
//----------------------------------------------------------------------------

QPoint YaMultiContactConfirmationToolTipPosition::initialPoint(const QWidget* label) const
{
	return globalRect.topLeft();
}

//----------------------------------------------------------------------------
// YaMultiContactConfirmationToolTip
//----------------------------------------------------------------------------

YaMultiContactConfirmationToolTip *YaMultiContactConfirmationToolTip::instance_ = 0;

YaMultiContactConfirmationToolTip* YaMultiContactConfirmationToolTip::instance()
{
	if (!instance_)
		instance_ = new YaMultiContactConfirmationToolTip();
	return instance_;
}

PsiTipLabel* YaMultiContactConfirmationToolTip::createTipLabel(const QString& text, QWidget* parent)
{
	YaMultiContactConfirmationTipLabel* label = new YaMultiContactConfirmationTipLabel(parent);
	connect(label, SIGNAL(removeContactConfirmation(QMimeData*)), SIGNAL(removeContactConfirmation(QMimeData*)));
	label->init(text);
	updateTipLabel(label, text, 0);
	return label;
}

bool YaMultiContactConfirmationToolTip::isVisible() const
{
	return PsiToolTip::isVisible() &&
	       dynamic_cast<YaMultiContactConfirmationTipLabel*>(PsiTipLabel::instance());
}

void YaMultiContactConfirmationToolTip::show(QRect rect, YaContactListModel* model, QMimeData* contactSelection, const QWidget* widget)
{
	if (!contactSelection)
		return;
	lastWidget_ = const_cast<QWidget*>(widget);
	lastRect_ = rect;
	model_ = model;
	contactSelection_ = contactSelection;
	doShowText(rect.topLeft(), contactSelection->text(), lastWidget_);
	contactSelection_ = 0;
	model_ = 0;
}

void YaMultiContactConfirmationToolTip::updateTipLabel(PsiTipLabel* label, const QString& text, const ToolTipPosition* pos)
{
	Q_UNUSED(pos);
	YaMultiContactConfirmationTipLabel* multiLabel = dynamic_cast<YaMultiContactConfirmationTipLabel*>(label);
	multiLabel->setText(text);
	multiLabel->setTipPosition(createTipPosition(QPoint(), lastWidget_));
	multiLabel->setRealModel(model_);
	multiLabel->setContactSelection(contactSelection_);
	multiLabel->setListView(lastWidget_);
}

ToolTipPosition* YaMultiContactConfirmationToolTip::createTipPosition(const QPoint&, const QWidget* parentWidget)
{
	return new YaMultiContactConfirmationToolTipPosition(lastRect_, parentWidget);
}
