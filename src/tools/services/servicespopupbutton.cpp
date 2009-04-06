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
#include "servicespopupbutton_p.h"
#include "servicesmodel.h"
#include "psicontact.h"

#include <QDebug>
#include <QMenu>

// ServicesPopupButton::Private
ServicesPopupButton::Private::Private(ServicesPopupButton * parent)
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

PsiAccount * ServicesPopupButton::Private::account = NULL;

void ServicesPopupButton::Private::actionChosen(QAction * action)
{
    if (action) {
        selected = actions[action];
        q->setToolTip(action->text());
        q->setIcon(action->icon());
    }
}

void ServicesPopupButton::Private::menuAboutToBeShown()
{
    menu->clear();
    // qDeleteAll(actions);
    actions.clear();

    if (account) {
        QString accountServer = account->jid().bare();
        accountServer = accountServer.mid(
                accountServer.lastIndexOf("@") + 1);
        menu->addAction(
                ServicesModel::iconForTransportType("xmpp"),
                tr("Jabber (no transport)"));

        foreach(PsiContact * contact, account->contactList()) {
            QString jid = contact->jid().bare();
            if (jid.contains("@") || !jid.endsWith(accountServer))
                continue;
            QString type = jid.left(jid.indexOf("."));
            actions[menu->addAction(
                ServicesModel::iconForTransportType(type),
                contact->name())] = jid;
        }
    }
}

ServicesPopupButton::ServicesPopupButton(QWidget * parent)
    : QToolButton(parent), d(new Private(this))
{
    setIcon(QIcon(":/services/data/services/jabber.png"));
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    setStyleSheet(
        "background: none;"
        "margin-top: -2px;"
        "padding-left: 10px;"
        "border-left: 2px solid silver;"
    );
}

ServicesPopupButton::~ServicesPopupButton()
{
    delete d;
}

void ServicesPopupButton::setAccount(PsiAccount * account)
{
    ServicesPopupButton::Private::account = account;
}

QString ServicesPopupButton::transport() const
{
    return d->selected;
}
