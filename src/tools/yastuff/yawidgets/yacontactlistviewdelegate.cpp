/*
 * yacontactlistviewdelegate.cpp
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

#include "yacontactlistviewdelegate.h"

#include <QPainter>
#include <QTextLayout>
#include <QTextOption>
#include <QApplication>
#include <QDebug>

#include "contactlistmodel.h"
#include "yacontactlistview.h"
#include "yacommon.h"
#include "xmpp_status.h"
#include "pixmaputil.h"
#include "yacontactlistview.h"
#include "yarostertooltip.h"
#include "yaemptytextlineedit.h"

#define NAME_STATUS_RATIO 0.5
#define YAPSI_PALE_ROSTER_JIDS
#define YAPSI_STRIP_JID_HOST

YaContactListViewDelegate::YaContactListViewDelegate(YaContactListView* parent)
	: ContactListViewDelegate(parent)
	, horizontalMargin_(5)
	, verticalMargin_(3)
	, margin_(3)
	, avatarSize_(31)
#ifdef YAPSI_PALE_ROSTER_JIDS
	, preserveJidHosts_(false)
#endif
{
	setDrawStatusIcon(true);
	phoneIcon = QIcon(":/iconsets/chatwindow/call.png");
}

const YaContactListView* YaContactListViewDelegate::contactList() const
{
	return static_cast<const YaContactListView*>(ContactListViewDelegate::contactList());
}

QSize YaContactListViewDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	return ContactListViewDelegate::sizeHint(option, index);
}

bool YaContactListViewDelegate::preserveJidHosts() const
{
#ifdef YAPSI_PALE_ROSTER_JIDS
	return true;
#else
	return preserveJidHosts_;
#endif
}

void YaContactListViewDelegate::setPreserveJidHosts(bool preserveJidHosts)
{
#ifdef YAPSI_PALE_ROSTER_JIDS
	preserveJidHosts_ = preserveJidHosts;
#else
	Q_UNUSED(preserveJidHosts);
#endif
}

QString YaContactListViewDelegate::nameText(const QStyleOptionViewItem& o, const QModelIndex& index) const
{
	QString name = index.data(Qt::DisplayRole).toString();
#ifdef YAPSI_PALE_ROSTER_JIDS
	if (!preserveJidHosts_) {
		return Ya::contactName(name, index.data(ContactListModel::JidRole).toString());
	}
// 	if (name == index.data(ContactListModel::JidRole).toString() && !preserveJidHosts_) {
// #ifndef YAPSI_STRIP_JID_HOST
// 		if (opt().hovered || o.state & QStyle::State_Selected) {
// 			return name;
// 		}
// #endif
//
// 		XMPP::Jid jid(name);
// 		return jid.node();
// 	}
#endif
	return name;
}

void YaContactListViewDelegate::drawName(QPainter* painter, const QStyleOptionViewItem& o, const QRect& rect, const QString& name, const QModelIndex& index) const
{
    QStyleOptionViewItem option(o);
    option.font.setPixelSize(nameFontSize(option.rect));
    option.font.setBold(false);

    if (XMPP::Status::Online != statusType(index)) {
        option.palette.setColor(QPalette::HighlightedText, Qt::gray);
        option.palette.setColor(QPalette::Text, Qt::gray);
    }

    /*
#ifdef YAPSI_PALE_ROSTER_JIDS
	if (name == index.data(ContactListModel::JidRole).toString() && !preserveJidHosts_) {
		XMPP::Jid jid(name);

		drawText(painter, option, rect, jid.node(), index);

#ifndef YAPSI_STRIP_JID_HOST
		if (opt().hovered || option.state & QStyle::State_Selected) {
			QStyleOptionViewItem o2(option);
			o2.rect = rect;
			o2.rect.setLeft(o2.rect.left() + o2.fontMetrics.width(jid.node()));
			o2.palette.setColor(QPalette::HighlightedText, Qt::gray);
			o2.palette.setColor(QPalette::Text, Qt::gray);
			drawText(painter, o2, o2.rect, QString("@") + jid.host(), index);
		}
#endif
		return;
	}
#endif*/
    drawText(painter, option, rect, name, index);
}

