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

#include "tooltipwindow.h"
#include "notifications.h"
#include "notifications_p.h"

#include <QDesktopWidget>
#include <QDebug>
#include <QApplication>

#define FRAMECOUNT 20
#define TIMEINTERVAL 300

Notifications * Notifications::Private::instance = NULL;

// Notifications::Private

Notifications::Private::Private()
{
    timeline.setFrameRange(0, FRAMECOUNT);
    timeline.setDuration(TIMEINTERVAL);
    connect(&timeline, SIGNAL(frameChanged(int)),
            this, SLOT(timelineFrameChanged(int)));
}

void Notifications::Private::actionChosen(const QString & id)
{
    TooltipWindow * tooltip = static_cast < TooltipWindow * > (sender());
    int key = tooltips.key(tooltip, -1);
    if (key == -1) return;
    chosenActions[key] = id;

    hideNotification(tooltip);
}

void Notifications::Private::hideNotification(TooltipWindow * tooltip)
{
    if (!visibleTooltipsList.contains(tooltip)) {
        return;
    }

    visibleTooltipsList.removeAll(tooltip);

    if (visibleTooltipsList.count() == 0) {
        return;
    }

    timeline.stop();
    positions.clear();

    int top = 0;
    foreach (TooltipWindow * tooltip, visibleTooltipsList) {
        int currenttop = tooltip->geometry().top();
        int diff = top - currenttop;
        positions[tooltip] = QPair < int, int > (currenttop, diff);

        top = top + tooltip->geometry().height() + 4;
    }

    timeline.start();

}

void Notifications::Private::timelineFrameChanged(int frame)
{
    qreal factor = frame / (qreal) FRAMECOUNT;

    foreach (TooltipWindow * tooltip, visibleTooltipsList) {
        QRect g = tooltip->geometry();
        g.moveTop(
                positions[tooltip].first
                + positions[tooltip].second * factor);
        tooltip->setGeometry(g);
    }

}

void Notifications::Private::deleteNotification(int id)
{
    TooltipWindow * tooltip = tooltips.value(id, NULL);
    hideNotification(tooltip);
    delete tooltip;
    tooltips.remove(id);
    chosenActions.remove(id);
}


// Notifications

Notifications * Notifications::instance()
{
    if (Private::instance == NULL) {
        Private::instance = new Notifications();
    }
    return Private::instance;
}

int Notifications::showNotification(const QString & title, const QString & text,
            const QPixmap & icon, const QMap < QString, QString > & actions,
            int timeout)
{
    TooltipWindow * tooltip = new TooltipWindow();
    tooltip->setTitle(title);
    tooltip->setText(text);
    tooltip->setIcon(icon);
    tooltip->setActions(actions);
    tooltip->setTimeout(timeout);

    tooltip->setStyleSheet("\
            #TooltipBase { border: none; } \
            #Frame { background-color: white;\
            border-width: 4px; \
            border-image: url(:/tooltip/data/tooltip_background.png) 4 4 4 4 stretch stretch; } \
            QPushButtoin { border: none; } \
            ");

    connect(tooltip, SIGNAL(actionChosen(const QString &)),
            d, SLOT(actionChosen(const QString &)));

    int newid = 0;
    // finding the first available id
    while (d->tooltips.contains(newid)) {
        ++newid;
    }

    // positioning and showing the window
    QRect g = tooltip->geometry();

    int top = 0;
    if (d->visibleTooltipsList.count() > 0) {
        top = d->visibleTooltipsList.last()->geometry().bottom() + 4;
    }

    g.moveTop(top);
    g.moveRight(qApp->desktop()->width());
    tooltip->setGeometry(g);
    tooltip->show();

    d->tooltips[newid] = tooltip;
    d->visibleTooltipsList << tooltip;
    return newid;
}

QString Notifications::chosenAction(int id) const
{
    return d->chosenActions.value(id, QString());
}

void Notifications::deleteNotification(int id)
{
    d->deleteNotification(id);
}

Notifications::Notifications()
    : d(new Private())
{

}

Notifications::~Notifications()
{
    delete d;
}
