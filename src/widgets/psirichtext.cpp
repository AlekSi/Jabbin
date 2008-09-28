/*
 * psirichtext.h - helper functions to handle Icons in QTextDocuments
 * Copyright (C) 2006  Michail Pishchagin
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "psirichtext.h"

#include <QApplication>
#include <QTextDocumentFragment>
#include <QTextCharFormat>
#include <QAbstractTextDocumentLayout> // for QTextObjectInterface
#include <QPainter>
#include <QRegExp>
#include <QVariant>
#include <QFont>
#include <QList>
#include <QQueue>
#include <QTextFrame>

#include "textutil.h"

#ifndef WIDGET_PLUGIN
#include "iconset.h"
#else
class PsiIcon;
class Iconset;
#endif

//----------------------------------------------------------------------------
// TextIconFormat
//----------------------------------------------------------------------------

const int TextIconFormat::IconFormatType = QTextFormat::UserObject + 1;

TextIconFormat::TextIconFormat(const QString &iconName, const QString &text)
	: QTextCharFormat()
{
	Q_UNUSED(text);

	setObjectType(TextIconFormat::IconFormatType);
	QTextFormat::setProperty(IconName, iconName);
	QTextFormat::setProperty(IconText, text);
	QTextFormat::setProperty(DrawIcon, true);

	// TODO: handle animations
}

//----------------------------------------------------------------------------
// IconTextObjectInterface
//----------------------------------------------------------------------------

#ifndef WIDGET_PLUGIN

class TextIconHandler : public QObject, public QTextObjectInterface
{
	Q_OBJECT
	Q_INTERFACES(QTextObjectInterface)
public:
	TextIconHandler(QObject *parent = 0);

	virtual QSizeF intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format);
	virtual void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format);
};

TextIconHandler::TextIconHandler(QObject *parent)
	: QObject(parent)
{
}

QSizeF TextIconHandler::intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
	Q_UNUSED(doc);
	Q_UNUSED(posInDocument)
	const QTextCharFormat charFormat = format.toCharFormat();

	return IconsetFactory::iconPixmap(charFormat.stringProperty(TextIconFormat::IconName)).size();
}

void TextIconHandler::drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
	Q_UNUSED(doc);
	Q_UNUSED(posInDocument);
	const QTextCharFormat charFormat = format.toCharFormat();

	if (charFormat.boolProperty(TextIconFormat::DrawIcon)) {
		const QPixmap pixmap = IconsetFactory::iconPixmap(charFormat.stringProperty(TextIconFormat::IconName));
		painter->drawPixmap(rect, pixmap, pixmap.rect());
	}
	else {
		painter->save();
		QFont font = painter->font();
		font.setUnderline(false);
		font.setBold(false);
		font.setItalic(false);
		painter->setFont(font);

		painter->drawText(rect,
		                  Qt::AlignCenter | Qt::AlignBottom | Qt::TextSingleLine,
		                  charFormat.stringProperty(TextIconFormat::IconText));
		painter->restore();
	}
}

#endif // WIDGET_PLUGIN

//----------------------------------------------------------------------------
// PsiRichText
//----------------------------------------------------------------------------

/**
 * You need to call this function on your QTextDocument to make it 
 * capable of displaying inline Icons. Uninstaller ships separately.
 */
void PsiRichText::install(QTextDocument *doc)
{
	Q_ASSERT(doc);
#ifndef WIDGET_PLUGIN
	static TextIconHandler *handler = 0;
	if (!handler)
		handler = new TextIconHandler(qApp);
	
	doc->documentLayout()->registerHandler(TextIconFormat::IconFormatType, handler);
#endif
}

/**
 * Make sure that QTextDocument has correctly layouted its text.
 */
void PsiRichText::ensureTextLayouted(QTextDocument *doc, int documentWidth, Qt::Alignment align, Qt::LayoutDirection layoutDirection, bool textWordWrap)
{
	// from QLabelPrivate::ensureTextLayouted

	Q_UNUSED(textWordWrap);
	Q_UNUSED(layoutDirection);
	Q_UNUSED(align);
	// bah, QTextDocumentLayout is private :-/
	// QTextDocumentLayout *lout = qobject_cast<QTextDocumentLayout *>(doc->documentLayout());
	// Q_ASSERT(lout);
	// 
	// int flags = (textWordWrap ? 0 : Qt::TextSingleLine) | align;
	// flags |= (layoutDirection == Qt::RightToLeft) ? QTextDocumentLayout::RTL : QTextDocumentLayout::LTR;
	// lout->setBlockTextFlags(flags);
	// 
	// if (textWordWrap) {
	// 	// ensure that we break at words and not just about anywhere
	// 	lout->setWordWrapMode(QTextOption::WordWrap);
	// }

	QTextFrameFormat fmt = doc->rootFrame()->frameFormat();
	fmt.setMargin(0);
	doc->rootFrame()->setFrameFormat(fmt);
	doc->setTextWidth(documentWidth);
}

