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

    QString title();
    QIcon icon();

    virtual int row();
    virtual int indexOf(ServiceItem * child);
    virtual ServicesModel * model() const;

    enum ItemType {
        Generic = 1
    };
    virtual int type() const;

public Q_SLOTS:
    void invalidate();

protected:
    void load();
    void initChildren();
    void notifyUpdated();

    void clearChildren();
    void addChildren(const QList < ServiceItem * > & children);
    void addChild(ServiceItem * child);

    virtual void _load();
    virtual void _initChildren();

    QString m_title;
    QIcon m_icon;

    bool m_itemLoaded;
    bool m_childrenLoaded;
    ServiceItem * m_parent;
    XMPP::DiscoItem m_discoItem;
    int m_type;

public:
    static QIcon m_genericIcon;
    static QIcon m_loadingIcon;
    static QIcon m_errorIcon;
    static QIcon m_serviceIcon;
    static QIcon m_serverIcon;
    static QIcon m_roomIcon;
    static QIcon m_userIcon;

private:
    QList < ServiceItem * > m_children;
};

/**
 * Base discovery class for XMPP services
 */
class XmppServiceItem: public ServiceItem {
    Q_OBJECT
public:
    XmppServiceItem(ServiceItem * parent, QString server);
    XmppServiceItem(ServiceItem * parent, DiscoItem item);

    enum ItemType {
        Service = Generic + 1,
        Server  = Generic + 2,
        Room    = Generic + 3,
        User    = Generic + 4
    };

protected:
    virtual void _load();
    virtual void _initChildren();
    virtual QIcon _defaultIcon();

protected Q_SLOTS:
    void discoItemsFinished();
    void discoInfoFinished();

private:
    bool m_waitingForInfo;
};

class ServicesRootItem: public ServiceItem {
public:
    ServicesRootItem(ServicesModel * model);
    int row();

    void addService(const QString service);
    ServicesModel * model() const;

private:
    ServicesModel * m_model;
};

class ServicesModel::Private {
public:
    Private(ServicesModel * parent);
    ~Private();

    ServicesRootItem * root;
    PsiAccount * psiAccount;

    void itemUpdated(ServiceItem * item);

    void childrenToBeAdded(ServiceItem * item, int from, int count);
    void childrenAdded();

    void childrenToBeCleared(ServiceItem * item, int count);
    void childrenCleared();

    QModelIndex indexOf(ServiceItem * item);


    ServicesModel * q;
};

#endif // SERVICESMODEL_PH_
