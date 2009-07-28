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

// #include "discodlg.h"

#include "psioptions.h"
#include "xmpp_tasks.h"
#include "xmpp_discoitem.h"
#include "protocol/discoinfoquerier.h"
#define INVALIDATE_PERIOD 60000

// ServiceItem
//
QIcon ServiceItem::m_genericIcon; // = QIcon(":/services/data/generic.png");
QIcon ServiceItem::m_loadingIcon; // = QIcon(":/services/data/loading.png");
QIcon ServiceItem::m_errorIcon;   // = QIcon(":/services/data/error.png");

QIcon ServiceItem::m_serviceIcon;  // = QIcon(":/services/data/service.png");
QIcon ServiceItem::m_serverIcon;   // = QIcon(":/services/data/list.png");
QIcon ServiceItem::m_roomIcon;     // = QIcon(":/services/data/users.png");
QIcon ServiceItem::m_userIcon;     // = QIcon(":/services/data/user.png");

QIcon ServiceItem::m_aimIcon;      // = QIcon(":/services/data/services/aim.png");
QIcon ServiceItem::m_facebookIcon; // = QIcon(":/services/data/services/facebook.png");
QIcon ServiceItem::m_gadugaduIcon; // = QIcon(":/services/data/services/gadu-gadu.png");
QIcon ServiceItem::m_gtalkIcon;    // = QIcon(":/services/data/services/gtalk.png");
QIcon ServiceItem::m_icqIcon;      // = QIcon(":/services/data/services/icq.png");
QIcon ServiceItem::m_ircIcon;      // = QIcon(":/services/data/services/irc.png");
QIcon ServiceItem::m_jabberIcon;   // = QIcon(":/services/data/services/jabber.png");
QIcon ServiceItem::m_msnIcon;      // = QIcon(":/services/data/services/msn.png");
QIcon ServiceItem::m_myspaceIcon;  // = QIcon(":/services/data/services/myspace.png");
QIcon ServiceItem::m_qqIcon;       // = QIcon(":/services/data/services/qq.png");
QIcon ServiceItem::m_sametimeIcon; // = QIcon(":/services/data/services/sametime.png");
QIcon ServiceItem::m_smsIcon;      // = QIcon(":/services/data/services/sms.png");
QIcon ServiceItem::m_xmppIcon;     // = QIcon(":/services/data/services/xmpp.png");
QIcon ServiceItem::m_yahooIcon;    // = QIcon(":/services/data/services/yahoo.png");

ServiceItem::ServiceItem(ServiceItem * parent, DiscoItem data)
    : QObject(parent), m_parent(parent), m_icon(), m_title(),
      m_itemLoaded(false), m_childrenLoaded(false), m_discoItem(data),
      m_type(ServicesModel::Generic)
{
}

ServiceItem::~ServiceItem()
{
    foreach (XMPP::Task * task, m_tasks) {
        task->safeDelete();
    }
    m_tasks.clear();

    qDeleteAll(m_children);
}

void ServiceItem::clearChildren()
{
    if (m_children.size() == 0)
        return;
    model()->d->childrenToBeCleared(this, m_children.size());
    qDeleteAll(m_children);
    m_children.clear();
    model()->d->childrenCleared();
}

void ServiceItem::removeChild(ServiceItem * child)
{
    model()->d->childrenToBeRemoved(this, m_children.indexOf(child), 1);
    m_children.removeAll(child);
    //child->deleteLater();
    model()->d->childrenRemoved();
}

void ServiceItem::addChild(ServiceItem * child)
{
    model()->d->childrenToBeAdded(this, m_children.size(), 1);
    m_children.append(child);
    model()->d->childrenAdded();
}

void ServiceItem::addChildren(const QList < ServiceItem * > & children)
{
    model()->d->childrenToBeAdded(this, m_children.size(), children.size());
    m_children += children;
    model()->d->childrenAdded();
}

int ServiceItem::type() const
{
    return m_type;
}

void ServiceItem::reload()
{
    clearChildren();
    m_itemLoaded = false;
    m_childrenLoaded = false;
    load();

}

int ServiceItem::childCount()
{
    initChildren();
    return m_children.count();
}

ServiceItem * ServiceItem::child(int index)
{
    initChildren();
    if (index < 0 || index >= m_children.count()) {
        return NULL;
    }

    return m_children.at(index);
}

ServiceItem * ServiceItem::parent() const
{
    return m_parent;
}

void ServiceItem::activate()
{
    // TODO: Make this
}