/**
 * Inserts an PsiIcon into document.
 * \param cursor this cursor is used to insert icon
 * \param iconName icon's name, by which it could be found in IconsetFactory
 * \param iconText icon's text, used when copy operation is performed
 */
void PsiRichText::insertIcon(QTextCursor &cursor, const QString &iconName, const QString &iconText)
{
#ifdef WIDGET_PLUGIN
	Q_UNUSED(cursor);
	Q_UNUSED(iconName);
	Q_UNUSED(iconText);
#else
	QTextCharFormat format = cursor.charFormat();
	
	TextIconFormat icon(iconName, iconText);
	cursor.insertText(QString(QChar::ObjectReplacementCharacter), icon);
	
	cursor.setCharFormat(format);
#endif
}

typedef QQueue<TextIconFormat *> TextIconFormatQueue;

/**
 * Adds null format to queue for all ObjectReplacementCharacters that were
 * already in the text. Returns passed \param text to save some code.
 */
static QString preserveOriginalObjectReplacementCharacters(QString text, TextIconFormatQueue *queue)
{
	int objReplChars = 0;
	objReplChars += text.count(QChar::ObjectReplacementCharacter);
	// <img> tags are replaced to ObjectReplacementCharacters
	// internally by Qt functions.
	// But we must be careful if some other character instead of
	// 0x20 is used immediately after tag opening, this could
	// create a hole. ejabberd protects us from it though.
	objReplChars += text.count("<img ");
	for (int i = objReplChars; i; i--)
		queue->enqueue(0);

	return text;
}

/**
 * Replaces all <icon> tags with handy ObjectReplacementCharacters, and
 * adds appropriate format to the \param queue. Returns processed 
 * \param text.
 */
static QString convertIconsToObjectReplacementCharacters(QString text, TextIconFormatQueue *queue)
{
	// Format: <icon name="" text="">
	static QRegExp rxName("name=\"([^\"]+)\"");
	static QRegExp rxText("text=\"([^\"]+)\"");
	
	QString result;
	QString work = text;

	forever {
		int start = work.indexOf("<icon");
		if (start == -1)
			break;
		
		result += preserveOriginalObjectReplacementCharacters(work.left(start), queue);
		
		int end = work.indexOf(">", start);
		Q_ASSERT(end != -1);
		
		QString fragment = work.mid(start, end - start);
		if (rxName.indexIn(fragment) != -1) {
			QString iconName = TextUtil::unescape(rxName.capturedTexts()[1]);
			QString iconText;
			if (rxText.indexIn(fragment) != -1)
				iconText = TextUtil::unescape(rxText.capturedTexts()[1]);
			
			queue->enqueue(new TextIconFormat(iconName, iconText));
			result += QChar::ObjectReplacementCharacter;
		}
		
		work = work.mid(end + 1);
	}
	
	return result + preserveOriginalObjectReplacementCharacters(work, queue);
}

/**
 * Applies text formats from \param queue to all ObjectReplacementCharacters
 * in \param doc, starting from \param cursor's position.
 */
static void applyFormatToIcons(QTextDocument *doc, TextIconFormatQueue *queue, QTextCursor &cursor)
{
	QTextCursor searchCursor = cursor;
	forever {
		searchCursor = doc->find(QString(QChar::ObjectReplacementCharacter), searchCursor);
		if (searchCursor.isNull())
			break;
		
		Q_ASSERT(!queue->isEmpty());
		TextIconFormat *format = queue->dequeue();
		if (format) { 
			searchCursor.setCharFormat(*format);
			delete format;
		}
	}
	
	// if it's not true, there's a memleak
	Q_ASSERT(queue->isEmpty());
	
	// clear the selection that's left after successful QTextDocument::find()
	cursor.clearSelection();
}

/**
 * Groups some related function calls together.
 */
