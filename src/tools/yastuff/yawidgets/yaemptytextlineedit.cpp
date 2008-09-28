/*
 * yaemptytextlineedit.cpp
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

#include "yaemptytextlineedit.h"

#include <QPainter>
#include <QVariant>
#include <QCoreApplication>
#include <QKeyEvent>

#include "yaeditorcontextmenu.h"
#include "yaokbutton.h"
#include "yaclosebutton.h"

YaEmptyTextLineEdit::YaEmptyTextLineEdit(QWidget* parent)
	: QLineEdit(parent)
	, okButton_(0)
	, cancelButton_(0)
{
	okButton_ = new YaOkButton(this);
	connect(okButton_, SIGNAL(clicked()), SLOT(okButtonClicked()));
	okButton_->setCursor(Qt::ArrowCursor);
	okButton_->show();

	cancelButton_ = new YaCloseButton(this);
	connect(cancelButton_, SIGNAL(clicked()), SLOT(cancelButtonClicked()));
	cancelButton_->setCursor(Qt::ArrowCursor);
	cancelButton_->hide();
}

YaEmptyTextLineEdit::~YaEmptyTextLineEdit()
{
}

QString YaEmptyTextLineEdit::emptyText() const
{
	return emptyText_;
}

void YaEmptyTextLineEdit::setEmptyText(const QString& emptyText)
{
	emptyText_ = emptyText;
	update();
}

void YaEmptyTextLineEdit::paintEvent(QPaintEvent* e)
{
	QLineEdit::paintEvent(e);
	if (text().isEmpty() && !emptyText().isEmpty()) {
		QPainter p(this);
		p.setPen(palette().color(QPalette::Disabled, QPalette::Text));
		p.drawText(rect().adjusted(10, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, emptyText());
	}
}

void YaEmptyTextLineEdit::contextMenuEvent(QContextMenuEvent* e)
{
	YaEditorContextMenu menu(this);
	menu.exec(e, this);
}

void YaEmptyTextLineEdit::resizeEvent(QResizeEvent* e)
{
	QLineEdit::resizeEvent(e);
	updateOkButton();
}

void YaEmptyTextLineEdit::showEvent(QShowEvent* e)
{
	QLineEdit::showEvent(e);
	updateOkButton();
}

void YaEmptyTextLineEdit::okButtonClicked()
{
	QKeyEvent ke(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
	QCoreApplication::instance()->sendEvent(this, &ke);
}

void YaEmptyTextLineEdit::cancelButtonClicked()
{
	QKeyEvent ke(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
	QCoreApplication::instance()->sendEvent(this, &ke);
}

void YaEmptyTextLineEdit::updateOkButton()
{
	QRect r;
	int spacing = 5;
	r.setSize(okButton_->sizeHint());
	r.moveTop((height() - r.height()) / 2);
	r.moveLeft(width() - r.width() - spacing);

	int paddingRight = 0;
	if (okButton_->isVisible()) {
		okButton_->setGeometry(r);

		paddingRight = rect().right() - r.left();
	}
	else if (cancelButton_->isVisible()){
		cancelButton_->setGeometry(r);

		paddingRight = rect().right() - r.left();
	}

#ifdef YAPSI_NO_STYLESHEETS
	setProperty("margin-right", QVariant(paddingRight));
#else
	setStyleSheet(QString("padding-right: %1px;").arg(paddingRight));
#endif

	update();
}

bool YaEmptyTextLineEdit::okButtonVisible() const
{
	return okButton_->isVisible();
}

void YaEmptyTextLineEdit::setOkButtonVisible(bool visible)
{
	okButton_->setVisible(visible);
	updateOkButton();
}

bool YaEmptyTextLineEdit::cancelButtonVisible() const
{
	return cancelButton_->isVisible();
}

void YaEmptyTextLineEdit::setCancelButtonVisible(bool visible)
{
	cancelButton_->setVisible(visible);
	updateOkButton();
}
