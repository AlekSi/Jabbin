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

class ServicesModelItem: public QObject {
    Q_OBJECT
public:
    ServicesModelItem(ServicesModelItem * parent, QString data);
    virtual ~ServicesModelItem();

    int childCount();
    ServicesModelItem * child(int index);
    ServicesModelItem * parent() const;

    virtual void activate();

    QString title();
    QIcon icon();

    virtual int row();
    virtual int indexOf(ServicesModelItem * child);
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

    virtual void _load();
    virtual void _initChildren();

    QString m_title;
    QString m_data;
    QIcon m_icon;

    bool m_itemLoaded;
    bool m_childrenLoaded;
    QList < ServicesModelItem * > m_children;
    ServicesModelItem * m_parent;
};

class ServicesServerItem: public ServicesModelItem {
    Q_OBJECT
public:
    ServicesServerItem(ServicesModelItem * parent, QString server);

    enum ItemType {
        Server = Generic + 1
    };
    virtual int type() const;

protected:
    virtual void _load();
    virtual void _initChildren();

protected Q_SLOTS:
    void discoInfoFinished();

private:
    XMPP::DiscoItem m_discoItem;
    bool m_waitingForInfo;

};

class ServicesRootItem: public ServicesModelItem {
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

    void itemUpdated(ServicesModelItem * item);
    QModelIndex indexOf(ServicesModelItem * item);

    ServicesModel * q;
};

#endif // SERVICESMODEL_PH_
