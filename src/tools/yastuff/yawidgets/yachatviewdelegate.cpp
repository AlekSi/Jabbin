/*
 * yachatviewdelegate.cpp - painting of custom chatlog items
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

#include "yachatviewdelegate.h"

#include <QPainter>
#include <QDateTime>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QTextDocumentFragment>
#include <QAbstractTextDocumentLayout>
#include <QRegExp>
#include <QAbstractItemView>

#include <QTextFrame>
#include <private/qtextcontrol_p.h>

#ifndef NO_YAHIGHLIGHTERS
#define YACHATVIEW_HIGHLIGHTERS
#endif

#ifdef YACHATVIEW_HIGHLIGHTERS
#include "wikihighlighter.h"
#include "quotationhighlighter.h"
#ifdef YAPSI_DEV
#include "cpphighlighter.h"
#endif
#endif

#ifndef NO_PSIRICHTEXT
#include "psirichtext.h"
#endif

#include "yacommon.h"
#include "yastyle.h"
#include "textutil.h"

struct DelegateMessageData {
	QSize sizeHint;
	QTextControl* textControl;

	DelegateMessageData()
		: sizeHint(-1, -1)
		, textControl(0)
	{}
};

Q_DECLARE_METATYPE(DelegateMessageData);
// qRegisterMetaType<DelegateMessageData>("DelegateMessageData");


static const int leftMargin = 7;
static const int rightMargin = 5;
static const int bottomMargin = 4;
static const int mergedTopMargin = 1;
static const int lineMargin = 2;
static const int linePadding = 6;
static const int timePadding = 3;
static const int namePadding = 4;
static const int messageLeftMargin = 16;

#define FIRST_RED_LETTER

YaChatViewDelegate::YaChatViewDelegate(QAbstractItemView* parent)
	: QItemDelegate(static_cast<QWidget*>(parent))
	, width_(-1)
{
	lineColor_ = QColor(0xE5,0xE5,0xE5);
	timeColor_ = QColor(0x99,0x99,0x99);
	nameColor_ = QColor(0x00,0x00,0x00);
	nameFirstLetterColor_ = QColor(0xFF,0x00,0x00);
	contentsColor_ = nameColor_;

	nameFont_ = QFont("Arial");
	nameFont_.setBold(true);
	nameFont_.setUnderline(true);

	timeFont_ = QFont("Verdana");

	contentsFont_ = QFont("Arial");
	contentsFont_.setPixelSize(11);

	dateHeaderFont_ = QFont("Arial");

#ifdef Q_WS_WIN
	nameFont_.setPixelSize(13);
	timeFont_.setPixelSize(11);
	dateHeaderFont_.setPixelSize(18);
#else
	nameFont_.setPixelSize(11);
	timeFont_.setPixelSize(10);
	dateHeaderFont_.setPixelSize(16);
#endif

	nameFontMetrics_ = new QFontMetrics(nameFont_);
	timeFontMetrics_ = new QFontMetrics(timeFont_);
	contentsFontMetrics_ = new QFontMetrics(contentsFont_);
	dateHeaderFontMetrics_ = new QFontMetrics(dateHeaderFont_);
}

YaChatViewDelegate::~YaChatViewDelegate()
{
	delete nameFontMetrics_;
	delete timeFontMetrics_;
	delete contentsFontMetrics_;
	delete dateHeaderFontMetrics_;
}

int YaChatViewDelegate::width() const
{
	return width_;
}

void YaChatViewDelegate::setWidth(int width)
{
	width_ = width;
}

void YaChatViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if (!index.isValid())
		return;

	switch (type(index)) {
	case YaChatViewModel::DateHeader:
		drawDateHeader(painter, option, index);
		break;
	case YaChatViewModel::DummyHeader:
		break;
	case YaChatViewModel::Message:
	default:
		Q_ASSERT(type(index) == YaChatViewModel::Message || type(index) == YaChatViewModel::MoodChanged || type(index) == YaChatViewModel::StatusTypeChangedNotice || index.data(YaChatViewModel::PersistentNoticeRole).toBool());
		drawMessage(painter, option, index);
	}
}

QSize YaChatViewDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if (!index.isValid())
		return QSize();

	switch (type(index)) {
	case YaChatViewModel::DateHeader:
		return dateHeaderSizeHint(option, index);
	case YaChatViewModel::DummyHeader:
		return QSize(5, 5);
	case YaChatViewModel::Message:
	default:
		Q_ASSERT(type(index) == YaChatViewModel::Message || type(index) == YaChatViewModel::MoodChanged || type(index) == YaChatViewModel::StatusTypeChangedNotice || index.data(YaChatViewModel::PersistentNoticeRole).toBool());
		return messageSizeHint(option, index);
	}

	return QSize();
}

bool YaChatViewDelegate::isHoveringText(const QModelIndex& index, const QPoint& pos) const
{
	if (!index.isValid())
		return false;

	switch (type(index)) {
	case YaChatViewModel::Message: {
		ensureMessageLayout(QStyleOptionViewItem(), index);
		DelegateMessageData data = ensureDelegateMessageData(index);
		QRect contentsRect(QPoint(0, 0), data.textControl->size().toSize());
		contentsRect.moveTopLeft(messageTextControlPosition(index));
		return contentsRect.contains(pos);
	}
	default:
		;
	}

	return false;
}

bool YaChatViewDelegate::isHoveringTime(const QModelIndex& index, const QPoint& pos) const
{
	if (!index.isValid())
		return false;

	switch (type(index)) {
	case YaChatViewModel::Message: {
		ensureMessageLayout(QStyleOptionViewItem(), index);

		int y = 0;
		y += linePadding;
		QRect timeRect(0, 0, parentWidget()->viewport()->width(), 100);
		QString time = "mm:mm";
		timeRect.setTop(timeRect.top() + y + timePadding);
		timeRect.setLeft(timeRect.right() - rightMargin - timeFontMetrics_->width(time));
		timeRect.setHeight(timeFontMetrics_->height());

		return timeRect.contains(pos);
	}
	default:
		;
	}

	return false;
}

bool YaChatViewDelegate::isHoveringEmoticon(const QModelIndex& index, const QPoint& pos) const
{
	if (!index.isValid())
		return false;

	switch (type(index)) {
	case YaChatViewModel::Message: {
		return !messageEmoticonTextUnder(index, pos).isEmpty();
	}
	default:
		;
	}

	return false;
}

QPoint YaChatViewDelegate::messageTextControlPosition(const QModelIndex& index) const
{
	int y = 0;

	if (!index.data(YaChatViewModel::MergeRole).toBool()) {
		y += linePadding;
		y += nameFontMetrics_->height() + namePadding;
	}
	else {
		y += mergedTopMargin;
	}

	return QPoint(leftMargin + messageLeftMargin, y);
}

void YaChatViewDelegate::sendControlEvent(const QModelIndex& index, QMouseEvent* e, const QPointF& coordinateOffset, QWidget* contextWidget)
{
	if (!index.isValid())
		return;

	if (e->type() == QEvent::MouseButtonPress && e->pos() != QPoint(-1, -1))
		mousePressPos_ = e->pos();

	switch (type(index)) {
	case YaChatViewModel::DateHeader:
	case YaChatViewModel::DummyHeader:
		break;
	case YaChatViewModel::Message:
	default:
		sendControlEventMessage(index, e, coordinateOffset - messageTextControlPosition(index), contextWidget);
		// Q_ASSERT(type(index) == YaChatViewModel::MoodChanged || type(index) == YaChatViewModel::StatusTypeChangedNotice || index.data(YaChatViewModel::PersistentNoticeRole).toBool());
	}
}

DelegateMessageData YaChatViewDelegate::ensureDelegateMessageData(const QModelIndex& index) const
{
	QModelIndex* mi = (QModelIndex*)(&index);
	return mi->data(YaChatViewModel::DelegateDataRole).value<DelegateMessageData>();
}

void YaChatViewDelegate::saveDelegateMessageData(const DelegateMessageData& data, const QModelIndex& index) const
{
	QVariant v;
	v.setValue(data);
	QAbstractItemModel* model = (QAbstractItemModel*)index.model();
	model->blockSignals(true);
	model->setData(index, v, YaChatViewModel::DelegateDataRole);
	model->blockSignals(false);
}

void YaChatViewDelegate::ensureDocumentLayouted(QTextDocument* doc, int documentWidth) const
{
	QTextOption opt = doc->defaultTextOption();

	opt.setAlignment(Qt::AlignLeft);
	opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
	opt.setTextDirection(Qt::LeftToRight);
	doc->setDefaultTextOption(opt);

	QTextFrameFormat fmt = doc->rootFrame()->frameFormat();
	fmt.setMargin(0);
	doc->rootFrame()->setFrameFormat(fmt);
	doc->setTextWidth(documentWidth);
}

void YaChatViewDelegate::ensureMessageLayout(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	DelegateMessageData data = ensureDelegateMessageData(index);
	bool updateSizeHint = false;
	if (width() != data.sizeHint.width()) {
		// qWarning("re-layout!!");
		if (!data.textControl) {
			QTextControl* control = new QTextControl((QObject*)this); // should set correct parent in order to use model with multiple views

			connect(control, SIGNAL(updateRequest(QRectF)),  SLOT(updateRequest(QRectF)));
			connect(control, SIGNAL(linkHovered(QString)),   SIGNAL(linkHovered(QString)));
			connect(control, SIGNAL(linkActivated(QString)), SIGNAL(linkActivated(QString)));

			QTextDocument* doc = control->document();
#ifndef NO_PSIRICHTEXT
			PsiRichText::install(doc);
#endif

#ifdef YACHATVIEW_HIGHLIGHTERS
			CombinedSyntaxHighlighter* hl = new CombinedSyntaxHighlighter(doc);
			new QuotationHighlighter(hl, doc);
			new WikiHighlighter(hl, doc);
#ifdef YAPSI_DEV
			new CppHighlighter(hl, doc);
#endif
#endif

			QPalette pal = option.palette;
			if (type(index) == YaChatViewModel::ContactComposing) {
				pal.setColor(QPalette::Text, QColor(0xCC, 0x99, 0x66));
			}
			else {
				pal = YaStyle::useNativeTextSelectionColors(pal);
			}
			data.textControl = control;
			doc->setUndoRedoEnabled(false);
			doc->setDefaultFont(parentWidget()->font());
			control->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
			control->setOpenExternalLinks(false);
			control->setPalette(pal);
			control->setFocus(false);

			QString messageText = index.data(Qt::DisplayRole).toString();
			if (type(index) == YaChatViewModel::ContactComposing ||
			    type(index) == YaChatViewModel::UserIsOfflineNotice ||
			    type(index) == YaChatViewModel::UserIsBlockedNotice ||
			    type(index) == YaChatViewModel::UserNotInListNotice ||
			    type(index) == YaChatViewModel::StatusTypeChangedNotice)
			{
				messageText = messageSenderName(index.data(YaChatViewModel::IncomingRole).toBool()) + messageText;
			}

#ifdef NO_PSIRICHTEXT
			if (index.data(YaChatViewModel::MessagePlainTextRole).toBool())
				doc->setPlainText(messageText);
			else
				doc->setHtml(messageText);
#else
			QTextCursor cursor = control->textCursor();
			PsiRichText::appendText(doc, cursor, messageText);
#endif

			doc->setUndoRedoEnabled(false);
		}

		int mainTextWidth = width() - messageTextControlPosition(index).x();
		ensureDocumentLayouted(data.textControl->document(), mainTextWidth);
		updateSizeHint = true;

		// TODO: think of a way to refactor all of this
		// data.textControl->document()->setTextWidth(mainTextWidth);
	}

	if (data.textControl) {
		QTextDocument* doc = data.textControl->document();
		Q_ASSERT(doc);
		if (doc->defaultFont() != parentWidget()->font()) {
			doc->setDefaultFont(parentWidget()->font());
			ensureDocumentLayouted(doc, (int)doc->textWidth());

			updateSizeHint = true;
		}

		if (updateSizeHint) {
			// sizeHint
			data.textControl->setTextWidth(doc->textWidth());
			data.sizeHint = data.textControl->size().toSize();

			data.sizeHint.setWidth(width());

			if (!index.data(YaChatViewModel::MergeRole).toBool()) {
				data.sizeHint += QSize(0, linePadding);
				data.sizeHint += QSize(0, nameFontMetrics_->height() + namePadding);
				data.sizeHint += QSize(0, bottomMargin);
			}
			else {
				data.sizeHint += QSize(0, mergedTopMargin + bottomMargin);
			}

			saveDelegateMessageData(data, index);
		}
	}
}

QString YaChatViewDelegate::messageSenderName(bool incoming) const
{
	if (incoming)
		return contactNick();

	return selfNick();
}

QString YaChatViewDelegate::messageSenderNameAsRichText(bool incoming) const
{
	QString result = messageSenderName(incoming);
	if (result.length() > 0) {
		QString tmp = result;
#ifdef FIRST_RED_LETTER
		result = QString("<b><font color='red'>%1</font>").arg(tmp.at(0));
		if (tmp.length() > 1) {
			result += tmp.mid(1);
		}
		result += "</b> ";
#else
		result = QString("<b>%1</b>").arg(tmp);
#endif
	}
	return result;
}

void YaChatViewDelegate::drawMessageName(QPainter* painter, QRect& nameRect, bool incoming) const
{
	QString name = TextUtil::unescape(messageSenderName(incoming));

	if (!name.isEmpty()) {
		painter->setFont(nameFont_);
#ifndef FIRST_RED_LETTER
		painter->setPen(nameColor_);
		painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine,
		                  nameFontMetrics_->elidedText(name, Qt::ElideRight, nameRect.width()));
#else
		QString firstChar = name.mid(0, 1);
		painter->setPen(nameFirstLetterColor_);
		painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, firstChar);

		nameRect.setLeft(nameRect.left() + nameFontMetrics_->width(firstChar));
		name = name.mid(1);
		painter->setPen(nameColor_);
		painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine,
		                  nameFontMetrics_->elidedText(name, Qt::ElideRight, nameRect.width()));
#endif
	}
}

void YaChatViewDelegate::drawMessage(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	ensureMessageLayout(option, index);

	// drawBackground(painter, option, index);
	DelegateMessageData data = ensureDelegateMessageData(index);

	int y = 0;

	painter->save();
	if (spooledType(index) != YaChatViewModel::Spooled_None && selectedText(index, SelectedTextOnly).isEmpty()) {
		if (spooledType(index) == YaChatViewModel::Spooled_History)
			painter->setOpacity(0.5);
		else
			painter->setOpacity(0.75);
	}

	if (type(index) == YaChatViewModel::ContactComposing) {
		painter->fillRect(option.rect, QColor(0xFF,0xFE,0xC8));
	}
	else if (type(index) == YaChatViewModel::MoodChanged) {
		painter->fillRect(option.rect, QColor(0xDF,0xED,0xFF));
	}
#ifdef YAPSI
	else if (index.data(YaChatViewModel::SpamRole).toInt()) { // this is more valued, than EmoteRole
		painter->fillRect(option.rect, QColor(0xFF,0x99,0x99));
	}
#endif
	else if (index.data(YaChatViewModel::EmoteRole).toBool()) {
		painter->fillRect(option.rect, QColor(0xEE,0xEE,0xEE));
	}

	if (!index.data(YaChatViewModel::MergeRole).toBool()) {
		painter->setPen(lineColor_);
		QPoint lineOffset(lineMargin, 0);
		painter->drawLine(option.rect.topLeft() + lineOffset, option.rect.topRight() - lineOffset);
		y += linePadding;

		QRect timeRect(option.rect);
		QString time = index.data(YaChatViewModel::DateTimeRole).toDateTime().toString("hh:mm");
		timeRect.setTop(timeRect.top() + y + timePadding);
		timeRect.setLeft(timeRect.right() - rightMargin - timeFontMetrics_->width(time));
		timeRect.setHeight(timeFontMetrics_->height());
		painter->setPen(timeColor_);
		painter->setFont(timeFont_);
		painter->drawText(timeRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, time);

		QRect nameRect(option.rect);
		nameRect.setTop(nameRect.top() + y);
		nameRect.setLeft(leftMargin);
		nameRect.setRight(timeRect.left());
		nameRect.setHeight(nameFontMetrics_->height());

		drawMessageName(painter, nameRect, index.data(YaChatViewModel::IncomingRole).toBool());

		y += nameFontMetrics_->height() + namePadding;
	}
	else {
		// (Qt 4.3.3/Windows) nasty workaround for disappearing transparency
		// on mouse hover
		painter->setPen(lineColor_);

		y += mergedTopMargin;
	}

	Q_ASSERT(y == messageTextControlPosition(index).y());
	painter->translate(option.rect.topLeft() + messageTextControlPosition(index));
	painter->setClipRect(option.rect.translated(-option.rect.x(), -option.rect.y()));
	// data.textControl->setPalette(option.palette);
	data.textControl->drawContents(painter, QRectF(), 0);
	painter->restore();

	// painter->save();
	// painter->translate(option.rect.topLeft());
	// QRect contentsRect(QPoint(0, 0), data.textControl->size().toSize());
	// contentsRect.moveTopLeft(messageTextControlPosition(index));
	// painter->drawRect(contentsRect);
	// painter->restore();
}

QSize YaChatViewDelegate::messageSizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	ensureMessageLayout(option, index);
	DelegateMessageData data = ensureDelegateMessageData(index);
	return data.sizeHint;
}

void YaChatViewDelegate::drawDateHeader(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QString text = Ya::DateFormatter::instance()->dateAndWeekday(index.data(YaChatViewModel::DateTimeRole).toDate());

	QRect rect(option.rect);
	rect.setLeft(rect.left() + leftMargin);
	rect.setHeight(dateHeaderFontMetrics_->height());
	painter->setPen(timeColor_);
	painter->setFont(dateHeaderFont_);
	painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine,
	                  dateHeaderFontMetrics_->elidedText(text, Qt::ElideRight, rect.width()));
}

QSize YaChatViewDelegate::dateHeaderSizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	Q_UNUSED(option);
	Q_UNUSED(index);
	return QSize(10, dateHeaderFontMetrics_->height());
}

// TODO: unify with messageEmoticonNameUnder
QString YaChatViewDelegate::messageEmoticonTextUnder(const QModelIndex& index, const QPoint& pos) const
{
	if (!index.isValid())
		return false;

	ensureMessageLayout(QStyleOptionViewItem(), index);
	DelegateMessageData data = ensureDelegateMessageData(index);
	QTextDocument* document = data.textControl->document();
	return PsiRichText::iconTextAt(document, pos - messageTextControlPosition(index));
}

// TODO: unify with messageEmoticonTextUnder
QString YaChatViewDelegate::messageEmoticonNameUnder(const QModelIndex& index, const QPoint& pos) const
{
	if (!index.isValid())
		return false;

	ensureMessageLayout(QStyleOptionViewItem(), index);
	DelegateMessageData data = ensureDelegateMessageData(index);
	QTextDocument* document = data.textControl->document();
	return PsiRichText::iconNameAt(document, pos - messageTextControlPosition(index));
}

void YaChatViewDelegate::sendControlEventMessage(const QModelIndex& index, QMouseEvent* e, const QPointF& coordinateOffset, QWidget* contextWidget)
{
	ensureMessageLayout(QStyleOptionViewItem(), index);
	DelegateMessageData data = ensureDelegateMessageData(index);
	QPoint epos = e->pos() + coordinateOffset.toPoint();

#if 0
	if (e->type() == QEvent::MouseButtonRelease && e->pos() == mousePressPos_) {
		// qWarning() << e->pos() << coordinateOffset.toPoint() << epos;
		QTextDocument* document = data.textControl->document();
		QString iconText = messageEmoticonTextUnder(index, epos + messageTextControlPosition(index));
		if (!iconText.isEmpty()) {
			QTextCursor cursor = PsiRichText::textIconCursorAt(document, epos);
			QTextCharFormat charFormat = cursor.charFormat();
			charFormat.setProperty(TextIconFormat::DrawIcon, !charFormat.boolProperty(TextIconFormat::DrawIcon));
			cursor.setCharFormat(charFormat);
			emit updateRequest();
		}
	}
#endif

	QMouseEvent e2(e->type(), epos, e->button(), e->buttons(), e->modifiers());
	data.textControl->processEvent(&e2, QPoint(0, 0), contextWidget);
}

YaChatViewModel::Type YaChatViewDelegate::type(const QModelIndex& index) const
{
	return YaChatViewModel::type(index);
}

YaChatViewModel::SpooledType YaChatViewDelegate::spooledType(const QModelIndex& index) const
{
	return YaChatViewModel::spooledType(index);
}

void YaChatViewDelegate::updateRequest(QRectF)
{
	emit updateRequest();
}

void YaChatViewDelegate::indexAboutToBeRemoved(const QModelIndex& index)
{
	switch (type(index)) {
	case YaChatViewModel::DateHeader:
	case YaChatViewModel::DummyHeader:
		break;
	case YaChatViewModel::Message:
	default:
	{
		Q_ASSERT(type(index) == YaChatViewModel::Message || type(index) == YaChatViewModel::MoodChanged || type(index) == YaChatViewModel::StatusTypeChangedNotice || index.data(YaChatViewModel::PersistentNoticeRole).toBool());
		DelegateMessageData data = ensureDelegateMessageData(index);
		if (data.textControl)
			delete data.textControl;
		break;
	}
	}
}

const QString& YaChatViewDelegate::selfNick() const
{
	return selfNick_;
}

const QString& YaChatViewDelegate::contactNick() const
{
	return contactNick_;
}

void YaChatViewDelegate::nicksChanged(QString selfNick, QString contactNick)
{
	selfNick_ = selfNick;
	contactNick_ = contactNick;
}

QString YaChatViewDelegate::selectedText(const QModelIndex& index, SelectedText selectedText) const
{
	if (!index.isValid())
		return QString();

	switch (type(index)) {
	case YaChatViewModel::DateHeader:
	case YaChatViewModel::DummyHeader:
		return QString();
	case YaChatViewModel::Message:
	default:
		return selectedMessageText(index, selectedText);
		// Q_ASSERT(type(index) == YaChatViewModel::MoodChanged || type(index) == YaChatViewModel::StatusTypeChangedNotice || index.data(YaChatViewModel::PersistentNoticeRole).toBool());
		// return QString();
	}

	return QString();
}

QString YaChatViewDelegate::selectedHtml(const QModelIndex& index, SelectedText selectedText) const
{
	if (!index.isValid())
		return QString();

	switch (type(index)) {
	case YaChatViewModel::DateHeader:
	case YaChatViewModel::DummyHeader:
		return QString();
	case YaChatViewModel::Message:
	default:
		return selectedMessageHtml(index, selectedText);
		// Q_ASSERT(type(index) == YaChatViewModel::MoodChanged || type(index) == YaChatViewModel::StatusTypeChangedNotice || index.data(YaChatViewModel::PersistentNoticeRole).toBool());
		// return QString();
	}

	return QString();
}

QString YaChatViewDelegate::selectedMessageText(const QModelIndex& index, SelectedText selectedText) const
{
	DelegateMessageData data = ensureDelegateMessageData(index);

	QTextDocument *doc = new QTextDocument();
	QTextCursor cursor(doc);
	cursor.insertFragment(data.textControl->textCursor().selection());
	// Note: make sure that copying emote messages also work
	QString result = PsiRichText::convertToPlainText(doc);
	delete doc;

	if (selectedText == SelectedTextWithHeader) {
		QString timestr = index.data(YaChatViewModel::DateTimeRole).toDateTime().toString("[hh:mm:ss]");
		QString formatStr = QString("%1 <%2> %3");
		if (index.data(YaChatViewModel::EmoteRole).toBool())
			formatStr = QString("%1 *%2 %3");

		result = formatStr.arg(timestr)
		                  .arg(TextUtil::unescape(messageSenderName(index.data(YaChatViewModel::IncomingRole).toBool())))
		                  .arg(result);
	}

	return result;
}

QString YaChatViewDelegate::selectedMessageHtml(const QModelIndex& index, SelectedText selectedText) const
{
	DelegateMessageData data = ensureDelegateMessageData(index);

	QTextDocument *doc = new QTextDocument();
	QTextCursor cursor(doc);
	cursor.insertFragment(data.textControl->textCursor().selection());
	// Note: make sure that copying emote messages also work
	QString result = Qt::escape(PsiRichText::convertToPlainText(doc));
	delete doc;

	if (selectedText == SelectedTextWithHeader) {
		QString timestr = index.data(YaChatViewModel::DateTimeRole).toDateTime().toString("hh:mm:ss");
		QString emoteHtmlOpen = index.data(YaChatViewModel::EmoteRole).toBool() ?
		                        "<div style=\"background-color: #f6f6f6; font: normal"
		                        " 80% Arial, sans-serif; padding: 0.2em 0;\">" :
		                        "<div>";
		QString emoteHtmlClose = index.data(YaChatViewModel::EmoteRole).toBool() ?
		                         "&nbsp;&nbsp;" :
		                         "</div><div style=\"font: 80% Arial, sans-serif; line-height:1.3em;"
		                         " color: #000; padding: .5em 0 .5em 1.5em;\">";
		QString formatStr = "<div style=\"background: #fff; margin: 0;\">"
		                    "<div style=\"border-top: 1px solid #e5e5e5; padding-top:0px;\">"
		                    "<div style=\"float:right; font: 70% Verdana, Arial, sans-serif;"
		                    " color: #999; padding: 2px 0.4em 0 0;\">%1</div>" +
		                    emoteHtmlOpen + "<div style=\"font: bold 80% Arial, sans-serif;"
		                    " color: #333; padding: 0.2em 0.4em; margin: 0;\">%2</div>" +
		                    emoteHtmlClose + "%3</div></div></div>";

		result = formatStr.arg(timestr)
		         .arg(messageSenderName(index.data(YaChatViewModel::IncomingRole).toBool()))
		         .arg(result);
	}

	return result;
}

QAbstractItemView* YaChatViewDelegate::parentWidget() const
{
	return static_cast<QAbstractItemView*>(parent());
}
