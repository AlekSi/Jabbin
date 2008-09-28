/*
 * psitextview.cpp - PsiIcon-aware QTextView subclass widget
 * Copyright (C) 2003-2006  Michail Pishchagin
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

#include "psitextview.h"

#include <QMenu>
#include <QScrollBar>
#include <QAbstractTextDocumentLayout>
#include <QTextDocumentFragment>

#include "urlobject.h"
#include "psirichtext.h"
#include "smoothscrollbar.h"

//----------------------------------------------------------------------------
// PsiTextView::Private
//----------------------------------------------------------------------------

//! \if _hide_doc_
class PsiTextView::Private : public QObject
{
	Q_OBJECT

public:
	Private(QObject *parent)
	: QObject(parent, "PsiTextView::Private") 
	{
		anchorOnMousePress = QString();
		hadSelectionOnMousePress = false;
	}

	QString anchorOnMousePress;
	bool hadSelectionOnMousePress;
		
	QString fragmentToPlainText(const QTextFragment &fragment);
	QString blockToPlainText(const QTextBlock &block);
	QString documentFragmentToPlainText(const QTextDocument &doc, QTextFrame::Iterator frameIt);
};
//!endif

//----------------------------------------------------------------------------
// PsiTextView
//----------------------------------------------------------------------------

/**
 * \class PsiTextView
 * \brief PsiIcon-aware QTextView-subclass widget
 */

/**
 * Default constructor.
 */
PsiTextView::PsiTextView(QWidget *parent)
: QTextEdit(parent)
{
	d = new Private(this);

	setReadOnly(true);
	PsiRichText::install(document());

	viewport()->setMouseTracking(true); // we want to get all mouseMoveEvents
}

/**
 * This function returns true if vertical scroll bar is 
 * at its maximum position.
 */
bool PsiTextView::atBottom()
{
	// '32' is 32 pixels margin, which was used in the old code
	return (verticalScrollBar()->maximum() - verticalScrollBar()->value()) <= 32;
}

/**
 * Scrolls the vertical scroll bar to its maximum position i.e. to the bottom.
 */
