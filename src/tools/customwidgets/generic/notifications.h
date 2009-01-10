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

#ifndef NOTIFICATIONS_H_
#define NOTIFICATIONS_H_

#include <QObject>
#include <QPixmap>

namespace CustomWidgets {

/**
 * Notifications system
 */
class Notifications: public QObject {
    Q_OBJECT

public:
    ~Notifications();

    /**
     * @returns instance of Notifications
     */
    static Notifications * instance();

    /**
     * Creates and shows a new notification
     * @returns id number of the notification
     * @param title notification title
     * @param text notification text
     * @param icon notification icon
     * @param actions list of actions
     * @param timeout timeout in seconds
     */
    int showNotification(const QString & title, const QString & text,
            const QPixmap & icon, const QMap < QString, QString > & actions,
            int timeout = 0);

    /**
     * @returns the chosen action for the specified notification
     * @param id notification id
     */
    QString chosenAction(int id) const;

    /**
     * Deletes the specified notification
     */
    void deleteNotification(int id);

Q_SIGNALS:
    /**
     * Emitted when the notification is closed either by choosing an
     * action or because of the timeout
     */
    void notificationExpired(int id);

private:
    Notifications();

    class Private;
    Private * const d;
};

} // namespace CustomWidgets

#endif // NOTIFICATIONS_H_

