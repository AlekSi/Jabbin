/*
 *   Copyright (C) 2009 Ivan Cukic <ivan.cukic+kde@gmail.com>
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

#include "updater.h"
#include "updater_p.h"
#include "jabbinpsi_revision.h"
#include "jabbinnotifications.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QDebug>
#include <QDesktopServices>

#define UPDATE_SERVER "http://www.jabbin.com/jabbinupdate/"

void UpdaterThread::run()
{
}

Updater * Updater::m_instance = NULL;

Updater * Updater::instance()
{
    if (!m_instance) {
        m_instance = new Updater();
    }
    return m_instance;
}

Updater::Updater()
    : d(new Private())
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
    QNetworkReply * reply = manager->get(QNetworkRequest(QUrl(QString() + UPDATE_SERVER + "check.php")));
    connect(JabbinNotifications::instance(), SIGNAL(notificationFinished(int, const QString &)),
            this, SLOT(notificationFinished(int, const QString &)));
}

void Updater::replyFinished(QNetworkReply * reply)
{
    QByteArray data = reply->readAll();
    qDebug() << "Updater::replyFinished: checking for the new version";
    if (data > JABBINPSI_FULL_VERSION) {
        qDebug() << "Updater::replyFinished: There is a new version available!" << data;
        d->notificationId = JabbinNotifications::instance()->createNotification(
            N_UPDATE_AVAILABLE, data);
    }
}

void Updater::notificationFinished(int id, const QString & action)
{
    if (d->notificationId != id) {
        return;
    }

    if (action == "download") {
        QDesktopServices::openUrl(QUrl(UPDATE_SERVER));
        qDebug() << "Updater::notificationFinished: downloading...";
    }
}


Updater::~Updater()
{
    delete d;
}

