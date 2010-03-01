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

#ifndef SERVICESMODEL_H_
#define SERVICESMODEL_H_

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qdir.h>
#include <QtGui/qfileiconprovider.h>
#include "psiaccount.h"
#include "psicontact.h"

class ServiceItem;

class ServicesModel: public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Roles {
        AddressRole = Qt::UserRole + 1,
        ServiceTypeRole,
        ServiceStatusRole
    };

    enum ItemTypes {
        NoType = 0,
        Generic,
        Service,
        Server,
        Room,
        User
    };

    enum ServiceStatus {
        Unregistered = 0,
        Offline,
        Online
    };

    ServicesModel(PsiAccount * psiAccount, QObject * parent = 0);
    ~ServicesModel();

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    XMPP::Jid jid(const QModelIndex & index) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    bool hasChildren(const QModelIndex &index = QModelIndex()) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent);
    Qt::DropActions supportedDropActions() const;

    PsiAccount * psiAccount() const;
    void setPsiAccount(PsiAccount * account);

    void reloadItem(const QModelIndex & index);

    static QIcon iconForTransportType(const QString & type);

public Q_SLOTS:
    void refresh(const QModelIndex &parent = QModelIndex());

private:
    class Private;
    Private * const d;

    friend class ServiceItem;
};

#endif // SERVICESMODEL_H_
