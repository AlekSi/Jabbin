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

#ifndef CALLHISTORY_P_H
#define CALLHISTORY_P_H

#include "callhistory.h"
#include "ui_callhistoryitemdelegate_base.h"

class CallHistoryItem {
public:

    enum Roles {
        Id = Qt::UserRole,
        PrettyDisplay = Qt::UserRole + 1
    };

    CallHistoryItem();

    CallHistoryItem(QString _name, QString _id, QDateTime _time, CallHistoryModel::Status _status);

    // Name of the caller
    QString name;

    // Jabber ID or phone number of the caller
    QString id;

    // Timestamp
    QDateTime time;

    // Status
    CallHistoryModel::Status status;

};

class CallHistoryItemEditor: public QWidget, Ui::CallHistoryItemDelegateBase {
    Q_OBJECT
public:
    CallHistoryItemEditor(QWidget * parent, QAbstractItemModel * model);
    ~CallHistoryItemEditor();

    static CallHistoryItemEditor * editor;
    QModelIndex editingIndex;

public Q_SLOTS:
    void call();
    void del();
    void toPhoneBook();

private:
    QAbstractItemModel * m_model;

    friend class CallHistoryItemDelegate;
};

#endif // CALLHISTORY_P_H

