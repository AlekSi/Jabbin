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

#include "servicesmodel.h"
#include "servicesmodel_p.h"
#include <QDebug>
#include <QApplication>
#include <QTimer>

#include "xmpp_tasks.h"
#include "xmpp_discoitem.h"
#include "protocol/discoinfoquerier.h"
#define INVALIDATE_PERIOD 60000

// ServicesModelItem
ServicesModelItem::ServicesModelItem(ServicesModelItem * parent, QString data)
    : m_parent(parent), m_icon(), m_title(), m_data(data),
      m_itemLoaded(false), m_childrenLoaded(false)
{

}

ServicesModelItem::~ServicesModelItem()
{
    qDeleteAll(m_children);
}

int ServicesModelItem::type() const
{
    return Generic;
}

int ServicesModelItem::childCount()
{
    initChildren();
    return m_children.count();
}

ServicesModelItem * ServicesModelItem::child(int index)
{
    initChildren();
    if (index < 0 || index >= m_children.count()) {
        return NULL;
    }

    return m_children.at(index);
}

ServicesModelItem * ServicesModelItem::parent() const
{
    return m_parent;
}

void ServicesModelItem::activate()
{
    // TODO: Make this
}

QString ServicesModelItem::title()
{
    load();
    return m_title;
}

QIcon ServicesModelItem::icon()
{
    load();
    return m_icon;
}

void ServicesModelItem::invalidate()
{
    m_itemLoaded = false;
    load();
}

void ServicesModelItem::load()
{
    if (m_itemLoaded) return;
    _load();
}

void ServicesModelItem::initChildren()
{
    if (m_childrenLoaded) return;
    load();
    _initChildren();
    m_childrenLoaded = true;
}

void ServicesModelItem::_load()
{
    // TODO:
    m_title = QObject::tr("Loading");
    m_icon = QIcon(":/services/data/loading.png");
    notifyUpdated();
}

void ServicesModelItem::_initChildren()
{
    // TODO:
}

int ServicesModelItem::row()
{
    ServicesModelItem * p = parent();
    return p->indexOf(this);
}

int ServicesModelItem::indexOf(ServicesModelItem * child)
{
    return m_children.indexOf(child);
}

ServicesModel * ServicesModelItem::model() const
{
    return parent()->model();
}

void ServicesModelItem::notifyUpdated()
{
    m_itemLoaded = true;
    model()->d->itemUpdated(this);
}

// ServicesServerItem
ServicesServerItem::ServicesServerItem(ServicesModelItem * parent, QString server)
    : ServicesModelItem(parent, server),
      m_waitingForInfo(false)
{
    m_discoItem.setJid(server.stripWhiteSpace());
    m_discoItem.setNode(QString());

}

void ServicesServerItem::discoInfoFinished()
{
    JT_DiscoInfo * jt = (JT_DiscoInfo *)sender();
    qDebug() << "discoInfoFinished()" <<
            jt->item().jid().full();

    if (jt->success()) {
        if (jt->item().name().isEmpty()) {
            m_title = jt->item().jid().full();
        } else {
            m_title = jt->item().name();
        }
        // qDebug() << "features: " << jt->item().features().id() << XMPP::Features::feature(jt->item().features().id());
        m_icon = QIcon(":/services/data/service.png");
    } else {
        // Handle the error
        m_title = tr("Error connecting to %1").arg(m_data);
        m_icon = QIcon(":/services/data/error.png");
        QTimer::singleShot(INVALIDATE_PERIOD, this, SLOT(invalidate()));
    }

    m_waitingForInfo = false;
    notifyUpdated();
}

int ServicesServerItem::type() const
{
    return Server;
}

void ServicesServerItem::_load()
{
    m_icon = QIcon(":/services/data/loading.png");
    m_title = tr("Connecting to %1").arg(m_data);

    JT_DiscoInfo * jt = new JT_DiscoInfo(model()->psiAccount()->client()->rootTask());
    connect(jt, SIGNAL(finished()), SLOT(discoInfoFinished()));

    if (m_waitingForInfo) {
        return;
    }

    qDebug() << m_waitingForInfo << "Calling jt->get(); " << m_discoItem.jid().full();

    jt->get(m_discoItem);

    // this would be a lot prettier if jt->go() returned bool
    // - false if connection error
    m_waitingForInfo = (jt->client() && &jt->client()->stream());
    qDebug() << "Success connect:" << m_waitingForInfo;
    jt->go(true);
}

void ServicesServerItem::_initChildren()
{
    m_children << new ServicesModelItem(this, "www");
    m_children << new ServicesModelItem(this, "www2");
}

