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
#include <QStyledItemDelegate>

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

    clickedItem = index;

    if (clickedWithButton == Qt::LeftButton) {
        switch (index.data(ServicesModel::ServiceTypeRole).toInt()) {
            case ServicesModel::Server:
            case ServicesModel::Room:
            case ServicesModel::User:
                if (!index.data(ServicesModel::ServiceStatusRole).toInt()) {
                    joinService();
                } else {
                    clickedWithButton = Qt::RightButton;
                }
                break;
            default:
                break;
        }
    }

    if (clickedWithButton == Qt::RightButton) {
        QMenu menu;


        switch (index.data(ServicesModel::ServiceTypeRole).toInt()) {
            case ServicesModel::Server:
            case ServicesModel::Room:
            case ServicesModel::User:
                if (int status = index.data(ServicesModel::ServiceStatusRole).toInt()) {
                    if (status == ServicesModel::Online) {
                        menu.addAction(tr("Log out"),
                                this, SLOT(serviceLogout()));
                    } else {
                        menu.addAction(tr("Log in"),
                                this, SLOT(serviceLogin()));
                    }
                    menu.addAction(tr("Change account"),
                            this, SLOT(joinService()));
                    menu.addAction(tr("Unregister service"),
                            this, SLOT(leaveService()));
                } else {
                    menu.addAction(tr("Register service"),
                            this, SLOT(joinService()));
                }
            default:
                menu.addSeparator();
                menu.addAction(tr("Reload service"),
                        this, SLOT(reloadItem()));
                break;
        }

        menu.exec(QCursor::pos());
    }
}

void ServicesPanel::Private::serviceLogout()
{
    Status s = makeStatus(STATUS_OFFLINE, "");
    account->actionAgentSetStatus(model->jid(clickedItem), s);
}

void ServicesPanel::Private::serviceLogin()
{
    Status s = makeStatus(STATUS_ONLINE, "");
    account->actionAgentSetStatus(model->jid(clickedItem), s);
}

bool ServicesPanel::Private::eventFilter(QObject * object, QEvent * event)
{
    if (object == treeServices->viewport() &&
            event->type() == QEvent::MouseButtonRelease) {
        qDebug() << "Released mouse";
        QMouseEvent * mouseEvent = static_cast < QMouseEvent * >(event);
        if (mouseEvent) {
            clickedWithButton = mouseEvent->button();
            if (clickedWithButton == Qt::RightButton &&
                !treeServices->indexAt(mouseEvent->pos()).isValid()
            ) {
                qDebug() << "popup the menu now!";
                QMenu menu;
                menu.addAction(tr("Reload list"),
                        this, SLOT(reloadAll()));
                menu.exec(QCursor::pos());
            }
        }
    }
    return QObject::eventFilter(object, event);
}

void ServicesPanel::Private::joinService()
{
    qDebug() << "join service" <<
        model->data(clickedItem, ServicesModel::AddressRole);
    account->featureActivated(Features::feature(Features::FID_Register),
            model->jid(clickedItem),
            QString());

    // model->reloadItem(clickedItem);
}

void ServicesPanel::Private::leaveService()
{
    qDebug() << "leave service" <<
        model->data(clickedItem, ServicesModel::AddressRole);
    account->actionRemove(model->jid(clickedItem));
    model->refresh(clickedItem);
}

void ServicesPanel::Private::reloadItem()
{
    qDebug() << "reload item" <<
        model->data(clickedItem, ServicesModel::AddressRole);
    model->refresh(clickedItem);
}

void ServicesPanel::Private::reloadAll()
{
    if (model)
        model->refresh(QModelIndex());
}

ServicesPanel * ServicesPanel::m_instance = NULL;

ServicesPanel * ServicesPanel::instance()
{
    return m_instance;
}

void ServicesPanel::init(PsiAccount * account)
{
    if (!account) return;

    d->account = account;
    if (d->model) {
        d->treeServices->setModel(0);
        d->model = 0;

        // delete d->model;
        // d->model = new ServicesModel(account);
        // d->treeServices->setModel(d->model);
    }
    d->reloadAll();
}

void ServicesPanel::showEvent(QShowEvent * event) {
    if (!d->model) {
        d->model = new ServicesModel(d->account);
        d->treeServices->setModel(d->model);
    }
    d->treeServices->viewport()->installEventFilter(d);

    for (int i = 0; i < d->model->rowCount(); i++) {
        d->treeServices->expand(d->model->index(i, 0));
    }

    d->treeServices->setFocus();

    if (event) QWidget::showEvent(event);
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

