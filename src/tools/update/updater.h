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

#ifndef C_UPDATER_H
#define C_UPDATER_H

#include <QObject>
#include <QNetworkReply>

class Updater: public QObject {
    Q_OBJECT
public:
    ~Updater();

    static Updater * instance();

public Q_SLOTS:
    void replyFinished(QNetworkReply * reply);
    void notificationFinished(int id, const QString & action);

private:
    class Private;
    Private * const d;

    Updater();

    static Updater * m_instance;
};

#endif // C_UPDATER_H

