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

#include <QList>

class ServicesModelItem {
public:
    ServicesModelItem(ServicesModelItem * parent, QString data);
    virtual ~ServicesModelItem();

    int childCount();
    ServicesModelItem * child(int index);
    ServicesModelItem * parent() const;

    void activate();

    QString title();
    QIcon icon();

    virtual int row();
    virtual int indexOf(ServicesModelItem * child);

    enum ItemType {
        Generic = 1
    };
    virtual int type() const;

protected:
    virtual void load();
    virtual void initChildren();

    QString m_title;
    QString m_data;
    QIcon m_icon;

    bool m_itemLoaded;
    bool m_childrenLoaded;
    QList < ServicesModelItem * > m_children;
    ServicesModelItem * m_parent;
};

class ServicesServerItem: public ServicesModelItem {
public:
    ServicesServerItem(ServicesModelItem * parent, QString server);

    enum ItemType {
        Server = Generic + 1
    };
    virtual int type() const;

protected:
    virtual void load();
    virtual void initChildren();
};

class ServicesRootItem: public ServicesModelItem {
public:
    ServicesRootItem();
    int row();

    void addService(const QString service);
};

class ServicesModel::Private {
public:
    Private();
    ~Private();

    ServicesRootItem * root;
};

#endif // SERVICESMODEL_PH_