void YaContactListViewDelegate::drawContact(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
        //we call everything related to painting from here
	drawBackground(painter, option, index);
	realDrawContact(painter, option, index);
}

int YaContactListViewDelegate::horizontalMargin() const
{
	return horizontalMargin_;
}

int YaContactListViewDelegate::verticalMargin() const
{
	return verticalMargin_;
}

void YaContactListViewDelegate::setHorizontalMargin(int margin)
{
	horizontalMargin_ = margin;
}

void YaContactListViewDelegate::setVerticalMargin(int margin)
{
	verticalMargin_ = margin;
}

int YaContactListViewDelegate::margin() const
{
	return margin_;
}

void YaContactListViewDelegate::setMargin(int margin)
{
	margin_ = margin;
}

int YaContactListViewDelegate::nameFontSize(const QRect& nameRect) const
{
	return 11;
}

int YaContactListViewDelegate::statusTypeFontSize(const QRect& statusTypeRect) const
{
	return 9;
}

int YaContactListViewDelegate::statusMessageFontSize(const QRect& statusRect) const
{
	return statusRect.height() - (margin() * 2);
}

QRect YaContactListViewDelegate::avatarRect(const QRect& visualRect) const
{
	int avatarSize = this->avatarSize() - (verticalMargin() * 2) - 1;
	QRect result(QPoint(0, 0), QPoint(avatarSize, avatarSize));
	result.moveTopRight(visualRect.topRight());
	result.translate( - horizontalMargin(), verticalMargin());
	return result;
}

void YaContactListViewDelegate::doAvatar(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QVariant avatarData = index.data(ContactListModel::PictureRole);
	QIcon avatar;
	if (avatarData.isValid()) {
		if (avatarData.type() == QVariant::Icon) {
			avatar = qvariant_cast<QIcon>(avatarData);
		}
	}



        /*QRect r = option.rect;
        r.adjust(horizontalMargin_, verticalMargin_,
                -horizontalMargin_, -verticalMargin_);
        avatar->*/

	Ya::VisualUtil::drawAvatar(painter,
	                           avatarRect(option.rect),
	                           statusType(index),
	                           XMPP::VCard::Gender(index.data(ContactListModel::GenderRole).toInt()),
	                           avatar, false);
}

void YaContactListViewDelegate::drawStatus(QPainter* painter, const QStyleOptionViewItem& option, const QRect& statusRect, const QString& status, const QModelIndex& index) const
{
	drawText(painter, option, statusRect, status, index);
}

QPalette YaContactListViewDelegate::blackText() const
{
	QPalette result = opt().palette;
	result.setColor(QPalette::Text, Qt::black);
	result.setColor(QPalette::HighlightedText, result.color(QPalette::Text));
	return result;
}

QStyleOptionViewItemV2 YaContactListViewDelegate::nameStyle(bool selected, XMPP::Status::Type status, Ya::VisualUtil::RosterStyle rosterStyle, bool hovered) const
{
	QStyleOptionViewItemV2 n_o = opt();
	n_o.font = Ya::VisualUtil::contactNameFont(rosterStyle, status);
	n_o.fontMetrics = QFontMetrics(n_o.font);
	QPalette namePalette = blackText();
	if (!selected)
		namePalette.setColor(QPalette::Text, Ya::VisualUtil::statusColor(status, hovered));
	n_o.palette = namePalette;
	return n_o;
}

QStyleOptionViewItemV2 YaContactListViewDelegate::statusTextStyle(Ya::VisualUtil::RosterStyle rosterStyle, bool hovered) const
{
	QStyleOptionViewItemV2 sm_o = opt();
	sm_o.font = Ya::VisualUtil::contactStatusFont(rosterStyle);
	sm_o.fontMetrics = QFontMetrics(sm_o.font);
	QPalette grayText;
	grayText.setColor(QPalette::Text, Ya::VisualUtil::contactStatusColor(hovered));
	grayText.setColor(QPalette::HighlightedText, grayText.color(QPalette::Text));
	sm_o.palette = grayText;
	return sm_o;
}

QRect YaContactListViewDelegate::textRect(const QRect& visualRect) const
{
        int avatarWidth = avatarRect(visualRect).width();
        QRect result(
                visualRect.left()   + horizontalMargin_,
                visualRect.top()    + verticalMargin_,
                visualRect.width()  - avatarWidth   - 2 * horizontalMargin_,
                visualRect.height() - verticalMargin_ * 2);
        return result;
}

