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

#include "selfavatar.h"
#include <QToolButton>
#include <QMenu>
#include <QDir>
#include <QDebug>
#include <QList>

#define MENU_AVATAR_ICON_SIZE QSize(32, 32)
#define MENU_AVATAR_COLUMN_COUNT 5
#define MENU_AVATAR_ROW_COUNT_MAX 7

class SelfAvatarMenu: public QMenu {
public:
    SelfAvatarMenu(SelfAvatar * parent)
        : QMenu(parent), q(parent)
    {
        grid = new QGridLayout(this);
        grid->setMargin(style()->pixelMetric(QStyle::PM_MenuPanelWidth, 0, this));
        grid->setSpacing(0);

        custom = new QToolButton(this);
        custom->setText(tr("Choose a custom avatar..."));
        custom->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        buttonStyle = CustomWidgets::Common::readFile(":/customwidgets/data/selfavatarmenu_button_style.css");
    }

    ~SelfAvatarMenu()
    {
    }

    void updateMenu()
    {
        QList < QDir > dirs;
        dirs << QDir(":images/avatars");

        int count = 0;

        grid->removeWidget(custom);

        foreach (QDir dir, dirs) {
            foreach(QString avatarName, dir.entryList()) {
                if (count >= MENU_AVATAR_COLUMN_COUNT * MENU_AVATAR_ROW_COUNT_MAX) break;

                QIcon avatar = QIcon(dir.absoluteFilePath(avatarName));

                if (buttons.size() > count) {
                    buttons.at(count)->setIcon(avatar);
                } else {
                    QToolButton * button = new QToolButton(this);
                    button->setIconSize(MENU_AVATAR_ICON_SIZE);
                    button->setIcon(avatar);
                    button->setStyleSheet(buttonStyle);
                    button->installEventFilter(this);
                    buttons.append(button);
                    grid->addWidget(button, count / MENU_AVATAR_COLUMN_COUNT, count % MENU_AVATAR_COLUMN_COUNT);
                    connect(button, SIGNAL(clicked()),
                            this, SLOT(close()));
                }

                ++count;
            }
        }

        grid->addWidget(custom, count / MENU_AVATAR_COLUMN_COUNT + 1, 0, 1, MENU_AVATAR_COLUMN_COUNT);

        while (buttons.size() > count) {
            QToolButton * button = buttons.takeLast();
            delete button;
        }
    }

    bool eventFilter(QObject * object, QEvent * event)
    {
        if (event->type() == QEvent::MouseButtonRelease) {
            QToolButton * button = static_cast < QToolButton * > (object);
            if (buttons.contains(button)) {
                q->setIcon(button->icon());
            }
        }
        return QMenu::eventFilter(object, event);
    }


private:
    QList < QToolButton * > buttons;
    QToolButton * custom;
    SelfAvatar * q;
    QGridLayout * grid;
    QString buttonStyle;
};

class SelfAvatar::Private {
public:
    Private()
        : menu(NULL)
    {
    }

    SelfAvatarMenu * menu;
};

SelfAvatar::SelfAvatar(QWidget * parent)
    : CustomWidgets::AvatarButton(parent), d(new Private())
{
    connect(this, SIGNAL(clicked()),
            this, SLOT(popupAvatarsMenu()));
}

SelfAvatar::~SelfAvatar()
{
    delete d;
}

void SelfAvatar::setContactList(const PsiContactList * contactList)
{
    // TODO
}

void SelfAvatar::setSelfContact(PsiContact * contact)
{
    setIcon(contact->picture());
}

void SelfAvatar::popupAvatarsMenu()
{
    if (!d->menu) {
        d->menu = new SelfAvatarMenu(this);
    }

    d->menu->updateMenu();
    d->menu->exec(mapToGlobal(rect().bottomLeft()));
}


