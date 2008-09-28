/*
 * contactlistviewdelegate.cpp - base class for painting contact list items
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

#include "contactlistviewdelegate.h"

#include <limits.h> // for INT_MAX

#include <QPainter>
#include <QSortFilterProxyModel>

#include "contactlistmodel.h"
#include "contactlistview.h"
#include "iconset.h"
#include "common.h"
#include "yavisualutil.h"

class ContactListViewDelegate::Private
{
public:
	ContactListView* contactList;
	HoverableStyleOptionViewItem opt;
	QString text;
	QIcon icon;
	QRect displayRect;
	QRect decorationRect;
	QIcon::Mode  iconMode;
	QIcon::State iconState;
};

ContactListViewDelegate::ContactListViewDelegate(ContactListView* parent)
	: QItemDelegate(parent)
{
	d = new Private;
	d->contactList = parent;
}

ContactListViewDelegate::~ContactListViewDelegate()
{
	delete d;
}

QIcon::Mode ContactListViewDelegate::iconMode() const
{
	return d->iconMode;
}

QIcon::State ContactListViewDelegate::iconState() const
{
	return d->iconState;
}

const QRect& ContactListViewDelegate::displayRect() const
{
	return d->displayRect;
}

const QRect& ContactListViewDelegate::decorationRect() const
{
	return d->decorationRect;
}

const HoverableStyleOptionViewItem& ContactListViewDelegate::opt() const
{
	return d->opt;
}

int ContactListViewDelegate::offset() const
{
	return qMax(d->opt.rect.right() - d->displayRect.right(), 4);
}

const ContactListView* ContactListViewDelegate::contactList() const
{
	return d->contactList;
}

inline static QIcon::Mode iconMode(QStyle::State state)
{
	if (!(state & QStyle::State_Enabled)) return QIcon::Disabled;
	if   (state & QStyle::State_Selected) return QIcon::Selected;
	return QIcon::Normal;
}

inline static QIcon::State iconState(QStyle::State state)
{
	return state & QStyle::State_Open ? QIcon::On : QIcon::Off;
}

inline static QRect textLayoutBounds(const QStyleOptionViewItemV2 &option)
{
	QRect rect = option.rect;
	const bool wrapText = option.features & QStyleOptionViewItemV2::WrapText;
	switch (option.decorationPosition) {
	case QStyleOptionViewItem::Left:
	case QStyleOptionViewItem::Right:
		rect.setWidth(INT_MAX >> 6);
		break;
	case QStyleOptionViewItem::Top:
	case QStyleOptionViewItem::Bottom:
		rect.setWidth(wrapText ? option.decorationSize.width() : (INT_MAX >> 6));
		break;
	}

	return rect;
}

void ContactListViewDelegate::doSetOptions(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	d->opt = setOptions(index, option);
	const QStyleOptionViewItemV2 *v2 = qstyleoption_cast<const QStyleOptionViewItemV2 *>(&option);
	d->opt.features = v2 ? v2->features : QStyleOptionViewItemV2::ViewItemFeatures(QStyleOptionViewItemV2::None);

	const HoverableStyleOptionViewItem *hoverable = qstyleoption_cast<const HoverableStyleOptionViewItem *>(&option);
	d->opt.hovered = hoverable ? hoverable->hovered : false;

	if ((d->opt.displayAlignment & Qt::AlignLeft) &&
	    (d->opt.displayAlignment & Qt::AlignRight) &&
	    (d->opt.displayAlignment & Qt::AlignHCenter) &&
	    (d->opt.displayAlignment & Qt::AlignJustify))
	{
		d->opt.hovered = true;
	}
}

void ContactListViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	doSetOptions(option, index);

	// prepare
	painter->save();
	// if (hasClipping())
	// 	painter->setClipRect(d->opt.rect);

	QVariant value;

	d->iconMode  = ::iconMode(opt().state);
	d->iconState = ::iconState(opt().state);

	d->icon = QIcon();
	value = index.data(Qt::DecorationRole);
	if (value.isValid()) {
		if (value.type() == QVariant::Icon) {
			d->icon = qvariant_cast<QIcon>(value);
			d->decorationRect = QRect(QPoint(0, 0),
				d->icon.actualSize(opt().decorationSize, d->iconMode, d->iconState));
		}
	}

	d->text = QString();
	value = index.data(Qt::DisplayRole);
	if (value.isValid()) {
		d->text = value.toString();
		// d->text = QItemDelegatePrivate::replaceNewLine(value.toString());

		d->displayRect = textRectangle(painter, textLayoutBounds(d->opt), d->opt.font, d->text);
	}

	QRect checkRect;
	// Qt::CheckState checkState = Qt::Unchecked;
	// value = index.data(Qt::CheckStateRole);
	// if (value.isValid()) {
	// 	checkState = static_cast<Qt::CheckState>(value.toInt());
	// 	checkRect = check(d->opt, d->opt.rect, value);
	// }

	// do the layout
	doLayout(d->opt, &checkRect, &d->decorationRect, &d->displayRect, false);

	// draw the item
	// drawBackground(painter, d->opt, index);
	// drawCheck(painter, d->opt, checkRect, checkState);

	switch (ContactListModel::indexType(index)) {
	case ContactListModel::ContactType:
		drawContact(painter, opt(), index);
		break;
	case ContactListModel::GroupType:
		drawGroup(painter, opt(), index);
		break;
	case ContactListModel::AccountType:
		drawAccount(painter, opt(), index);
		break;
	default:
		QItemDelegate::paint(painter, option, index);
	}
	painter->restore();

}

void ContactListViewDelegate::defaultDraw(QPainter* painter, const QStyleOptionViewItem& option) const
{
	if (!d->icon.isNull())
		d->icon.paint(painter, d->decorationRect, option.decorationAlignment, d->iconMode, d->iconState);
	drawText(painter, d->opt, d->displayRect, d->text, QModelIndex());
	drawFocus(painter, d->opt, d->text.isEmpty() ? QRect() : d->displayRect);
}

int ContactListViewDelegate::avatarSize() const
{
	return contactList()->largeIcons() ? 32 : 16;
}

void ContactListViewDelegate::drawContact(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	drawBackground(painter, d->opt, index);

	// if (contactList()->showStatus() && !contact->status().status().isEmpty())
		d->text = QString("%1 (%2)").arg(d->text).arg(index.data(ContactListModel::StatusTextRole).toString());

	QIcon avatar;
	QRect displayRect = d->displayRect;
	if (contactList()->showIcons()) {
		QVariant avatarData = index.data(ContactListModel::PictureRole);
		if (avatarData.isValid())
			if (avatarData.type() == QVariant::Icon)
				avatar = qvariant_cast<QIcon>(avatarData);

		if (!avatarData.isNull())
			d->displayRect.adjust(0, 0, -avatarSize() - offset(), 0);
	}

	defaultDraw(painter, option);

	if (contactList()->showIcons()) {
		d->decorationRect = d->opt.rect;
		d->decorationRect.setLeft(d->decorationRect.right() - avatarSize());
		d->icon = avatar;
		d->text = QString();
		defaultDraw(painter, option);
	}

	// if ( d->u ) {
	// 	UserResourceList::ConstIterator it = d->u->priority();
	// 	if(it != d->u->userResourceList().end()) {
	// 		if(d->u->isSecure((*it).name())) {
	// 			const QPixmap &pix = IconsetFactory::iconPixmap("psi/cryptoYes");
	// 			int y = (height() - pix.height()) / 2;
	// 			p->drawPixmap(x, y, pix);
	// 			x += 24;
	// 		}
	// 	}
	// }
}

void ContactListViewDelegate::drawGroup(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	// d->opt.font.setPointSize(::option.smallFontSize);

	// if (!::option.clNewHeadings || (option.state & QStyle::State_Selected))
		drawBackground(painter, d->opt, index);

	// d->text += " " + group->groupInfo();

	if (!d->icon.isNull())
		d->icon.paint(painter, d->decorationRect, option.decorationAlignment, d->iconMode, d->iconState);

	QRect displayRect = d->displayRect;

	QFontMetrics fm(d->opt.font);
	// int info_y = ((d->displayRect.height() - fm.height()) / 2) + fm.ascent();
	int info_y = (d->displayRect.height() - fm.ascent()) / 2;
	d->displayRect.setTop(d->displayRect.top() + info_y);
	drawText(painter, d->opt, d->displayRect, d->text, index);
	d->displayRect = displayRect;

	// if (!::option.clNewHeadings) {
	// 	QRect rect = option.rect;
	// 	rect.adjust(0, 0, -1, -1);
	// 	painter->setPen(qvariant_cast<QColor>(index.data(Qt::ForegroundRole)));
	// 	painter->drawRect(rect);
	// }
	// else if (!(option.state & QStyle::State_Selected)) {
		d->displayRect.setLeft(d->displayRect.left() + fm.width(d->text) + 8);
		if (d->displayRect.width() > 8) {
			int h = d->displayRect.top() + (d->displayRect.height() / 2) - 1;
			painter->setPen(qvariant_cast<QColor>(index.data(Qt::BackgroundRole)));
			painter->drawLine(d->displayRect.x(), h, d->displayRect.right(), h);
			h++;
			painter->setPen(qvariant_cast<QColor>(index.data(Qt::ForegroundRole)));
			painter->drawLine(d->displayRect.x(), h, d->displayRect.right(), h);
		}
	// }

	d->displayRect = displayRect;
	drawFocus(painter, d->opt, d->text.isEmpty() ? QRect() : d->displayRect);
}

void ContactListViewDelegate::drawAccount(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	drawBackground(painter, d->opt, index);

	if (!d->icon.isNull())
		d->icon.paint(painter, d->decorationRect, option.decorationAlignment, d->iconMode, d->iconState);

	drawText(painter, d->opt, d->displayRect, d->text, index);

	QFontMetrics fm(d->opt.font);
	d->displayRect.adjust(fm.width(d->text) + 8, 0, 0, 0);
	
	QRect rect = d->displayRect;
	// const QPixmap &pix = account->usingSecurityLayer() ? IconsetFactory::iconPixmap("psi/cryptoYes") : IconsetFactory::iconPixmap("psi/cryptoNo");
	// rect.adjust(0, (rect.height() - pix.height()) / 2, 0, 0);
	// painter->drawPixmap(rect.topLeft(), pix);

	// d->displayRect.adjust(pix.width(), 0, 0, 0);
	// d->text = account->groupInfo();
	drawText(painter, d->opt, d->displayRect, d->text, index);
	
	// if (!::option.clNewHeadings) {
	{	QRect rect = option.rect;
		rect.adjust(0, 0, -1, -1);
		painter->setPen(qvariant_cast<QColor>(index.data(Qt::ForegroundRole)));
		painter->drawRect(rect);
	}
	// }

	// drawFocus(painter, d->opt, d->text.isEmpty() ? QRect() : d->displayRect);
}

void ContactListViewDelegate::drawText(QPainter* painter, const QStyleOptionViewItem& option, const QRect& rect, const QString& text, const QModelIndex& index) const
{
	if (text.isEmpty())
		return;

	QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
	                          ? QPalette::Normal : QPalette::Disabled;
	if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
		cg = QPalette::Inactive;
	if (option.state & QStyle::State_Selected) {
		painter->setPen(option.palette.color(cg, QPalette::HighlightedText));
	}
	else {
		painter->setPen(option.palette.color(cg, QPalette::Text));
	}

	QString txt = text;
	if (rect.width() < option.fontMetrics.width(text)) {
		// txt = option.fontMetrics.elidedText(text, option.textElideMode, rect.width());
		painter->save();
		painter->setClipRect(rect);
	}

	// painter->save();
	// painter->setPen(Qt::gray);
	// painter->drawLine(rect.left(), rect.top() + option.fontMetrics.ascent(), rect.right(), rect.top() + option.fontMetrics.ascent());
	// painter->restore();

	painter->setFont(option.font);
	painter->drawText(rect.x(), rect.y() + option.fontMetrics.ascent(), txt);

	if (rect.width() < option.fontMetrics.width(text)) {
		painter->restore();
		Ya::VisualUtil::drawTextFadeOut(painter, rect, backgroundColor(option, index), 15);
	}
}

QSize ContactListViewDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	Q_UNUSED(option);
	if (index.isValid()) {
		if (ContactListModel::indexType(index) == ContactListModel::ContactType)
			return QSize(16, avatarSize());
		return QSize(16, 16 + 3);
	}

	return QSize(0, 0);
}

// copied from void QItemDelegate::drawBackground(), Qt 4.3.4
QColor ContactListViewDelegate::backgroundColor(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if (hovered()) {
		return Ya::VisualUtil::hoverHighlightColor();
	}

	if (option.showDecorationSelected && (option.state & QStyle::State_Selected)) {
		QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
		                          ? QPalette::Normal : QPalette::Disabled;
		if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
			cg = QPalette::Inactive;

		return option.palette.brush(cg, QPalette::Highlight).color();
	}
	else {
		QVariant value = index.data(Qt::BackgroundRole);
		if (qVariantCanConvert<QBrush>(value)) {
			return qvariant_cast<QBrush>(value).color();
		}
	}

	return Qt::white;
}

void ContactListViewDelegate::drawBackground(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QStyleOptionViewItem opt = option;

	// otherwise we're not painting the left item margin sometimes
	// (for example when changing current selection by keyboard)
	opt.rect.setLeft(0);

	// QItemDelegate::drawBackground(painter, opt, index);
	{
		if (option.showDecorationSelected && (option.state & QStyle::State_Selected)) {
			painter->fillRect(opt.rect, backgroundColor(option, index));
		}
		else {
			QPointF oldBO = painter->brushOrigin();
			painter->setBrushOrigin(opt.rect.topLeft());
			painter->fillRect(opt.rect, backgroundColor(option, index));
			painter->setBrushOrigin(oldBO);
		}
	}
}

bool ContactListViewDelegate::hovered() const
{
	return opt().hovered;
}

void ContactListViewDelegate::setHovered(bool hovered) const
{
	d->opt.hovered = hovered;
}

QPoint ContactListViewDelegate::hoveredPosition() const
{
	if (!opt().hovered)
		return QPoint();

	return QPoint(opt().decorationSize.width(), opt().decorationSize.height());
}