QString YaContactListViewDelegate::statusText(const QModelIndex& index) const
{
	QString result = index.data(ContactListModel::StatusTextRole).toString();
	// if (result.isEmpty() && statusType(index) == XMPP::Status::Offline)
	// 	result = tr("offline");
	return result;
}

XMPP::Status::Type YaContactListViewDelegate::statusType(const QModelIndex& index) const
{
	return static_cast<XMPP::Status::Type>(index.data(ContactListModel::StatusTypeRole).toInt());
}

bool YaContactListViewDelegate::drawStatusTypeText(const QModelIndex& index) const
{
	// return true;
	return statusType(index) == XMPP::Status::Away ||
	       statusType(index) == XMPP::Status::XA ||
	       statusType(index) == XMPP::Status::DND ||
	       statusType(index) == XMPP::Status::Online ||
	       statusType(index) == XMPP::Status::FFC
	       ;
}

void YaContactListViewDelegate::drawStatusTypeText(QPainter* painter, const QStyleOptionViewItem& option, QRect* rect, const QModelIndex& index) const
{
	QString statusTypeText = Ya::statusFullText(statusType(index));
	QRect statusTypeRect(*rect);
	QStyleOptionViewItemV2 st_o = option;
	st_o.font.setPixelSize(statusTypeFontSize(option.rect));
	st_o.fontMetrics = QFontMetrics(st_o.font);
	QPalette palette = blackText();
	// palette.setColor(QPalette::Text, Ya::VisualUtil::statusColor(statusType(index), hovered()));
	palette.setColor(QPalette::Text, Qt::gray);
	st_o.palette = palette;
	statusTypeRect.setWidth(st_o.fontMetrics.width(statusTypeText));
	statusTypeRect.moveTop(statusTypeRect.top() + option.fontMetrics.ascent() - st_o.fontMetrics.ascent());

	if (!statusTypeText.isEmpty()) {
		/*
		if (option.fontMetrics.width(index.data(Qt::DisplayRole).toString()) > rect->width() - statusTypeRect.width()) {
			statusTypeRect.moveLeft(rect->right() - statusTypeRect.width() + 1);
			rect->setWidth(statusTypeRect.left() - rect->left());
		}
		else {
			rect->setWidth(option.fontMetrics.width(index.data(Qt::DisplayRole).toString()));
			statusTypeRect.moveLeft(rect->right() + 1);
		}
		*/
		if (painter) {
			drawText(painter, st_o, statusTypeRect, statusTypeText, index);
		}
	}
}

void YaContactListViewDelegate::drawEditorBackground(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if (!contactList() || !contactList()->textInputInProgress())
		return;

	// reset clipping as if we have editor widget shown, the background is effectively drawn only
	// underneath of editor widget
	painter->setClipRect(option.rect);

	drawBackground(painter, option, index);
}

QRect YaContactListViewDelegate::nameRect(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QRect textRect = this->textRect(option.rect);

	QRect nameRect(textRect);
	nameRect.setHeight(textRect.height() * NAME_STATUS_RATIO);

        /* ivan: aligning to top
	if (statusText(index).isEmpty()) {
		nameRect.moveTop(textRect.top() + (textRect.height() - nameRect.height()) / 2);
	}
        */

	if (drawStatusIcon(statusType(index))) {
		QPixmap statusPixmap = Ya::VisualUtil::rosterStatusPixmap(statusType(index));
		nameRect.translate(statusPixmap.width() + 3, 0);
	}

	nameRect.setRight(option.rect.right() - horizontalMargin() - 1);
	return nameRect;
}

