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
#include "psievent.h"

#define N_INCOMING_CALL      "options.notification.incoming_call"
#define N_UPDATE_AVAILABLE   "options.notification.update_available"
#define N_CHAT_REQUEST       "options.notification.chat_request"
#define N_STATUS_REQUEST     "options.notification.status_request"
#define N_CONTACT_ONLINE     "options.notification.contact_online"
#define N_INCOMING_FILE      "options.notification.incomming_file"
#define N_INCOMING_VOICEMAIL "options.notification.incomming_voicemail"

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
     * Creates a notification of the specified type.
     * @param type type
     * @param data extra data needed for the notification
     * @param ctype    @see QObject::connect()
     * @returns id of the notification, 0 iff not sucess
     */
    int createNotification(
        const QString & type,
        const QStringList & data,
        Qt::ConnectionType ctype = Qt::AutoConnection);

    /**
     * Creates a notification of the specified type.
     * @param event psi event
     * @returns id of the notification, 0 iff not sucess
     */
    int createNotification(
        PsiEvent * event,
        Qt::ConnectionType ctype = Qt::AutoConnection
    );

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