void PsiTextView::scrollToBottom()
{
	if (verticalScrollBar()->isSliderDown())
		return;

	SmoothScrollBar* smoothScrollBar = dynamic_cast<SmoothScrollBar*>(verticalScrollBar());
	if (smoothScrollBar)
		smoothScrollBar->setValueImmediately(verticalScrollBar()->maximum());
	else
		verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

/**
 * Scrolls the vertical scroll bar to its minimum position i.e. to the top.
 */
void PsiTextView::scrollToTop()
{
	if (verticalScrollBar()->isSliderDown())
		return;

	verticalScrollBar()->setValue(verticalScrollBar()->minimum());
}

/**
 * This function is provided for convenience. Please see 
 * PsiRichText::appendText() documentation for usage details.
 */
void PsiTextView::appendText(const QString &text)
{
	QTextCursor cursor = textCursor();
	Selection selection = saveSelection(cursor);
	
	PsiRichText::appendText(document(), cursor, text);
	
	restoreSelection(cursor, selection);
	setTextCursor(cursor);
}

/**
 * Saves current Selection in a structure, so it could be restored at later time.
 */
PsiTextView::Selection PsiTextView::saveSelection(QTextCursor &cursor)
{
	Selection selection;
	selection.start = selection.end = -1;

	if (cursor.hasSelection()) {
		selection.start = cursor.selectionStart();
		selection.end   = cursor.selectionEnd();
	}
	
	return selection;
}

/**
 * Restores a Selection that was previously saved by call to saveSelection().
 */
void PsiTextView::restoreSelection(QTextCursor &cursor, Selection selection)
{
	if (selection.start != -1 && selection.end != -1) {
		cursor.setPosition(selection.start, QTextCursor::MoveAnchor);
		cursor.setPosition(selection.end,   QTextCursor::KeepAnchor);
	}
}

/**
 * Returns HTML markup for selected text. If no text is selected, returns
 * HTML markup for all text.
 */
QString PsiTextView::getHtml() const
{
	PsiTextView *ptv = (PsiTextView *)this;
	QTextCursor cursor = ptv->textCursor();
	int position = ptv->verticalScrollBar()->value();
	
	bool unselectAll = false;
	if (!hasSelectedText()) {
		ptv->selectAll();
		unselectAll = true;
	}
	
	QMimeData *mime = createMimeDataFromSelection();
	QString result = mime->html();
	delete mime;
	
	// we need to restore original position if selectAll() 
	// was called, because setTextCursor() (which is necessary
	// to clear selection) will move vertical scroll bar
	if (unselectAll) {
		cursor.clearSelection();
		ptv->setTextCursor(cursor);
		ptv->verticalScrollBar()->setValue(position);
	}
	
	return result;
}

void PsiTextView::contextMenuEvent(QContextMenuEvent *e) 
{
	QMenu *menu;
	if (!anchorAt(e->pos()).isEmpty())
		menu = URLObject::getInstance()->createPopupMenu(anchorAt(e->pos()));
	else
		menu = createStandardContextMenu();
	menu->exec(e->globalPos());
	e->accept();
	delete menu;
}

// Copied (with modifications) from QTextBrowser
void PsiTextView::mouseMoveEvent(QMouseEvent *e)
{
	QTextEdit::mouseMoveEvent(e);

	QString anchor = anchorAt(e->pos());
	viewport()->setCursor(anchor.isEmpty() ? Qt::ArrowCursor : Qt::PointingHandCursor);
}

// Copied (with modifications) from QTextBrowser
void PsiTextView::mousePressEvent(QMouseEvent *e)
{
	d->anchorOnMousePress = anchorAt(e->pos());
	if (!textCursor().hasSelection() && !d->anchorOnMousePress.isEmpty()) {
		QTextCursor cursor = textCursor();
		QPoint mapped = QPoint(e->pos().x() + horizontalScrollBar()->value(), 
		                       e->pos().y() + verticalScrollBar()->value()); // from QTextEditPrivate::mapToContents
		const int cursorPos = document()->documentLayout()->hitTest(mapped, Qt::FuzzyHit);
		if (cursorPos != -1)
			cursor.setPosition(cursorPos);	   
		setTextCursor(cursor);
	}

	QTextEdit::mousePressEvent(e);

	d->hadSelectionOnMousePress = textCursor().hasSelection();
}

// Copied (with modifications) from QTextBrowser
void PsiTextView::mouseReleaseEvent(QMouseEvent *e)
{
	QTextEdit::mouseReleaseEvent(e);

	if (!(e->button() & Qt::LeftButton))
		return;

	const QString anchor = anchorAt(e->pos());

	if (anchor.isEmpty())
		return;

	if (!textCursor().hasSelection()
		|| (anchor == d->anchorOnMousePress && d->hadSelectionOnMousePress))
		URLObject::getInstance()->popupAction(anchor);
}

/**
 * This is overridden in order to properly convert Icons back
 * to their original text.
 */
QMimeData *PsiTextView::createMimeDataFromSelection() const
{
	QTextDocument *doc = new QTextDocument();
	QTextCursor cursor(doc);
	cursor.insertFragment(textCursor().selection());
	QString text = PsiRichText::convertToPlainText(doc);
	delete doc;
	
	QMimeData *data = new QMimeData;
	data->setText(text);
	data->setHtml(Qt::convertFromPlainText(text));
	return data;
}

/**
 * Ensures that if PsiTextView was scrolled to bottom when resize
 * operation happened, it will still be scrolled to bottom after the fact.
 */
void PsiTextView::resizeEvent(QResizeEvent *e)
{
	bool atEnd = verticalScrollBar()->value() ==
	             verticalScrollBar()->maximum();
	bool atStart = verticalScrollBar()->value() ==
	               verticalScrollBar()->minimum();
	double value = 0;
	if (!atEnd && !atStart)
		value = (double)verticalScrollBar()->maximum() /
		        (double)verticalScrollBar()->value();

	QTextEdit::resizeEvent(e);

	if (atEnd)
		verticalScrollBar()->setValue(verticalScrollBar()->maximum());
	else if (value != 0)
		verticalScrollBar()->setValue((int) ((double)verticalScrollBar()->maximum() / value));
}

#include "psitextview.moc"