void YaContactListViewDelegate::realDrawContact(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	drawEditorBackground(painter, option, index);
	doAvatar(painter, option, index);

        // do we draw the phone icon?
        if (index.data(ContactListModel::CallableRole).toBool()) {
                QRect phoneIconRect = QRect(0, 0, 16, 16);
                phoneIconRect.moveTopRight(
                        avatarRect(option.rect).topLeft()
                        - QPoint(horizontalMargin_, 0));
                phoneIcon.paint(painter, phoneIconRect);
        }

	bool selected = option.state & QStyle::State_Selected;

	QRect textRect = this->textRect(option.rect);
	// painter->drawRect(textRect);

	QRect nameRect = this->nameRect(option, index);

	QStyleOptionViewItemV2 n_o = nameStyle(selected, statusType(index), Ya::VisualUtil::RosterStyleNormal, hovered());

	if (drawStatusIcon(statusType(index))) {
		QPixmap statusPixmap = Ya::VisualUtil::rosterStatusPixmap(statusType(index));
		QRect statusPixmapRect(nameRect);
		statusPixmapRect.setSize(statusPixmap.size());
		statusPixmapRect.moveTop(nameRect.top() + n_o.fontMetrics.ascent() - statusPixmapRect.height() + 1);
		statusPixmapRect.moveLeft(nameRect.left() - (statusPixmap.width() + 3));
		painter->drawPixmap(statusPixmapRect, statusPixmap);
	}

	// begin - status message
	QRect statusRect(nameRect);
        statusRect.moveTop(nameRect.bottom());
        statusRect.setHeight(textRect.height() - nameRect.height());

	QString status = statusText(index);
	if (drawStatusTypeText(index) && status.isEmpty()) {
		drawStatusTypeText(painter, n_o, &statusRect, index);
	}

        n_o.displayAlignment = Qt::AlignBottom | Qt::AlignLeft;
	drawName(painter, n_o, nameRect, nameText(option, index), index);

	if (!status.isEmpty()) {
		QStyleOptionViewItemV2 sm_o = statusTextStyle(Ya::VisualUtil::RosterStyleNormal, hovered());
		sm_o.palette.setColor(QPalette::Highlight, option.palette.color(QPalette::Highlight));
		drawStatus(painter, sm_o, statusRect, statusText(index), index);
	}
}

static int groupButtonMargin = 7;

QRect YaContactListViewDelegate::groupButtonRect(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QRect buttonRectDest(0, 0, 11, 11);
	buttonRectDest.moveTo(option.rect.left() + groupButtonMargin - 2,
	                      option.rect.top() + (option.rect.height() - buttonRectDest.height()) / 2);
	return buttonRectDest;
}

QRect YaContactListViewDelegate::groupNameRect(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QRect buttonRectDest = this->groupButtonRect(option, index);

	// TODO: use groupFontSize
	QStyleOptionViewItemV2 o = opt();
	QRect groupNameRect;
	//o.font.setBold(true);
	o.font.setPixelSize(11);
	o.fontMetrics = QFontMetrics(o.font);

	QString text = index.data(Qt::DisplayRole).toString();

	if (!index.data(ContactListModel::ExpandedRole).toBool())
		text += QString(" (%1)").arg(index.model()->rowCount(index));

	groupNameRect.setHeight(o.fontMetrics.height());
	int info_y = (option.rect.height() - o.fontMetrics.ascent()) / 2;
	groupNameRect.setLeft(buttonRectDest.right() + 6);
	groupNameRect.moveTop(option.rect.top() + info_y - 1);
	groupNameRect.setWidth(o.fontMetrics.width(text) + 5);

	if (groupNameRect.right() > option.rect.right() - groupButtonMargin)
		groupNameRect.setRight(option.rect.right() - groupButtonMargin);

        // groupNameRect.setHeight(qMax(14, groupNameRect.height()));
	return groupNameRect;
}

