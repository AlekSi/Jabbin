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

#include "servicespanel.h"
#include "servicespanel_p.h"
#include "servicesmodel.h"

#include <QDebug>
#include <QMenu>
#include <QCursor>
#include <QMouseEvent>

using XMPP::Jid;

ServicesPanel::Private::Private(ServicesPanel * parent)
    : account(NULL), model(NULL), q(parent)
{
    setupUi(parent);
    connect(treeServices, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(itemClicked(const QModelIndex &)));
}

void ServicesPanel::Private::itemClicked(const QModelIndex & index)
{
    qDebug() << "item was clicked" << index.data();
    qDebug() << "         type is" << index.data(ServicesModel::ServiceTypeRole);
    qDebug() << "      address is" << index.data(ServicesModel::AddressRole);

    if (clickedWithButton == Qt::RightButton) {
        QMenu menu;

        clickedItem = index;

        switch (index.data(ServicesModel::ServiceTypeRole).toInt()) {
            case ServicesModel::Room:
            case ServicesModel::User:
                menu.addAction(tr("Join service"),
                        this, SLOT(joinService()));
                menu.addAction(tr("Bookmark to contact list"),
                        this, SLOT(addToContacts()));
            default:
                menu.addAction(tr("Reload"),
                        this, SLOT(reloadItem()));
        }

        menu.exec(QCursor::pos());
    }
}

bool ServicesPanel::Private::eventFilter(QObject * object, QEvent * event)
{
    if (object == treeServices->viewport() &&
            event->type() == QEvent::MouseButtonRelease) {
        qDebug() << "Released mouse";
        QMouseEvent * mouseEvent = static_cast < QMouseEvent * >(event);
        if (mouseEvent) {
            clickedWithButton = mouseEvent->button();
        }
    }
    return false;
}

void ServicesPanel::Private::joinService()
{
    qDebug() << "join service" <<
        model->data(clickedItem, ServicesModel::AddressRole);
}

void ServicesPanel::Private::addToContacts()
{
    qDebug() << "add to contacts" <<
        model->data(clickedItem, ServicesModel::AddressRole);
}

void ServicesPanel::Private::reloadItem()
{
    qDebug() << "reload item" <<
        model->data(clickedItem, ServicesModel::AddressRole);
}


ServicesPanel * ServicesPanel::m_instance = NULL;

ServicesPanel * ServicesPanel::instance()
{
    return m_instance;
}

void ServicesPanel::init(/*const Jid & jid,*/ PsiAccount * account)
{
    // d->jid = jid;
    d->account = account;
    //ServicesModel * model = new ServicesModel(account);
    //virtual void showEvent ( QShowEvent * event )d->treeServices->setModel(model);
}

void ServicesPanel::showEvent(QShowEvent *) {
    if (!d->model) {
        d->model = new ServicesModel(d->account);
        d->treeServices->setModel(d->model);
    }
    d->treeServices->viewport()->installEventFilter(d);
}

ServicesPanel::ServicesPanel(QWidget * parent)
    : QWidget(parent), d(new Private(this))
{
    m_instance = this;
}

ServicesPanel::~ServicesPanel()
{
    delete d;
}