QString ServiceItem::title()
{
    load();
    return m_title;
}

QIcon ServiceItem::icon()
{
    load();
    return m_icon;
}

XMPP::Jid ServiceItem::jid()
{
    load();
    return m_discoItem.jid();
}

void ServiceItem::invalidate()
{
    m_itemLoaded = false;
    load();
}

void ServiceItem::load()
{
    if (m_itemLoaded) return;
    _load();
}

void ServiceItem::initChildren()
{
    if (m_childrenLoaded) return;
    load();
    _initChildren();
    m_childrenLoaded = true;
}

void ServiceItem::_load()
{
    // TODO:
    m_title = QObject::tr("Loading...");
    m_icon = m_loadingIcon;
    notifyUpdated();
}

void ServiceItem::_initChildren()
{
    // TODO:
}

int ServiceItem::row()
{
    ServiceItem * p = parent();
    return p->indexOf(this);
}

int ServiceItem::indexOf(ServiceItem * child)
{
    return m_children.indexOf(child);
}

void ServiceItem::removeMe()
{
    if (m_parent) {
        m_parent->removeChild(this);
    }
}

ServicesModel * ServiceItem::model() const
{
    return parent()->model();
}

void ServiceItem::notifyUpdated()
{
    m_itemLoaded = true;
    model()->d->itemUpdated(this);
}

// XmppServiceItem
XmppServiceItem::XmppServiceItem(ServiceItem * parent, QString server)
    : ServiceItem(parent, DiscoItem()),
      m_waitingForInfo(false), contact(NULL)
{
    m_discoItem.setJid(server.stripWhiteSpace());
    m_discoItem.setNode(QString());
    m_type = ServicesModel::Service;
}

XmppServiceItem::XmppServiceItem(ServiceItem * parent, DiscoItem item)
    : ServiceItem(parent, item),
      m_waitingForInfo(false), contact(NULL)
{
    if (parent) {
        m_type = parent->type() + 1;
    }
}

bool XmppServiceItem::isLoaded() const {
    return !m_waitingForInfo;
}

void XmppServiceItem::contactUpdated()
{
    if (contact) {
        m_title = contact->name() + " (" + contact->status().typeString() + ")";
        notifyUpdated();
    } else {
        reload();
    }
}

void XmppServiceItem::discoInfoFinished()
{
    JT_DiscoInfo * jt = (JT_DiscoInfo *)sender();

    if (jt->success() && jt->item().jid().full() != QString()) {
        m_discoItem = jt->item();

        if (!m_discoItem.identities().isEmpty()
                && ( m_discoItem.identities().first().category == "gateway"
                || m_discoItem.identities().first().category == "server")
                ) {
            m_title = m_discoItem.name();
            if (m_title.isEmpty() || m_title == "Openfire Server") {
                m_title = m_discoItem.jid().full();
            }

            contact = model()->psiAccount()->findContact(
                jt->item().jid());
            if (contact) {
                connect(contact, SIGNAL(updated()), this, SLOT(contactUpdated()));
                m_title += " (" + contact->status().typeString() + ")";
            } else {
                if (m_type != ServicesModel::Service) {
                    m_title += " (" + tr("not registered") + ")";
                }
            }

            m_waitingForInfo = false;

            if (m_discoItem.identities().first().category == "server") {
                _initChildren();
                addChild(new ServiceItem(this, DiscoItem()));
            }

            m_icon = _defaultIcon();
        } else {
            m_title = "delete me";
            removeMe();
        }
    } else {
        // Handle the error
        m_title = tr("Error connecting to %1").arg(m_discoItem.jid().bare());
        m_icon = m_errorIcon;
        QTimer::singleShot(INVALIDATE_PERIOD, this, SLOT(invalidate()));
    }

    m_waitingForInfo = false;
    notifyUpdated();

    m_tasks.remove(jt);
    jt->safeDelete();
}

void XmppServiceItem::discoItemsFinished()
{
    JT_DiscoItems *jt = (JT_DiscoItems *)sender();

    clearChildren();
    if ( jt->success() ) {

        QList < ServiceItem * > children;
        foreach (DiscoItem item, jt->items()) {
            children << new XmppServiceItem(this, item);
        }

        // Enable the following for scrollbar testing
        // for (int i = 0; i < 30; i++) {
        //     children << new ServiceItem(this, DiscoItem());
        // }

        addChildren(children);

        m_icon = _defaultIcon();
        notifyUpdated();
    }

    m_tasks.remove(jt);
    jt->safeDelete();
}