void YaContactListViewDelegate::drawGroup(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	// painter->drawRect(option.rect);

	QRect buttonRectDest = this->groupButtonRect(option, index);
	// painter->drawRect(buttonRectDest);

	Ya::DecorationState decorationState = Ya::Normal;
	if (hovered() && buttonRectDest.contains(hoveredPosition())) {
		decorationState = Ya::Hover;
		setHovered(false); // don't draw hovered background
	}

	drawEditorBackground(painter, option, index);
	drawBackground(painter, opt(), index);

	QPixmap button = Ya::groupPixmap(buttonRectDest.size(),
	                                 index.data(ContactListModel::ExpandedRole).toBool(),
	                                 decorationState);
	QRect buttonRect(0, 0, button.width(), button.height());
	painter->drawPixmap(buttonRectDest, button, buttonRect);

	// TODO: use groupFontSize
	QStyleOptionViewItemV2 o = opt();
	//o.font.setBold(true);
	o.font.setPixelSize(11);
	o.fontMetrics = QFontMetrics(o.font);

	QString text = index.data(Qt::DisplayRole).toString();

	if (!index.data(ContactListModel::ExpandedRole).toBool())
		text += QString(" (%1)").arg(index.model()->rowCount(index));

	o.rect = groupNameRect(option, index);

	QPalette groupNamePalette = blackText();
	groupNamePalette.setColor(QPalette::Text, Ya::VisualUtil::rosterGroupForeColor());
	o.palette = groupNamePalette;

	drawText(painter, o, o.rect, text, index);
	// painter->drawRect(o.rect);

	QPixmap background = Ya::VisualUtil::dashBackgroundPixmap();
	QRect groupRect(o.rect.right() - 1, 0, option.rect.width(), background.height());
	groupRect.moveTo(groupRect.left(),
	                 option.rect.top() + (option.rect.height() - groupRect.height()) / 2 + 1);
	painter->setClipRect(groupRect);
	groupRect.setLeft(0);
	painter->drawTiledPixmap(groupRect, background);
}

void YaContactListViewDelegate::drawAccount(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	drawBackground(painter, opt(), index);
	// painter->drawRect(option.rect);

	int buttonMargin = 7;
	QRect buttonRectDest(0, 0, 16, 16);
	buttonRectDest.moveTo(option.rect.right() - buttonRectDest.width() - buttonMargin, option.rect.top() + (option.rect.height() - buttonRectDest.height()) / 2);
	// painter->drawRect(buttonRectDest);

	// TODO: use accountFontSize
	QStyleOptionViewItemV2 o = opt();
	// o.font.setBold(true);
	o.font.setPixelSize(14);
	o.fontMetrics = QFontMetrics(o.font);

	QString text = index.data(Qt::DisplayRole).toString();

	// if (!index.data(ContactListModel::ExpandedRole).toBool())
	// 	text += QString(" (%1)").arg(account->count());

	o.rect.setHeight(o.fontMetrics.ascent());
	int info_y = (option.rect.height() - o.fontMetrics.ascent()) / 2;
	o.rect.setLeft(decorationRect().right() + 3);
	o.rect.setTop(o.rect.top() + info_y);
	o.rect.setWidth(o.fontMetrics.width(text) + 5);

	// if (o.rect.right() > buttonRectDest.left() - buttonMargin)
	// 	o.rect.setRight(buttonRectDest.left() - buttonMargin);

	QPalette blackText = o.palette;
	blackText.setColor(QPalette::Text, Ya::VisualUtil::rosterAccountForeColor());
	o.palette = blackText;

	QVariant value = index.data(Qt::DecorationRole);
	if (value.isValid()) {
		if (value.type() == QVariant::Icon) {
			QIcon icon = qvariant_cast<QIcon>(value);
			icon.paint(painter, decorationRect(), option.decorationAlignment, iconMode(), iconState());
		}
	}

	drawText(painter, o, o.rect, text, index);
	// painter->drawRect(o.rect);

	// QPixmap button = Ya::groupPixmap(buttonRectDest.size(), index.data(ContactListModel::ExpandedRole).toBool(), (option.state & QStyle::State_Selected) ? Ya::Hover : Ya::Normal);
	// QRect buttonRect(0, 0, button.width(), button.height());
	// painter->drawPixmap(buttonRectDest, button, buttonRect);
}

int YaContactListViewDelegate::avatarSize() const
{
	return avatarSize_; // ContactListViewDelegate::avatarSize();
}

void YaContactListViewDelegate::setAvatarSize(int avatarSize)
{
	avatarSize_ = avatarSize;
}

void YaContactListViewDelegate::drawText(QPainter* painter, const QStyleOptionViewItem& option, const QRect& rect, const QString& text, const QModelIndex& index) const
{
	QStyleOptionViewItem opt = option;
	// if (!(option.state & QStyle::State_Active))
	// 	opt.state &= ~QStyle::State_Selected;

	ContactListViewDelegate::drawText(painter, opt, rect, text, index);
}