// ServicesRootItem
ServicesRootItem::ServicesRootItem(ServicesModel * model)
    : ServicesModelItem(NULL, QString()), m_model(model)
{
}

int ServicesRootItem::row()
{
    return 0;
}

void ServicesRootItem::addService(const QString service)
{
    m_children << new ServicesServerItem(this, service);
}

ServicesModel * ServicesRootItem::model() const
{
    return m_model;
}

// ServicesModel::Private
ServicesModel::Private::Private(ServicesModel * parent)
    : q(parent)
{
}

ServicesModel::Private::~Private()
{
}

void ServicesModel::Private::itemUpdated(ServicesModelItem * item)
{
    QModelIndex index = indexOf(item);
    q->dataChanged(index, index);
}

QModelIndex ServicesModel::Private::indexOf(ServicesModelItem * item)
{
    if (item == root) {
        return QModelIndex();
    }

    ServicesModelItem * parentItem = item->parent();
    return q->index(parentItem->row(), 0, indexOf(parentItem));
}

// ServicesModel
ServicesModel::ServicesModel(PsiAccount * psiAccount, QObject * parent)
    : QAbstractItemModel(parent),
      d(new Private(this))
{
    d->psiAccount = psiAccount;
    d->root = new ServicesRootItem(this);
    d->root->addService("jabber.org");
    d->root->addService("jabber.com");
    d->root->addService("localhost");
}

PsiAccount * ServicesModel::psiAccount() const
{
    return d->psiAccount;
}

ServicesModel::~ServicesModel()
{
    delete d;
}

QModelIndex ServicesModel::index(int row, int column, const QModelIndex & parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    ServicesModelItem * parentItem;

     if (!parent.isValid())
         parentItem = d->root;
     else
         parentItem = static_cast < ServicesModelItem * >
             (parent.internalPointer());

     ServicesModelItem * childItem = parentItem->child(row);
     if (childItem)
         return createIndex(row, column, childItem);
     else
         return QModelIndex();
}

QModelIndex ServicesModel::parent(const QModelIndex & index) const
{
    if (!index.isValid())
        return QModelIndex();

    ServicesModelItem * childItem = static_cast < ServicesModelItem * >
        (index.internalPointer());
    ServicesModelItem * parentItem = childItem->parent();

    if (parentItem == d->root)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int ServicesModel::rowCount(const QModelIndex & parent) const
{
    if (parent.column() > 0)
        return 0;

    ServicesModelItem * parentItem;
    if (!parent.isValid())
        parentItem = d->root;
    else
        parentItem = static_cast < ServicesModelItem * >
            (parent.internalPointer());

    return parentItem->childCount();
}

int ServicesModel::columnCount(const QModelIndex & parent) const
{
    return 1;
}

QVariant ServicesModel::data(const QModelIndex & index, int role) const
{
    ServicesModelItem * item;

     if (!index.isValid()) {
         item = d->root;
     } else {
         item = static_cast < ServicesModelItem * >
             (index.internalPointer());
     }

     if (!item) {
         return QVariant();
     }

    switch (role) {
        case Qt::DisplayRole:
            return item->title();
        case Qt::DecorationRole:
            return item->icon();
        case Qt::SizeHintRole:
            switch (item->type()) {
                case ServicesModelItem::Generic:
                    return QSize(24, 24);
                case ServicesServerItem::Server:
                    return QSize(32, 32);
                default:
                    return QSize(16, 16);
            }
        default:
            return QVariant();
    }

    return QVariant();
}

bool ServicesModel::setData(const QModelIndex & index, const QVariant & value,
        int role)
{
    // not supported
    return false;
}

QVariant ServicesModel::headerData(int section, Qt::Orientation orientation,
        int role) const
{
    switch (role) {
        case Qt::DisplayRole:
            return QString("Header");
        case Qt::SizeHintRole:
            return QSize(100, 32);
        default:
            return QVariant();
    }
}


bool ServicesModel::hasChildren(const QModelIndex & index) const
{
    return (rowCount(index) != 0);
}

Qt::ItemFlags ServicesModel::flags(const QModelIndex & index) const
{
    if (!index.isValid())
         return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}


void ServicesModel::sort(int column, Qt::SortOrder order)
{
    // nope
}


QStringList ServicesModel::mimeTypes() const
{
    return QStringList();
}

QMimeData * ServicesModel::mimeData(const QModelIndexList & indexes) const
{
    return NULL;
}

bool ServicesModel::dropMimeData(const QMimeData * data, Qt::DropAction action,
                  int row, int column, const QModelIndex & parent)
{
    return false;
}

Qt::DropActions ServicesModel::supportedDropActions() const
{
    return 0;
}

void ServicesModel::refresh(const QModelIndex & parent)
{
    //
}

