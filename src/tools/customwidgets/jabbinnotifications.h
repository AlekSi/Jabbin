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

#ifndef JABBINNOTIFICATIONS_H_
#define JABBINNOTIFICATIONS_H_

#include <QObject>
#include <QPixmap>

#define N_INCOMING_CALL      "notification.incoming_call"
#define N_CHAT_REQUEST       "notification.chat_request"
#define N_STATUS_REQUEST     "notification.status_request"
#define N_CONTACT_ONLINE     "notification.contact_online"
#define N_INCOMING_FILE      "notification.incomming_file"
#define N_INCOMING_VOICEMAIL "notification.incomming_voicemail"

/**
 * Jabbin Notifications system
 */
class JabbinNotifications: public QObject {
    Q_OBJECT

public:
    ~JabbinNotifications();

    /**
     * @returns instance of Notifications
     */
    static JabbinNotifications * instance();

    /**
     * Creates a notification of the specified type.
     * @param type type
     * @param data extra data needed for the notification
     * @param ctype    @see QObject::connect()
     * @returns id of the notification, 0 iff not sucess
     */
    int createNotification(
        const QString & type,
        const QString & data,
        Qt::ConnectionType ctype = Qt::AutoConnection);

    /**
     * Closes the specified notification
     * @param id id
     */
    void endNotification(int id);

Q_SIGNALS:
    void notificationFinished(int id, const QString & action);

private:
    JabbinNotifications();

    class Private;
    Private * const d;
};

#endif // JABBINNOTIFICATIONS_H_

