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

#include "servicespopupbutton.h"
#include "psiaccount.h"

#include <QDebug>
#include <QMenu>
#include <QHash>

// ServicesPopupButton::Private
class ServicesPopupButton::Private: public QObject {
    Q_OBJECT

public:
    Private(ServicesPopupButton * parent);

public Q_SLOTS:
    void menuAboutToBeShown();
    void actionChosen(QAction * action);

public:
    QMenu * menu;
    static PsiAccount * account;
    QHash < QAction *, QString > actions;
    QString selected;

    ServicesPopupButton * const q;
};
