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

#include "advancedtabwidget.h"
#include "advancedtabbar.h"
#include <QDebug>

namespace CustomWidgets {

// AdvancedTabWidget::Private
class AdvancedTabWidget::Private {
public:
    Private(AdvancedTabWidget * parent)
    {
        tabBar = new AdvancedTabBar(parent);
        parent->setTabBar(tabBar);
    }

    AdvancedTabBar * tabBar;
};

// AdvancedTabWidget
AdvancedTabWidget::AdvancedTabWidget(QWidget *parent)
    : QTabWidget(parent), d(new Private(this))
{
}

AdvancedTabWidget::~AdvancedTabWidget()
{
    delete d;
}

void AdvancedTabWidget::resizeEvent(QResizeEvent * event)
{
    QTabWidget::resizeEvent(event);
    tabBar()->resize(size());
}

void AdvancedTabWidget::setViewType(AdvancedTabBar::ViewType viewType)
{
    d->tabBar->setViewType(viewType);
}

AdvancedTabBar::ViewType AdvancedTabWidget::viewType() const
{
    return d->tabBar->viewType();
}

void AdvancedTabWidget::setTabBarThickness(int value)
{
    d->tabBar->setThickness(value);
}

} // namespace CustomWidgets