void YaContactListViewDelegate::drawBackground(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	// if (option.showDecorationSelected && (option.state & QStyle::State_Selected)) {
	// 	if (!(option.state & QStyle::State_Active) && parent()) {
	// 		QWidget* parentWidget = static_cast<QWidget*>(parent());
	// 		YaContactListView* contactListView = dynamic_cast<YaContactListView*>(parentWidget);
	// 		if (contactListView && !contactListView->drawSelectionBackground()) {
	// 			return;
	// 		}
	// 	}
	// }

	ContactListViewDelegate::drawBackground(painter, option, index);
}

bool YaContactListViewDelegate::drawStatusIcon(XMPP::Status::Type type) const
{
	return drawStatusIcon() ||
	       type == XMPP::Status::Away ||
	       type == XMPP::Status::XA ||
	       type == XMPP::Status::DND ||
	       type == XMPP::Status::Blocked ||
	       type == XMPP::Status::Reconnecting ||
	       type == XMPP::Status::NotAuthorizedToSeeStatus;
}

bool YaContactListViewDelegate::drawStatusIcon() const
{
	return drawStatusIcon_;
}

void YaContactListViewDelegate::setDrawStatusIcon(bool draw)
{
	drawStatusIcon_ = draw;
}

void YaContactListViewDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QRect r;
	switch (ContactListModel::indexType(index)) {
	case ContactListModel::ContactType:
		r = this->nameRect(option, index).adjusted(-1, 0, 0, 1);
		break;
	case ContactListModel::GroupType:
		r = this->groupNameRect(option, index).adjusted(-1, 0, 0, 0);
		r.setRight(option.rect.right() - groupButtonMargin);
		break;
	// case ContactListModel::AccountType:
	// 	drawAccount(painter, opt(), index);
	// 	break;
	default:
		;
	}

	if (!r.isEmpty()) {
		editor->setGeometry(r);
	}
}

QRect YaContactListViewDelegate::rosterToolTipArea(const QRect& rect) const
{
	return rect.adjusted(0, 3, 0, 0);
}

QWidget* YaContactListViewDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QLineEdit* result = new YaEmptyTextLineEdit(parent);
	result->setFrame(false);

	contactList()->setEditingIndex(index, true);

	return result;
}

// we're preventing modifications of QLineEdit while it's still being displayed,
// and the contact we're editing emits dataChanged()
void YaContactListViewDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(editor);
	if (lineEdit && !lineEdit->text().isEmpty()) {
		return;
	}

	ContactListViewDelegate::setEditorData(editor, index);
}

// adapted from QItemDelegate::eventFilter()
bool YaContactListViewDelegate::eventFilter(QObject* object, QEvent* event)
{
	QWidget *editor = ::qobject_cast<QWidget*>(object);
	QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(editor);
	if (!editor || !lineEdit)
		return false;

	if (event->type() == QEvent::KeyPress) {
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Home) {
			lineEdit->setCursorPosition(0);
			return true;
		}
		else if (keyEvent->key() == Qt::Key_Down || keyEvent->key() == Qt::Key_End) {
			lineEdit->setCursorPosition(lineEdit->text().length());
			return true;
		}
		else if (keyEvent->key() == Qt::Key_PageUp || keyEvent->key() == Qt::Key_PageDown) {
			return true;
		}
	}

	// make sure data is not committed
	if (event->type() == QEvent::FocusOut) {
		if (!editor->isActiveWindow() || (QApplication::focusWidget() != editor)) {
			QWidget *w = QApplication::focusWidget();
			while (w) { // don't worry about focus changes internally in the editor
				if (w == editor)
					return false;
				w = w->parentWidget();
			}
// #ifndef QT_NO_DRAGANDDROP
// 			// The window may lose focus during an drag operation.
// 			// i.e when dragging involves the taskbar on Windows.
// 			if (QDragManager::self() && QDragManager::self()->object != 0)
// 				return false;
// #endif
			// Opening a modal dialog will start a new eventloop
			// that will process the deleteLater event.
			if (QApplication::activeModalWidget() && !QApplication::activeModalWidget()->isAncestorOf(editor))
				return false;
			// emit commitData(editor);
			emit closeEditor(editor, NoHint);
		}
		return false;
	}

	return ContactListViewDelegate::eventFilter(object, event);
}