QIcon XmppServiceItem::_defaultIcon()
{
    if (!m_discoItem.identities().isEmpty()) {
        DiscoItem::Identity id = m_discoItem.identities().first();

        QIcon icon = ServicesModel::iconForTransportType(id.type);
        if (!icon.isNull())
            return icon;
    }

    switch (m_type) {
        case ServicesModel::Service:
            return m_serviceIcon;
        case ServicesModel::Server:
            return m_serverIcon;
        case ServicesModel::Room:
            return m_roomIcon;
        case ServicesModel::User:
            return m_userIcon;
        default:
            return m_genericIcon;
    }
    return QIcon(":/services/data/service.png");
}

void XmppServiceItem::_load()
{
    if (m_waitingForInfo) {
        return;
    }

    m_icon = m_loadingIcon;
    m_title = tr("Connecting to %1").arg(m_discoItem.jid().bare());

    JT_DiscoInfo * jt = new JT_DiscoInfo(model()->psiAccount()->client()->rootTask());
    m_tasks << jt;

    connect(jt, SIGNAL(finished()), SLOT(discoInfoFinished()));

    jt->get(m_discoItem);

    // this would be a lot prettier if jt->go() returned bool
    // - false if connection error
    m_waitingForInfo = (jt->client() && &jt->client()->stream());
    jt->go();
}

void XmppServiceItem::_initChildren()
{
    if (m_waitingForInfo) {
        return;
    }

    m_icon = m_loadingIcon;

    JT_DiscoItems * jt = new JT_DiscoItems(model()->psiAccount()->client()->rootTask());
    m_tasks << jt;
    connect(jt, SIGNAL(finished()), SLOT(discoItemsFinished()));

    jt->get(m_discoItem.jid(), m_discoItem.node());

    // this would be a lot prettier if jt->go() returned bool
    // - false if connection error
    m_waitingForInfo = (jt->client() && &jt->client()->stream());
    jt->go();

}

// ServicesRootItem
ServicesRootItem::ServicesRootItem(ServicesModel * model)
    : ServiceItem(NULL, DiscoItem()), m_model(model)
{
}

int ServicesRootItem::row()
{
    return 0;
}

void ServicesRootItem::addService(const QString service)
{
    // m_children << new XmppServiceItem(this, service);
    addChild(new XmppServiceItem(this, service));
}

void ServicesRootItem::reload()
{
    for (int i = 0; i < childCount(); i++) {
        child(i)->reload();
    }
}

ServicesModel * ServicesRootItem::model() const
{
    return m_model;
}

// ServicesModel::Private
ServicesModel::Private::Private(ServicesModel * parent)
    : q(parent), modelStatus(Normal)
{
    // Initializing icons here since they can not be
    // initialized before QApplication instance is
    // created
}

ServicesModel::Private::~Private()
{
}

void ServiceItem::initIcons()
{
    if (!ServiceItem::m_genericIcon.isNull())
        return;

    ServiceItem::m_genericIcon  = QIcon(":/services/data/generic.png");
    ServiceItem::m_loadingIcon  = QIcon(":/services/data/loading.png");
    ServiceItem::m_errorIcon    = QIcon(":/services/data/error.png");
    ServiceItem::m_serviceIcon  = QIcon(":/services/data/service.png");
    ServiceItem::m_serverIcon   = QIcon(":/services/data/list.png");
    ServiceItem::m_roomIcon     = QIcon(":/services/data/users.png");
    ServiceItem::m_userIcon     = QIcon(":/services/data/user.png");

    ServiceItem::m_aimIcon      = QIcon(":/services/data/services/aim.png");
    ServiceItem::m_facebookIcon = QIcon(":/services/data/services/facebook.png");
    ServiceItem::m_gadugaduIcon = QIcon(":/services/data/services/gadu-gadu.png");
    ServiceItem::m_gtalkIcon    = QIcon(":/services/data/services/gtalk.png");
    ServiceItem::m_icqIcon      = QIcon(":/services/data/services/icq.png");
    ServiceItem::m_ircIcon      = QIcon(":/services/data/services/irc.png");
    ServiceItem::m_jabberIcon   = QIcon(":/services/data/services/jabber.png");
    ServiceItem::m_msnIcon      = QIcon(":/services/data/services/msn.png");
    ServiceItem::m_myspaceIcon  = QIcon(":/services/data/services/myspace.png");
    ServiceItem::m_qqIcon       = QIcon(":/services/data/services/qq.png");
    ServiceItem::m_sametimeIcon = QIcon(":/services/data/services/sametime.png");
    ServiceItem::m_smsIcon      = QIcon(":/services/data/services/sms.png");
    ServiceItem::m_xmppIcon     = QIcon(":/services/data/services/xmpp.png");
    ServiceItem::m_yahooIcon    = QIcon(":/services/data/services/yahoo.png");
}

