/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "advancedlineedit.h"
#include <QPushButton>
#include <QStyle>
#include <QDebug>
#include <QFile>
#include <QPainter>
#include <QColor>

class AdvancedLineEdit::Private {
public:
    Private(AdvancedLineEdit * parent)
        : q(parent),
          beginButton(NULL),
          endButton(NULL),
          emptyText()
    {
        flags[BeginButton] = Hide;
        flags[EndButton] = Hide;
    }

    QPushButton * beginButton;
    QPushButton * endButton;
    QString emptyText;
    QMap < AdvancedLineEdit::Button, AdvancedLineEdit::ButtonFlag > flags;
    QString mainstyle;
    AdvancedLineEdit * q;

    QString readFile(QString fileName)
    {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "File not found " << fileName;
            return QString();
        }

        QString result;

        QTextStream in(&file);
        QString line = in.readLine();
        while (!line.isNull()) {
            result += line;
            line = in.readLine();
        }

        return result;
    }

    void invalidateButtons()
    {
        qDebug() << "invalidating";

        int frameWidth = q->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
        int beginWidth =
            (
                flags[BeginButton] == ShowAlways ||
                ( flags[BeginButton] != Hide && q->hasFocus() )
            ) ? (beginButton->sizeHint().width()) : (0);
        int endWidth =
            (
                flags[EndButton] == ShowAlways ||
                ( flags[EndButton] != Hide && q->hasFocus() )
            ) ? (endButton->sizeHint().width()) : (0);

        qDebug() << "invalidating";

        beginButton->setVisible(beginWidth != 0);
        endButton->setVisible(endWidth != 0);

        q->setStyleSheet(mainstyle
                .arg(endWidth + 1 + frameWidth)
                .arg(beginWidth + 1 + frameWidth)
            );
    }
};

AdvancedLineEdit::AdvancedLineEdit(QWidget *parent)
    : QLineEdit(parent), d(new Private(this))
{
    d->beginButton = new QPushButton(this);
    d->endButton = new QPushButton(this);

    connect(d->beginButton, SIGNAL(clicked()),
        this, SIGNAL(beginButtonClicked()));
    connect(d->endButton, SIGNAL(pressed()),
        this, SIGNAL(endButtonClicked()));

    d->beginButton->setIcon(QIcon(":/customwidgets/generic/data/advancedlineedit_beginicon.png"));
    d->beginButton->setIconSize(QSize(16, 16));
    d->beginButton->setCursor(Qt::ArrowCursor);
    d->beginButton->setStyleSheet(
        d->readFile(":/customwidgets/generic/data/advancedlineedit_beginbutton_style.css"));

    d->endButton->setIcon(QIcon(":/customwidgets/generic/data/advancedlineedit_endicon.png"));
    d->endButton->setIconSize(QSize(16, 16));
    d->endButton->setCursor(Qt::ArrowCursor);
    d->endButton->setStyleSheet(
        d->readFile(":/customwidgets/generic/data/advancedlineedit_endbutton_style.css"));

    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);

    QSize msz = minimumSizeHint();

    setMinimumSize(qMax(msz.width(), d->beginButton->sizeHint().height() + frameWidth * 2 + 2),
                   qMax(msz.height(), d->beginButton->sizeHint().height() + frameWidth * 2 + 2));

    setMinimumSize(qMax(msz.width(), d->endButton->sizeHint().height() + frameWidth * 2 + 2),
                   qMax(msz.height(), d->endButton->sizeHint().height() + frameWidth * 2 + 2));

    d->mainstyle =
        d->readFile(":/customwidgets/generic/data/advancedlineedit_style.css");

    d->invalidateButtons();
}

AdvancedLineEdit::~AdvancedLineEdit()
{
    delete d->beginButton;
    delete d->endButton;
    delete d;
}

void AdvancedLineEdit::resizeEvent(QResizeEvent * event)
{
    QSize sizeEndButton = d->beginButton->sizeHint();
    QSize sizeBeginButton = d->beginButton->sizeHint();

    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    int frameHeight = style()->pixelMetric(QStyle::PM_LayoutTopMargin);

    d->beginButton->move(6,
        (rect().bottom() + 1 - sizeBeginButton.height()) / 2);
    d->endButton->move(rect().right() - 6 - sizeEndButton.width(),
        (rect().bottom() + 1 - sizeEndButton.height()) / 2);

    QLineEdit::resizeEvent(event);
}

void AdvancedLineEdit::paintEvent(QPaintEvent * event)
{
    QLineEdit::paintEvent(event);

    if (text() == QString() && !hasFocus()) {
        QPainter p(this);
        QColor c = p.pen().color();
        c.setAlpha(c.alpha() / 2);
        p.setPen(QPen(c));

        QRect rect(QPoint(), size());
        rect.setLeft(d->beginButton->x() + 6 +
                (d->beginButton->isVisible() ? d->beginButton->width() : 0));
        rect.setTop(d->beginButton->y());
        rect.setHeight(d->beginButton->height());

        // If the end button is not visible, then we add it's width
        // to the rectangles right coordinate
        rect.setRight(d->endButton->x() - 6 +
                (d->endButton->isVisible() ? 0 : d->beginButton->width()));
        p.drawText(rect, Qt::AlignVCenter | Qt::AlignLeft, d->emptyText);
    }
}

void AdvancedLineEdit::setEmptyText(const QString & text)
{
    d->emptyText = text;
}

QString AdvancedLineEdit::emptyText() const
{
    return d->emptyText;
}

void AdvancedLineEdit::setButtonFlag(AdvancedLineEdit::Button button,
        AdvancedLineEdit::ButtonFlag flag)
{
    d->flags[button] = flag;
    d->invalidateButtons();
}

AdvancedLineEdit::ButtonFlag AdvancedLineEdit::buttonFlag(AdvancedLineEdit::Button button) const
{
    return d->flags[button];
}

void AdvancedLineEdit::setButtonIcon(AdvancedLineEdit::Button button,
        const QIcon & icon)
{
    switch (button) {
        case BeginButton:
            d->beginButton->setIcon(icon);
            break;
        case EndButton:
            d->endButton->setIcon(icon);
            break;
    }
}

QIcon AdvancedLineEdit::buttonIcon(AdvancedLineEdit::Button button) const
{
    switch (button) {
        case BeginButton:
            return d->beginButton->icon();
        case EndButton:
            return d->endButton->icon();
    }
    return QIcon();
}

void AdvancedLineEdit::focusInEvent(QFocusEvent * event)
{
    d->invalidateButtons();
    QLineEdit::focusInEvent(event);
}

void AdvancedLineEdit::focusOutEvent(QFocusEvent * event)
{
    d->invalidateButtons();
    QLineEdit::focusOutEvent(event);
}
