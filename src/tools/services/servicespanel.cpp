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

    clickedItem = index;

    if (clickedWithButton == Qt::LeftButton) {
        switch (index.data(ServicesModel::ServiceTypeRole).toInt()) {
            case ServicesModel::Server:
            case ServicesModel::Room:
            case ServicesModel::User:
                joinService();
                break;
            default:
                break;
        }
    } else if (clickedWithButton == Qt::RightButton) {
        QMenu menu;


        switch (index.data(ServicesModel::ServiceTypeRole).toInt()) {
            case ServicesModel::Server:
            case ServicesModel::Room:
            case ServicesModel::User:
                menu.addAction(tr("Register service"),
                        this, SLOT(joinService()));
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
    return QObject::eventFilter(object, event);
}

void ServicesPanel::Private::joinService()
{
    qDebug() << "join service" <<
        model->data(clickedItem, ServicesModel::AddressRole);
    account->featureActivated(Features::feature(Features::FID_Register),
            model->jid(clickedItem),
            QString());
}

void ServicesPanel::Private::reloadItem()
{
    qDebug() << "reload item" <<
        model->data(clickedItem, ServicesModel::AddressRole);
    model->refresh(clickedItem);
}


ServicesPanel * ServicesPanel::m_instance = NULL;

ServicesPanel * ServicesPanel::instance()
{
    return m_instance;
}

void ServicesPanel::init(PsiAccount * account)
{
    d->account = account;
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
    QWidget::showEvent(event);
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

