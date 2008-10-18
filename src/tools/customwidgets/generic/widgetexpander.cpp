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

#include "widgetexpander.h"
#include <QWidget>
#include <QTimeLine>
#include <QDebug>

#define ANIMATION_DURATION 500

class WidgetExpander::Private {
public:
    Private()
        : timeline(NULL), fullwidth(100), minwidth(50)
    {
    }

    QTimeLine * timeline;
    int fullwidth, minwidth;
    QWidget * child;
};

WidgetExpander::WidgetExpander(QWidget * child, QWidget * parent)
    : QWidget(), d(new Private())
{
    d->timeline = new QTimeLine();
    d->timeline->setCurveShape(QTimeLine::EaseInOutCurve);
    d->timeline->setDuration(ANIMATION_DURATION);

    connect(d->timeline, SIGNAL(frameChanged(int)),
        this, SLOT(setChildWidth(int)));

    setChildWidget(child);
    setParent(parent);
}

void WidgetExpander::setChildWidget(QWidget * child)
{
    qDebug() << "setChildWidget";
    if (!child || d->child == child) {
        return;
    }

    d->child = child;

    child->installEventFilter(this);
    child->setParent(this);
    child->move(0, 0);
    child->resize(d->minwidth, height());
}

void WidgetExpander::expand()
{
    d->timeline->setStartFrame(d->minwidth);
    d->timeline->setEndFrame(d->fullwidth);
    d->timeline->start();
}

void WidgetExpander::contract()
{
    d->timeline->setStartFrame(d->fullwidth);
    d->timeline->setEndFrame(d->minwidth);
    d->timeline->start();
}

WidgetExpander::~WidgetExpander()
{
    delete d->timeline;
    delete d;
}

void WidgetExpander::setChildWidth(int i)
{
    if (d->child) {
        d->child->resize(i, height());
    }
}

void WidgetExpander::resizeEvent(QResizeEvent *)
{
    d->minwidth = width() / 2;
    d->fullwidth = width();

    if (d->child) {
        qDebug() << d->minwidth, height();
        d->child->resize(d->minwidth, height());
    }
}

bool WidgetExpander::eventFilter(QObject * object, QEvent * event)
{
    if (object == d->child && event->type() == QEvent::FocusIn) {
        expand();
    } else if (object == d->child && event->type() == QEvent::FocusOut) {
        contract();
    } else if (object == d->child && event->type() == QEvent::Resize) {
        resize(width(), d->child->height());
    }
    return QObject::eventFilter(object, event);
}
