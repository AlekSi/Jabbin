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

#include "joimnotifications.h"
#include "generic/notifications.h"

class JoimNotifications::Private
{
public:
    Private()
    {

    }

    static JoimNotifications * instance;
};

JoimNotifications *
 JoimNotifications::Private::instance = NULL;

JoimNotifications::JoimNotifications()
    : d(new Private())
{

}

JoimNotifications::~JoimNotifications()
{
}

JoimNotifications * JoimNotifications::instance()
{
    if (!Private::instance) {
        Private::instance = new JoimNotifications();
    }
    return Private::instance;
}
