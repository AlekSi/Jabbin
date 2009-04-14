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
#include <QStyle>
#include <QEvent>
#include <cmath>

EmoticonsPopupMenu::EmoticonsPopupMenu(EmoticonsPopupButton * parent)
    : QMenu(parent), q(parent)
{
    grid = new QGridLayout(this);
    grid->setMargin(style()->pixelMetric(QStyle::PM_MenuPanelWidth, 0, this));
    grid->setSpacing(4);
}

EmoticonsPopupMenu::~EmoticonsPopupMenu()
{
}

void EmoticonsPopupMenu::updateMenu(Iconset * iconset)
{
    int count = iconset->count();
    int width = sqrt(count) + 1;

    int index = 0;
    QListIterator < PsiIcon * > i = iconset->iterator();
    while (i.hasNext()) {
        PsiIcon * icon = i.next();
        // addAction(icon->icon(), icon->name());
        QToolButton * button = new QToolButton(this);
        button->setIcon(icon->icon());
        button->installEventFilter(this);
        buttons[button] = icon->name();
        buttons[button] = icon->defaultText();

        grid->addWidget(button,
            index / width, index % width);
        connect(button, SIGNAL(clicked()),
                this, SLOT(close()));
        index++;
    }
}

bool EmoticonsPopupMenu::eventFilter(QObject * object, QEvent * event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QToolButton * button = static_cast < QToolButton * > (object);
        if (buttons.contains(button)) {
            q->textSelected(buttons[button]);
            // q->textSelected(":)");
            // q->setAvatar(button->icon());
        }
    }
    return QMenu::eventFilter(object, event);
}

// EmoticonsPopupButton::Private
EmoticonsPopupButton::Private::Private(EmoticonsPopupButton * parent)
    : QObject(), menu(NULL), q(parent)
{
    menu = new EmoticonsPopupMenu(q);
    connect(menu, SIGNAL(aboutToShow()),
            this, SLOT(menuAboutToBeShown()));
    connect(menu, SIGNAL(triggered(QAction *)),
            this, SLOT(actionChosen(QAction *)));
    parent->setMenu(menu);
    parent->setPopupMode(QToolButton::InstantPopup);
};

void EmoticonsPopupButton::Private::menuAboutToBeShown()
{
    menu->updateMenu(& iconset);

}

EmoticonsPopupButton::EmoticonsPopupButton(QWidget * parent)
    : QToolButton(parent), d(new Private(this))
{
    // setIcon(QIcon(":/services/data/services/jabber.png"));
}

EmoticonsPopupButton::~EmoticonsPopupButton()
{
    delete d;
}

void EmoticonsPopupButton::setIconset(const Iconset & iconset)
{
    d->iconset = iconset;

    QListIterator < PsiIcon * > i = iconset.iterator();
    while (i.hasNext()) {
        PsiIcon * icon = i.next();
        setIcon(icon->icon());
        return;
    }
}

