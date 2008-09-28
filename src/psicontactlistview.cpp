/*
 * psicontactlistview.cpp - Psi-specific ContactListView-subclass
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

#include "psicontactlistview.h"

#include <QHelpEvent>

#include "psicontactlistviewdelegate.h"
#include "psicontactlistmodel.h"
#include "psitooltip.h"
#include "psioptions.h"

/**
 * TODO
 */
PsiContactListView::PsiContactListView(QWidget* parent)
	: ContactListView(parent)
{
	setItemDelegate(new PsiContactListViewDelegate(this));

#ifdef Q_WS_MAC
	setFrameShape(QFrame::NoFrame);
#endif

	connect(this, SIGNAL(activated(const QModelIndex&)), SLOT(itemActivated(const QModelIndex&)));
	showStatus_ = PsiOptions::instance()->getOption("options.ui.contactlist.status-messages.show").toBool();
}

/**
 * TODO
 */
void PsiContactListView::setModel(QAbstractItemModel* _model)
{
	PsiContactListModel* model = dynamic_cast<PsiContactListModel*>(_model);
	Q_ASSERT(model);
	ContactListView::setModel(model);
}

/**
 * Re-implemented in order to use PsiToolTip class to display tooltips.
 */
bool PsiContactListView::viewportEvent(QEvent* event)
{
	if (event->type() == QEvent::ToolTip
	    && isActiveWindow())
	{
		QHelpEvent* he = static_cast<QHelpEvent*>(event);
		showToolTip(indexAt(he->pos()), he->globalPos());
		return true;
	}

	return ContactListView::viewportEvent(event);
}

void PsiContactListView::showToolTip(const QModelIndex& index, const QPoint& globalPos) const
{
	QVariant toolTip = model()->data(index, Qt::ToolTipRole);
	if (!toolTip.toString().isEmpty())
		PsiToolTip::showText(globalPos, toolTip.toString(), this);
}

/**
 * TODO
 */
void PsiContactListView::optionsUpdate()
{
}

/**
 * Prevent emitting activated events when clicking with right mouse button.
 */
void PsiContactListView::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() & Qt::LeftButton)
		ContactListView::mouseReleaseEvent(event);
}

/**
 * TODO
 */
void PsiContactListView::itemActivated(const QModelIndex& index)
{
	model()->setData(index, QVariant(true), PsiContactListModel::ActivateRole);
}

/**
 * Returns true if the model is set to show contact's status along with its name in
 * one column.
 */
bool PsiContactListView::showStatus() const
{
	return showStatus_;
}

/**
 * If \param show is true, then model will provide contact's status along with its name.
 * \sa showStatus()
 */
void PsiContactListView::setShowStatus(bool showStatus)
{
	if (showStatus_ != showStatus) {
		showStatus_ = showStatus;
		PsiOptions::instance()->setOption("options.ui.contactlist.status-messages.show", showStatus_);
		doItemsLayout(); // FIXME: is it ok?
		emit showStatusChanged(showStatus_);
	}
}