static void appendTextHelper(QTextDocument *doc, QString text, QTextCursor &cursor)
{
	TextIconFormatQueue queue;
	
	// we need to save this to start searching from 
	// here when applying format to icons
	int initialpos = cursor.position();
	cursor.insertFragment(QTextDocumentFragment::fromHtml(convertIconsToObjectReplacementCharacters(text, &queue)));
	cursor.setPosition(initialpos);
	
	applyFormatToIcons(doc, &queue, cursor);
}

/**
 * Sets entire contents of specified QTextDocument to text.
 * \param text text to append to the QTextDocument. Please note that if you
 *             insert any <icon>s, attributes' values MUST be Qt::escaped.
 */
void PsiRichText::setText(QTextDocument *doc, const QString &text)
{
	QFont font = doc->defaultFont();
	doc->clear();
	QTextCursor cursor(doc);
	QTextCharFormat charFormat = cursor.charFormat();
	charFormat.setFont(font);
	cursor.setCharFormat(charFormat);
	appendText(doc, cursor, text);
}

/**
 * Appends a new paragraph with text to the end of the document.
 * \param text text to append to the QTextDocument. Please note that if you
 *             insert any <icon>s, attributes' values MUST be Qt::escaped.
 */
void PsiRichText::appendText(QTextDocument *doc, QTextCursor &cursor, const QString &text)
{
	cursor.beginEditBlock();
	cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
	cursor.clearSelection();
	if (!cursor.atBlockStart()) {
		cursor.insertBlock();
		
		// clear trackbar for new blocks
		QTextBlockFormat blockFormat = cursor.blockFormat();
		blockFormat.clearProperty(QTextFormat::BlockTrailingHorizontalRulerWidth);
		cursor.setBlockFormat(blockFormat);
	}
	
	appendTextHelper(doc, text, cursor);
	
	cursor.endEditBlock();
}

/**
 * Call this function on your QTextDocument to get plain text
 * representation, and all Icons will be replaced by their
 * initial text.
 */
QString PsiRichText::convertToPlainText(const QTextDocument *doc)
{
	QString obrepl = QString(QChar::ObjectReplacementCharacter);
	QQueue<QTextCharFormat> queue;
	QTextCursor nc = doc->find(obrepl, 0);
	QTextCursor cursor;

	while (!nc.isNull()) {
		queue.enqueue(nc.charFormat());

		cursor = nc;
		nc = doc->find(obrepl, cursor);
	}

	QString raw = doc->toPlainText();

	QStringList parts = raw.split(obrepl);

	QString result = parts.at(0);

	for (int i = 1; i < parts.size(); ++i) {
		if (!queue.isEmpty()) {
			QTextCharFormat format = queue.dequeue();
			if ((format).objectType() == TextIconFormat::IconFormatType) {
				result += format.stringProperty(TextIconFormat::IconText);
			}
		}
		result += parts.at(i);
	}
	return result;
}

/**
 * Returns non-null QTextCursor in case there's an ObjectReplacementCharacter
 * at the specified position in the document. Returns a null QTextCursor
 * otherwise.
 */
QTextCursor PsiRichText::textIconCursorAt(QTextDocument* doc, const QPoint& mousePos)
{
	int pos = doc->documentLayout()->hitTest(mousePos, Qt::ExactHit);
	QString obrepl = QString(QChar::ObjectReplacementCharacter);
	if (pos != -1) {
		QTextCursor nc = doc->find(obrepl, 0);
		QTextCursor cursor;

		while (!nc.isNull()) {
			if (pos == nc.position()-1) {
				return nc;
			}

			nc = doc->find(obrepl, nc);
		}
	}

	return QTextCursor();
}

static QString stringPropertyAt(QTextDocument* doc, const QPoint& mousePos, int property)
{
	QTextCursor cursor = PsiRichText::textIconCursorAt(doc, mousePos);
	if (!cursor.isNull()) {
		QTextCharFormat charFormat = cursor.charFormat();
		return charFormat.stringProperty(property);
	}

	return QString();
}

/**
 * Returns the icon's original text property that exists at given position
 * in \a doc.
 */
QString PsiRichText::iconTextAt(QTextDocument* doc, const QPoint& mousePos)
{
	return stringPropertyAt(doc, mousePos, TextIconFormat::IconText);
}

QString PsiRichText::iconNameAt(QTextDocument* doc, const QPoint& mousePos)
{
	return stringPropertyAt(doc, mousePos, TextIconFormat::IconName);
}

#ifndef WIDGET_PLUGIN
#include "psirichtext.moc"
#endif
