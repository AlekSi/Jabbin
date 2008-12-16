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

#include "styledwindow.h"
#include <QDebug>
#include <QLayout>

#include <QResizeEvent>
#include <QRect>
#include <QPoint>
#include <QToolButton>

#include <QBitmap>
#include <QPixmap>
#include <QPainter>

#define DEFAULT_BORDER_SIZE 2
#define HEADER_SIZE 20

// StyledWindow::Private
class StyledWindow::Private {
public:
    Private(StyledWindow * parent)
        : q(parent), borderLayout(NULL), borderSize(DEFAULT_BORDER_SIZE), customDecorations(false)
    {

    }

    ~Private()
    {
        delete borderLayout;
    }

    void ensureBorderWidgets()
    {
        if (borderLayout) return;

        borderLayout = new QGridLayout();

        borderLayout->addWidget(borderWidgets[Top | Left]  = new QWidget(q), 0, 0);
        borderLayout->addWidget(borderWidgets[Top]         = new QWidget(q), 0, 1);
        borderLayout->addWidget(borderWidgets[Top | Right] = new QWidget(q), 0, 2);

        borderLayout->addWidget(borderWidgets[Left]  = new QWidget(q), 1, 0, 2, 1);
        borderLayout->addWidget(borderWidgets[Right] = new QWidget(q), 1, 2, 2, 1);

        borderLayout->addWidget(borderWidgets[Bottom | Left]  = new QWidget(q), 3, 0);
        borderLayout->addWidget(borderWidgets[Bottom]         = new QWidget(q), 3, 1);
        borderLayout->addWidget(borderWidgets[Bottom | Right] = new QWidget(q), 3, 2);

        borderLayout->setRowStretch(2, 1);
        borderLayout->setContentsMargins(0, 0, 0, 0);

        // Size policies
        QSizePolicy policy;

        policy = QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        borderWidgets[Top]->setSizePolicy(policy);
        borderWidgets[Bottom]->setSizePolicy(policy);

        policy = QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
        borderWidgets[Left]->setSizePolicy(policy);
        borderWidgets[Right]->setSizePolicy(policy);

        policy = QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        borderWidgets[Top | Left]->setSizePolicy(policy);
        borderWidgets[Top | Right]->setSizePolicy(policy);
        borderWidgets[Bottom | Left]->setSizePolicy(policy);
        borderWidgets[Bottom | Right]->setSizePolicy(policy);

        // Mouse cursor
        borderWidgets[Top]->setCursor(Qt::SizeVerCursor);
        borderWidgets[Bottom]->setCursor(Qt::SizeVerCursor);

        borderWidgets[Left]->setCursor(Qt::SizeHorCursor);
        borderWidgets[Right]->setCursor(Qt::SizeHorCursor);

        borderWidgets[Top | Left]->setCursor(Qt::SizeFDiagCursor);
        borderWidgets[Bottom | Right]->setCursor(Qt::SizeFDiagCursor);

        borderWidgets[Bottom | Left]->setCursor(Qt::SizeBDiagCursor);
        borderWidgets[Top | Right]->setCursor(Qt::SizeBDiagCursor);

        foreach (QWidget * widget, borderWidgets) {
            widget->installEventFilter(q);
            widget->setObjectName("Border");
        }

        titlebarLayout = new QBoxLayout(QBoxLayout::LeftToRight);

        addTitlebarItem(Icon, q->windowIcon());
        addTitlebarItem(Minimize, QIcon(":/customwidgets/generic/data/window_button_minimize.png"), tr("Minimize"));
        addTitlebarItem(Maximize, QIcon(":/customwidgets/generic/data/window_button_maximize.png"), tr("Maximize"));
        addTitlebarItem(Close,    QIcon(":/customwidgets/generic/data/window_button_close.png"), tr("Close"));

        addTitlebarItem(Title, QIcon());
        titlebarItems[Title]->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
        titlebarItems[Title]->setText(q->windowTitle());
        titlebarItems[Title]->setObjectName("TitlebarTitle");

        titlebarItems[Title]->setStyleSheet(
            "QToolButton {         \
                border: none;      \
                text-align: left;  \
                alignment: left;   \
                font-weight: bold; \
            }");

        borderLayout->addLayout(titlebarLayout, 1, 1);
    }