ServiceItem * ServicesModel::Private::findItem(const Jid & jid)
{
    QList < ServiceItem * > stack;
    stack << root;

    ServiceItem * item;
    while (stack.size()) {
        item = stack.takeFirst();
        if (item->jid() == jid) {
            return item;
        }

        for (int i = 0; i < item->childCount(); i++) {
            stack << item->child(i);
        }
    }

    return NULL;
}

void ServicesModel::Private::contactUpdated(const Jid & jid)
{
    if (jid.full().contains("@"))
        return;

    XmppServiceItem * item = dynamic_cast < XmppServiceItem * >
        (findItem(jid));
    if (item) {
        item->contactUpdated();
    }
}

void ServicesModel::Private::itemUpdated(ServiceItem * item)
{
    QModelIndex index = indexOf(item);
    q->dataChanged(index, index);
}

void ServicesModel::Private::childrenToBeAdded(ServiceItem * item, int from, int count)
{
    if (count < 1) return;
    modelStatus = Inserting;
    QModelIndex index = indexOf(item);
    q->beginInsertRows(index, from, from + count - 1);
}

void ServicesModel::Private::childrenAdded()
{
    if (modelStatus == Inserting) {
        q->endInsertRows();
    }
    modelStatus == Normal;
}

void ServicesModel::Private::childrenToBeRemoved(ServiceItem * item, int from, int count)
{
    if (count < 1) return;
    modelStatus = Deleting;
    QModelIndex index = indexOf(item);
    q->beginRemoveRows(index, from, from + count - 1);
}

void ServicesModel::Private::childrenRemoved()
{
    if (modelStatus == Deleting) {
        q->endRemoveRows();
    }
    modelStatus == Normal;
}

void ServicesModel::Private::childrenToBeCleared(ServiceItem * item, int count)
{
    if (count < 1) return;
    modelStatus = Deleting;
    QModelIndex index = indexOf(item);
    q->beginRemoveRows(index, 0, count - 1);
}

void ServicesModel::Private::childrenCleared()
{
    if (modelStatus == Deleting) {
        q->endRemoveRows();
    }
    modelStatus == Normal;
}

QModelIndex ServicesModel::Private::indexOf(ServiceItem * item)
{
    if (item == root) {
        return QModelIndex();
    }

    ServiceItem * parentItem = item->parent();
    return q->index(parentItem->row(), 0, indexOf(parentItem));
}

// ServicesModel
ServicesModel::ServicesModel(PsiAccount * psiAccount, QObject * parent)
    : QAbstractItemModel(parent),
      d(new Private(this))
{
    ServiceItem::initIcons();
    d->psiAccount = psiAccount;
    connect(d->psiAccount, SIGNAL(updateContact(Jid)),
        d, SLOT(contactUpdated(Jid)));
    d->root = new ServicesRootItem(this);

    // QVariantList lvalue = PsiOptions::instance()
    //     ->getOption("service.providers.list").toList();

    // foreach (QVariant service, lvalue) {
    //     d->root->addService(service.toString());
    // }

    qDebug() << "Setting service domain to" <<
        psiAccount->jid().domain();
    d->root->addService(psiAccount->jid().host());

    // DiscoDlg * dlg = new DiscoDlg(psiAccount, psiAccount->jid(), QString());
    // dlg->show();
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

    ServiceItem * parentItem;

     if (!parent.isValid())
         parentItem = d->root;
     else
         parentItem = static_cast < ServiceItem * >
             (parent.internalPointer());

     ServiceItem * childItem = parentItem->child(row);
     if (childItem)
         return createIndex(row, column, childItem);
     else
         return QModelIndex();
}

