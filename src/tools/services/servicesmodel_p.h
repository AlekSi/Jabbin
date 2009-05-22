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

#ifndef SERVICESMODEL_PH_
#define SERVICESMODEL_PH_

#include <QObject>
#include <QList>
#include <QIcon>
#include "servicesmodel.h"

#include "xmpp_discoitem.h"
#include "xmpp_task.h"

/**
 * Generic implementation to be subclassed to implement the actual
 * service discovery
 */
class ServiceItem: public QObject {
    Q_OBJECT
public:
    ServiceItem(ServiceItem * parent, DiscoItem item);
    virtual ~ServiceItem();

    int childCount();
    ServiceItem * child(int index);
    ServiceItem * parent() const;

    virtual void activate();
    void removeMe();

    QString title();
    QIcon icon();
    XMPP::Jid jid();

    virtual int row();
    virtual int indexOf(ServiceItem * child);
    virtual ServicesModel * model() const;

    virtual int type() const;
    virtual void reload();

public Q_SLOTS:
    void invalidate();

protected:
    void load();
    void initChildren();
    void notifyUpdated();

    void clearChildren();
    void addChildren(const QList < ServiceItem * > & children);
    void addChild(ServiceItem * child);
    void removeChild(ServiceItem * child);

    virtual void _load();
    virtual void _initChildren();

    QString m_title;
    QIcon m_icon;

    bool m_itemLoaded;
    bool m_childrenLoaded;
    ServiceItem * m_parent;
    XMPP::DiscoItem m_discoItem;
    QList < XMPP::Task * > m_tasks;
    int m_type;

public:
    static void initIcons();
    static QIcon m_genericIcon;
    static QIcon m_loadingIcon;
    static QIcon m_errorIcon;
    static QIcon m_serviceIcon;
    static QIcon m_serverIcon;
    static QIcon m_roomIcon;
    static QIcon m_userIcon;

    static QIcon m_xmppIcon;
    static QIcon m_msnIcon;
    static QIcon m_myspaceIcon;
    static QIcon m_qqIcon;
    static QIcon m_aimIcon;
    static QIcon m_gtalkIcon;
    static QIcon m_jabberIcon;
    static QIcon m_facebookIcon;
    static QIcon m_gadugaduIcon;
    static QIcon m_yahooIcon;
    static QIcon m_icqIcon;
    static QIcon m_ircIcon;
    static QIcon m_smsIcon;

private:
    QList < ServiceItem * > m_children;
};

class PsiContact;

/**
 * Base discovery class for XMPP services
 */
class XmppServiceItem: public ServiceItem {
    Q_OBJECT
public:
    XmppServiceItem(ServiceItem * parent, QString server);
    XmppServiceItem(ServiceItem * parent, DiscoItem item);
    PsiContact * contact;
    bool isLoaded() const;

protected:
    virtual void _load();
    virtual void _initChildren();
    virtual QIcon _defaultIcon();

protected Q_SLOTS:
    void discoItemsFinished();
    void discoInfoFinished();

public Q_SLOTS:
    void contactUpdated();

private:
    bool m_waitingForInfo;
};

class ServicesRootItem: public ServiceItem {
public:
    ServicesRootItem(ServicesModel * model);
    int row();

    void addService(const QString service);
    void reload();
    ServicesModel * model() const;

private:
    ServicesModel * m_model;
};

class ServicesModel::Private: public QObject {
    Q_OBJECT
public:
    Private(ServicesModel * parent);
    ~Private();

    enum ModelStatus {
        Normal = 0,
        Deleting,
        Inserting
    };
    ModelStatus modelStatus;

    ServicesRootItem * root;
    PsiAccount * psiAccount;

    void itemUpdated(ServiceItem * item);

    ServiceItem * findItem(const Jid & jid);

    void childrenToBeAdded(ServiceItem * item, int from, int count);
    void childrenAdded();

    void childrenToBeRemoved(ServiceItem * item, int from, int count);
    void childrenRemoved();

    void childrenToBeCleared(ServiceItem * item, int count);
    void childrenCleared();

    QModelIndex indexOf(ServiceItem * item);

public Q_SLOTS:
    void contactUpdated(const Jid & jid);

private:
    ServicesModel * q;
};

#endif // SERVICESMODEL_PH_