    void addTitlebarItem(qreal coordinate, const QIcon & icon, const QString & tooltip = QString())
    {
        if (titlebarItems.contains(coordinate)) return;

        QToolButton * button = new QToolButton(q);
        button->setIcon(icon);
        button->setToolTip(tooltip);
        button->setStyleSheet("QToolButton { border: none; }");
        button->installEventFilter(q);
        button->setObjectName("Titlebar");

        titlebarItems[coordinate] = button;

        QMapIterator < qreal, QToolButton * > iterator(titlebarItems);
        int i = 0;

        while (iterator.hasNext()) {
            iterator.next();
            if (coordinate < iterator.key()) {
                titlebarLayout->insertWidget(i - 1, button);
                return;
            }
            ++i;
        }

        titlebarLayout->addWidget(button);
    }

    void changeBorder(int delta, int hdr = 0)
    {
        if (!q->layout()) return;

        int left, top, right, bottom;

        q->layout()->getContentsMargins(&left, &top, &right, &bottom);

        left   += delta;
        top    += delta;
        right  += delta;
        bottom += delta;

        if (customDecorations) {
            ensureBorderWidgets();

            // Sizes
            borderWidgets[Top]->setMinimumHeight(borderSize);
            borderWidgets[Bottom]->setMinimumHeight(borderSize);
            borderWidgets[Left]->setMinimumWidth(borderSize);
            borderWidgets[Right]->setMinimumWidth(borderSize);

            borderWidgets[Top | Left]->setMinimumSize(borderSize, borderSize);
            borderWidgets[Top | Right]->setMinimumSize(borderSize, borderSize);
            borderWidgets[Bottom | Left]->setMinimumSize(borderSize, borderSize);
            borderWidgets[Bottom | Right]->setMinimumSize(borderSize, borderSize);

        }

        top += hdr * HEADER_SIZE;

        q->layout()->setContentsMargins(left, top, right, bottom);
    }

    enum Border {
        Left = 1,
        Top = 2,
        Right = 4,
        Bottom = 8
    };

    StyledWindow * q;
    QGridLayout * borderLayout;
    QBoxLayout * titlebarLayout;
    QMap < short, QWidget * > borderWidgets;
    QMap < qreal, QToolButton * > titlebarItems;
    QPoint originalMouseCoordinate;
    QRect originalWindowGeometry;
    short resizeFlags;
    int borderSize;
    bool customDecorations : 1;
};

// AdvancedTabWidget
StyledWindow::StyledWindow(QWidget *parent)
    : QWidget(parent), d(new Private(this))
{
}

StyledWindow::~StyledWindow()
{
    delete d;
}

void StyledWindow::setUseCustomDecorations(bool value)
{
    if (value == d->customDecorations) return;

    d->customDecorations = value;

    Qt::WindowFlags flags = windowFlags();
    if (value) {
        flags |= Qt::FramelessWindowHint;
        d->changeBorder(d->borderSize, 1);
    } else {
        flags &= ~Qt::FramelessWindowHint;
        d->changeBorder(- d->borderSize, -1);
    }
    setWindowFlags(flags);
}

void StyledWindow::setBorderSize(int size)
{
    if (d->borderSize == size) return;

    if (d->customDecorations && layout()) {
        d->changeBorder(size - d->borderSize);
    }

    d->borderSize = size;
}

void StyledWindow::resizeEvent(QResizeEvent * event)
{
    QWidget::resizeEvent(event);

    if (d->borderLayout) {
        d->borderLayout->setGeometry(QRect(QPoint(), event->size()));

        QRegion region;
        QRect rect(QPoint(), size());

        region += rect.adjusted(d->borderSize * 2, 0, - d->borderSize * 2, 0);
        region += rect.adjusted(0, d->borderSize * 2, 0, 0);// - d->borderSize * 2);
        QRect corner(rect.topLeft(), QSize(d->borderSize * 4, d->borderSize * 4));
        region += QRegion(corner, QRegion::Ellipse);
        corner.moveTopRight(rect.topRight());
        region += QRegion(corner, QRegion::Ellipse);
        setMask(region);
    }
}

