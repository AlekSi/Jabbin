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

#include "jabbinnotifications.h"
#include "generic/notifications.h"
#include "psioptions.h"
#include "phononsoundplayer.h"
#include <QMap>

class NotificationItem {
public:
    NotificationItem()
        : tooltipId(0), player(NULL)
    {

    }

    int tooltipId;
    SoundPlayer * player;
};

class JabbinNotifications::Private
{
public:
    Private()
    {

    }

    QMap  < int, NotificationItem > notifications;

    static JabbinNotifications * instance;
};

JabbinNotifications *
 JabbinNotifications::Private::instance = NULL;

JabbinNotifications::JabbinNotifications()
    : d(new Private())
{
    connect(
         CustomWidgets::Notifications::instance(), SIGNAL(notificationFinished(int, const QString &)),
         this, SIGNAL(notificationFinished(int, const QString &)));

}

JabbinNotifications::~JabbinNotifications()
{
}

JabbinNotifications * JabbinNotifications::instance()
{
    if (!Private::instance) {
        Private::instance = new JabbinNotifications();
    }
    return Private::instance;
}

int JabbinNotifications::createNotification(
        const QString & type, const QString & data,
        Qt::ConnectionType ctype)
{
    int id = 0;

#define getOption(A, B) PsiOptions::instance()->getOption(A).to##B ()
    if (getOption(type + ".tooltip", Bool)) {
        QString message;
        QMap < QString, QString > actions;
        int timeout = 5;
        if (type == N_INCOMING_CALL) {
            message = tr("Incoming call from %1").arg(data);
            actions["accept"] = tr("Accept");
            actions["reject"] = tr("Reject");
            timeout = 0;
        } else if (type == N_CHAT_REQUEST) {
            message = tr("%1 wants to chat with you").arg(data);
        } else if (type == N_STATUS_REQUEST) {
            message = tr("%1 wants to see your status").arg(data);
        } else if (type == N_CONTACT_ONLINE) {
            message = tr("%1 is now online").arg(data);
        } else if (type == N_INCOMING_CALL) {
            message = tr("%1 sent you a file").arg(data);
        } else if (type == N_INCOMING_VOICEMAIL) {
            message = tr("Incoming voicemail from %1").arg(data);
        }

        id = CustomWidgets::Notifications::instance()->showNotification(
            tr("Notification from Jabbin"), message,
            QPixmap(), actions,
            timeout);
        d->notifications[id].tooltipId = id;
    }

    QString value = getOption(type + ".sound", String);
    if (!value.isEmpty()) {
        if (!id) {
            while (d->notifications.contains(--id))
                ;
        }

        d->notifications[id].player = new SoundPlayer(NULL);
        d->notifications[id].player->playContinuosSound(value,
            (type == N_INCOMING_CALL) ? 60000 : 10);
    }

    if (getOption(type + ".popupjabbin", Bool)) {

    }

#undef getOption

    return id;
}

void JabbinNotifications::endNotification(int id)
{
    if (d->notifications.contains(id)) {
        if (d->notifications[id].tooltipId > 0) {
            CustomWidgets::Notifications::instance()->deleteNotification(
                  d->notifications[id].tooltipId);
        }
        if (d->notifications[id].player) {
            d->notifications[id].player->stop();
            d->notifications[id].player->deleteLater();
        }
    }
}
