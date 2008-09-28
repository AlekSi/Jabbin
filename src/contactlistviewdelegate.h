/*
 * contactlistviewdelegate.h - base class for painting contact list items
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

#ifndef CONTACTLISTVIEWDELEGATE_H
#define CONTACTLISTVIEWDELEGATE_H

#include <QItemDelegate>

#include "contactlistmodel.h"

#include "hoverabletreeview.h"

class ContactListView;
class ContactListItemProxy;
class PsiContact;
class ContactListGroup;
class PsiAccount;
class QStyleOptionViewItemV2;

class ContactListViewDelegate : public QItemDelegate
{
public:
	ContactListViewDelegate(ContactListView* parent);
	virtual ~ContactListViewDelegate();

	virtual const ContactListView* contactList() const;

	virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

	void doSetOptions(const QStyleOptionViewItem& option, const QModelIndex& index) const;

protected:
	virtual void drawContact(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	virtual void drawGroup(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	virtual void drawAccount(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

	virtual int avatarSize() const;
	virtual void defaultDraw(QPainter* painter, const QStyleOptionViewItem& option) const;

	QColor backgroundColor(const QStyleOptionViewItem& option, const QModelIndex& index) const;

	void drawText(QPainter* painter, const QStyleOptionViewItem& o, const QRect& rect, const QString& text, const QModelIndex& index) const;
	void drawBackground(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

	QIcon::Mode iconMode() const;
	QIcon::State iconState() const;
	const QRect& displayRect() const;
	const QRect& decorationRect() const;
	const HoverableStyleOptionViewItem& opt() const;

	// these two are functional in paint() call
	bool hovered() const;
	QPoint hoveredPosition() const;
	void setHovered(bool hovered) const;

	int offset() const;

private:
	class Private;
	Private* d;
};

#endif