int StyledWindow::borderSize() const
{
    return d->borderSize;
}

void StyledWindow::showTitlebarItem(TitlebarItem item, bool show)
{
    showTitlebarItem((qreal)item, show);
}

bool StyledWindow::isTitlebarItemVisible(TitlebarItem item) const
{
    return isTitlebarItemVisible((qreal)item);
}

void StyledWindow::addTitlebarItem(qreal coordinate, const QIcon & icon, const QString & tooltip)
{
    d->addTitlebarItem(coordinate, icon, tooltip);
}

void StyledWindow::showTitlebarItem(qreal coordinate, bool show)
{
    if (!d->titlebarItems.contains(coordinate)) return;
    d->titlebarItems[coordinate]->setVisible(show);
}

bool StyledWindow::isTitlebarItemVisible(qreal coordinate) const
{
    if (!d->titlebarItems.contains(coordinate)) return false;
    return d->titlebarItems[coordinate]->isVisible();
}

QToolButton * StyledWindow::titlebarItem(TitlebarItem item) const
{
    return titlebarItem((qreal)item);
}

QToolButton * StyledWindow::titlebarItem(qreal coordinate) const
{
    if (d->titlebarItems.contains(coordinate)) {
        return d->titlebarItems[coordinate];
    }
    return NULL;
}

bool StyledWindow::eventFilter(QObject * object, QEvent * event)
{
    if (d->customDecorations) {
        // Resizing related events
        if (d->borderWidgets.key((QWidget *)object) != 0) {
            if (event->type() == QEvent::MouseButtonPress) {
                d->originalMouseCoordinate = QCursor::pos (); //mouseEvent->pos();
                d->originalWindowGeometry = geometry();
                d->resizeFlags = d->borderWidgets.key((QWidget *)object);
            } else if (event->type() == QEvent::MouseButtonRelease) {
                d->resizeFlags = 0;
            } else if (event->type() == QEvent::MouseMove && d->resizeFlags) {
                QRect geometry = d->originalWindowGeometry;
                QPoint delta = QCursor::pos() - d->originalMouseCoordinate;
                if (Private::Left & d->resizeFlags) {
                    geometry.setLeft(geometry.left() + delta.x());
                }
                if (Private::Top & d->resizeFlags) {
                    geometry.setTop(geometry.top() + delta.y());
                }
                if (Private::Right & d->resizeFlags) {
                    geometry.setRight(geometry.right() + delta.x());
                }
                if (Private::Bottom & d->resizeFlags) {
                    geometry.setBottom(geometry.bottom() + delta.y());
                }
                setGeometry(geometry);
            }
        }

        // Titlebar related events
        qreal coordinate;
        if (object == d->titlebarItems[Title]) {
            if (event->type() == QEvent::MouseButtonPress) {
                d->originalMouseCoordinate = QCursor::pos ();
                d->originalWindowGeometry = geometry();
                d->resizeFlags = -1;
                d->titlebarItems[Title]->setCursor(Qt::SizeAllCursor);
            } else if (event->type() == QEvent::MouseButtonRelease) {
                d->resizeFlags = 0;
                d->titlebarItems[Title]->setCursor(Qt::ArrowCursor);
            } else if (event->type() == QEvent::MouseMove && d->resizeFlags) {
                move(d->originalWindowGeometry.topLeft() + QCursor::pos()
                        - d->originalMouseCoordinate);
            }
        } else if ((coordinate = d->titlebarItems.key((QToolButton *)object, Title)) != Title) {
            // Matching all other titlebar elements (except Title)
            if (event->type() == QEvent::MouseButtonRelease) {
                qDebug() << "click!";
                if (coordinate == Minimize) {
                    setWindowState(windowState() | Qt::WindowMinimized);
                } else if (coordinate == Maximize) {
                    setWindowState(windowState() ^ Qt::WindowMaximized);
                } else if (coordinate == Close) {
                    close();
                }
            }
        }
    }
    return QWidget::eventFilter(object, event);
}