QModelIndex ServicesModel::parent(const QModelIndex & index) const
{
    if (!index.isValid())
        return QModelIndex();

    ServiceItem * childItem = static_cast < ServiceItem * >
        (index.internalPointer());
    ServiceItem * parentItem = childItem->parent();

    if (parentItem == d->root)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int ServicesModel::rowCount(const QModelIndex & parent) const
{
    if (parent.column() > 0)
        return 0;

    ServiceItem * parentItem;
    if (!parent.isValid())
        parentItem = d->root;
    else
        parentItem = static_cast < ServiceItem * >
            (parent.internalPointer());

    return parentItem->childCount();
}

int ServicesModel::columnCount(const QModelIndex & parent) const
{
    return 1;
}

QVariant ServicesModel::data(const QModelIndex & index, int role) const
{
    ServiceItem * item;

     if (!index.isValid()) {
         item = d->root;
     } else {
         item = static_cast < ServiceItem * >
             (index.internalPointer());
     }

     if (!item) {
         return QVariant();
     }

    switch (role) {
        case Qt::DisplayRole:
            return item->title();
        case Qt::DecorationRole:
        {
            XmppServiceItem * xmppitem = dynamic_cast < XmppServiceItem * > (item);
            if (!xmppitem || !xmppitem->contact && xmppitem->type() != Service)
                return item->icon().pixmap(
                        data(index, Qt::SizeHintRole).toSize()
                        - QSize(4, 4), QIcon::Disabled);
            return item->icon().pixmap(
                    data(index, Qt::SizeHintRole).toSize()
                    - QSize(4, 4), QIcon::Normal);
        }
        case Qt::SizeHintRole:
            switch (item->type()) {
                case Generic:
                    return QSize(28, 28);
                case Service:
                    {
                    XmppServiceItem * xmppitem = dynamic_cast < XmppServiceItem * > (item);
                    if (xmppitem->isLoaded())
                         return QSize(0, 0);
                    else
                         return QSize(36, 36);
                    }
                case Server:
                    return QSize(36, 36);
                case Room:
                    return QSize(24, 24);
                case User:
                    return QSize(24, 24);
                default:
                    return QSize(16, 16);
            }
        case Qt::ForegroundRole:
        {
            XmppServiceItem * xmppitem = dynamic_cast < XmppServiceItem * > (item);
            if (!xmppitem || !xmppitem->contact && xmppitem->type() != Service)
                return QColor(0, 0, 0, 100);
            return QVariant();
        }
        case ServiceTypeRole:
            return item->type();
        case ServiceStatusRole:
        {
            XmppServiceItem * xmppitem = dynamic_cast < XmppServiceItem * > (item);
            if (!xmppitem || !xmppitem->contact)
                return ServicesModel::Unregistered;
            if (xmppitem->contact->status().type() == Status::Offline)
                return ServicesModel::Offline;
            else
                return ServicesModel::Online;
        }
            return 0; //item->...();
        case AddressRole:
            return item->jid().bare();
        default:
            return QVariant();
    }

    return QVariant();
}

XMPP::Jid ServicesModel::jid(const QModelIndex & index) const
{
    ServiceItem * item;

    if (!index.isValid()) {
        item = d->root;
    } else {
        item = static_cast < ServiceItem * >
            (index.internalPointer());
    }

    if (!item) {
        return XMPP::Jid();
    }

    return item->jid();
}

void ServicesModel::reloadItem(const QModelIndex & index)
{
    ServiceItem * item;

    if (!index.isValid()) {
        item = d->root;
    } else {
        item = static_cast < ServiceItem * >
            (index.internalPointer());
    }

    if (item) {
        item->reload();
    }
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
    ServiceItem * parentItem;

     if (!parent.isValid())
         parentItem = d->root;
     else
         parentItem = static_cast < ServiceItem * >
             (parent.internalPointer());

     parentItem->reload();
}

QIcon ServicesModel::iconForTransportType(const QString & type)
{
    if (type == "aim")
        return ServiceItem::m_aimIcon;
    else if (type == "irc")
        return ServiceItem::m_ircIcon;
    else if (type == "gadu-gadu")
        return ServiceItem::m_gadugaduIcon;
    else if (type == "icq")
        return ServiceItem::m_icqIcon;
    else if (type == "lcs")
        return ServiceItem::m_msnIcon;
    else if (type == "msn")
        return ServiceItem::m_msnIcon;
    else if (type == "ocs")
        return ServiceItem::m_msnIcon;
    else if (type == "qq")
        return ServiceItem::m_qqIcon;
    else if (type == "sms")
        return ServiceItem::m_smsIcon;
    else if (type == "xmpp")
        return ServiceItem::m_jabberIcon;
    else if (type == "yahoo")
        return ServiceItem::m_yahooIcon;
    else if (type == "facebook")
        return ServiceItem::m_facebookIcon;
    else if (type == "myspaceim")
        return ServiceItem::m_myspaceIcon;
    else if (type == "sametime")
        return ServiceItem::m_sametimeIcon;
    return QIcon();
}
