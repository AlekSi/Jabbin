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

void ServicesModelItem::load()
{
    if (m_itemLoaded) return;
    // TODO:

    m_itemLoaded = true;
}

void ServicesModelItem::initChildren()
{
    if (m_childrenLoaded) return;
    load();
    // TODO:

    m_childrenLoaded = true;
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

// ServicesServerItem
ServicesServerItem::ServicesServerItem(ServicesModelItem * parent, QString server)
    : ServicesModelItem(parent, server)
{
}

int ServicesServerItem::type() const
{
    return Server;
}

void ServicesServerItem::load()
{
    m_title = m_data;
    m_icon = QIcon(":/services/data/service.png");
}

void ServicesServerItem::initChildren()
{
    if (m_childrenLoaded) return;
    load();

    m_children << new ServicesModelItem(this, "www");
    m_children << new ServicesModelItem(this, "www2");
    m_childrenLoaded = true;
}

// ServicesRootItem
ServicesRootItem::ServicesRootItem()
    : ServicesModelItem(NULL, QString())
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

// ServicesModel::Private
ServicesModel::Private::Private()
    : root(new ServicesRootItem())
{
}

ServicesModel::Private::~Private()
{
}

// ServicesModel
ServicesModel::ServicesModel(QObject * parent)
    : QAbstractItemModel(parent),
      d(new Private())
{
    qDebug() << "ServicesModel::ServicesModel(...)";
    d->root->addService("jabber.org");
    d->root->addService("jabber.com");
}

ServicesModel::~ServicesModel()
{
    qDebug() << "ServicesModel::~ServicesModel()";
    delete d;
}

QModelIndex ServicesModel::index(int row, int column, const QModelIndex & parent) const
{
    qDebug() << "ServicesModel::index " << row << column << parent;
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
    qDebug() << "ServicesModel::parent " << index;
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
    qDebug() << "ServicesModel::rowCount " << parent;
    if (parent.column() > 0)
        return 0;

    ServicesModelItem * parentItem;
    if (!parent.isValid())
        parentItem = d->root;
    else
        parentItem = static_cast < ServicesModelItem * >
            (parent.internalPointer());

    qDebug() << parentItem->title() << parentItem->childCount();
    return parentItem->childCount();
}

int ServicesModel::columnCount(const QModelIndex & parent) const
{
    qDebug() << "ServicesModel::columnCount " << parent;
    return 1;
}

QVariant ServicesModel::data(const QModelIndex & index, int role) const
{
    qDebug() << "ServicesModel::data " << index << role;
    ServicesModelItem * item;

     if (!index.isValid()) {
         qDebug() << "           index is not valid, using root element";
         item = d->root;
     } else {
         qDebug() << "           index is valid, using root element";
         item = static_cast < ServicesModelItem * >
             (index.internalPointer());
     }

     if (!item) {
         qDebug() << "           item is null, exiting";
         return QVariant();
     }

    switch (role) {
        case Qt::DisplayRole:
            qDebug() << "           returning title";
            return item->title();
        case Qt::DecorationRole:
            qDebug() << "           returning icon";
            return item->icon();
        case Qt::SizeHintRole:
            qDebug() << "           returning size for " << (void *) item;
            qDebug() << "           type is " << item->type();
            switch (item->type()) {
                case ServicesModelItem::Generic:
                    qDebug() << "           returning 24";
                    return QSize(24, 24);
                case ServicesServerItem::Server:
                    qDebug() << "           returning 32";
                    return QSize(32, 32);
                default:
                    qDebug() << "           returning 16";
                    return QSize(16, 16);
            }
        default:
            qDebug() << "           unknown role";
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
    qDebug() << "ServicesModel::headerData " << role;
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
    qDebug() << "ServicesModel::hasChildren " << index;
    return (rowCount(index) != 0);
}

Qt::ItemFlags ServicesModel::flags(const QModelIndex & index) const
{
    qDebug() << "ServicesModel::flags " << index;
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

