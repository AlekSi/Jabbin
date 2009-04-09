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

#include "emoticonspopupbutton.h"
#include "emoticonspopupbutton_p.h"

#include <QDebug>

// EmoticonsPopupButton::Private
EmoticonsPopupButton::Private::Private(EmoticonsPopupButton * parent)
    : QObject(), menu(NULL), q(parent)
{
    menu = new QMenu();
    connect(menu, SIGNAL(aboutToShow()),
            this, SLOT(menuAboutToBeShown()));
    connect(menu, SIGNAL(triggered(QAction *)),
            this, SLOT(actionChosen(QAction *)));
    parent->setMenu(menu);
    parent->setPopupMode(QToolButton::InstantPopup);
};

void EmoticonsPopupButton::Private::actionChosen(QAction * action)
{
    if (action) {
        // selected = actions[action];
    }
}

void EmoticonsPopupButton::Private::menuAboutToBeShown()
{
    menu->clear();
    actions.clear();

    QListIterator < PsiIcon * > i = iconset.iterator();
    while (i.hasNext()) {
        PsiIcon * icon = i.next();
        menu->addAction(icon->icon(), icon->name());
    }
}

EmoticonsPopupButton::EmoticonsPopupButton(QWidget * parent)
    : QToolButton(parent), d(new Private(this))
{
    setIcon(QIcon(":/services/data/services/jabber.png"));
}

EmoticonsPopupButton::~EmoticonsPopupButton()
{
    delete d;
}

void EmoticonsPopupButton::setIconset(const Iconset & iconset)
{
    d->iconset = iconset;
}

