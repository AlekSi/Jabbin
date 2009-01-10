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

#include "advancedtabbar.h"
#include <QDebug>
#include <QTabBar>
#include <QPainter>
#include <QPaintEvent>
#include <QListWidget>

namespace CustomWidgets {

// AdvancedTabBar::Private
class AdvancedTabBar::Private {
public:
    Private(AdvancedTabBar * parent)
        : viewType(AdvancedTabBar::TabBar), list(NULL), thickness(100)
    {
        list = new QListWidget(parent);
        list->setObjectName("AdvancedTabBarList");
        list->hide();
        connect(list, SIGNAL(currentRowChanged(int)),
                parent, SLOT(setCurrentIndex(int)));
        connect(list, SIGNAL(currentRowChanged(int)),
                parent, SLOT(setCurrentIndex(int)));
        connect(parent, SIGNAL(currentChanged(int)),
                parent, SLOT(currentIndexChanged(int)));

    }

    AdvancedTabBar::ViewType viewType;
    QListWidget * list;
    int thickness;
};

// AdvancedTabBar
AdvancedTabBar::AdvancedTabBar(QWidget * parent)
    : QTabBar(parent), d(new Private(this))
{
}

AdvancedTabBar::~AdvancedTabBar()
{
    delete d;
}

QSize AdvancedTabBar::tabSizeHint(int index) const
{
    if (d->viewType != ListView) {
        return QTabBar::tabSizeHint(index);
    } else {
        QSize size = QSize(-1, -1);
        if (index >= 0 || index < d->list->count()) {
            size = d->list->item(index)->sizeHint();
        }
        if (size == QSize(-1, -1)) {
            size = QSize(d->thickness, d->thickness);
        }
        return size;
    }
}

void AdvancedTabBar::paintEvent(QPaintEvent * event) {
    if (d->list->isVisible()) {
        return;
    }

    QTabBar::paintEvent(event);
}

void AdvancedTabBar::resizeEvent(QResizeEvent * event)
{
    QTabBar::resizeEvent(event);
    switch (shape()) {
        case QTabBar::RoundedNorth:
        case QTabBar::TriangularNorth:
        case QTabBar::RoundedSouth:
        case QTabBar::TriangularSouth:
            setMinimumSize(parentWidget()->width(), 0);
            d->list->setViewMode(QListView::IconMode);
            break;
        case QTabBar::RoundedEast:
        case QTabBar::TriangularEast:
        case QTabBar::RoundedWest:
        case QTabBar::TriangularWest:
            setMinimumSize(0, parentWidget()->height());
            d->list->setViewMode(QListView::ListMode);
            break;
    }
    d->list->resize(size());
}

void AdvancedTabBar::setViewType(AdvancedTabBar::ViewType viewType)
{
    if (d->viewType == viewType) return;

    d->viewType = viewType;
    switch (viewType) {
        case None:
            hide();
            break;
        case TabBar:
            show();
            d->list->hide();
            break;
        case ListView:
            show();
            d->list->show();
            d->list->resize(size());
            break;
    }
}

AdvancedTabBar::ViewType AdvancedTabBar::viewType() const
{
    return d->viewType;
}

void AdvancedTabBar::tabLayoutChange()
{
    d->list->setIconSize(iconSize());

    while (count() < d->list->count()) {
        delete d->list->takeItem(0);
    }
    if (count() == -1) return;

    while (count() > d->list->count()) {
        d->list->addItem(QString());
    }

    for (int i = 0; i < count(); i++) {
        QListWidgetItem * item = d->list->item(i);
        item->setText(tabText(i));
        item->setIcon(tabIcon(i));
        item->setToolTip(tabToolTip(i));
        item->setWhatsThis(tabWhatsThis(i));
    }

    d->list->setCurrentRow(currentIndex());
}

void AdvancedTabBar::currentIndexChanged(int index)
{
    d->list->setCurrentRow(index);
}

void AdvancedTabBar::setThickness(int value)
{
    d->thickness = value;
    update();
}

} // namespace CustomWidgets
