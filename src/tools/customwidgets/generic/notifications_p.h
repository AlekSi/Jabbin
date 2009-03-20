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

#ifndef NOTIFICATIONS_P_H_
#define NOTIFICATIONS_P_H_

#include "notifications.h"
#include "tooltipwindow.h"

#include <QObject>
#include <QMap>
#include <QTimeLine>

namespace CustomWidgets {

class Notifications::Private: public QObject {
    Q_OBJECT

public:
    Private();
    static Notifications * instance;

    QMap < int, TooltipWindow * > tooltips;
    QList < TooltipWindow * > visibleTooltipsList;

    QMap < TooltipWindow *, QPair < int, int > > positions;
    QTimeLine timeline;

    void hideNotification(TooltipWindow * tooltip);
    void deleteNotification(int id);

public Q_SLOTS:
    void actionChosen(const QString & id);
    void timelineFrameChanged(int frame);
};

} // namespace CustomWidgets

#endif // NOTIFICATIONS_P_H_

