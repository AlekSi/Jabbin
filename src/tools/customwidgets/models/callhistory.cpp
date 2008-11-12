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

#include "callhistory.h"
#include <QTime>
#include <QDebug>
#include <QSize>
#include <QIcon>
#include <QItemDelegate>
#include "ui_callhistoryitemdelegate_base.h"

class CallHistoryItem {
public:
    enum Status {
        Invalid = -1,
        Unknown = 0,
        Sent,
        Received,
        Missed,
        Rejected
    };

    enum Roles {
        Id = Qt::UserRole
    };

    CallHistoryItem()
        : name(QString()), id(QString()), time(QDateTime()), status(Invalid)
    {

    }

    CallHistoryItem(QString _name, QString _id, QDateTime _time, Status _status)
        : name(_name), id(_id), time(_time), status(_status)
    {

    }

    // Name of the caller
    QString name;

    // Jabber ID or phone number of the caller
    QString id;

    // Timestamp
    QDateTime time;

    // Status
    Status status;

};

class CallHistoryModel::Private {
public:
    QList < CallHistoryItem > items;
    QListView * list;
};

class CallHistoryItemEditor: public QWidget, Ui::CallHistoryItemDelegateBase {
    CallHistoryItemEditor(QWidget * parent)
        : QWidget(parent)
    {
        setupUi(this);
    }

    friend class CallHistoryItemDelegate;
};

CallHistoryItemDelegate::CallHistoryItemDelegate(CallHistoryModel * model)
    : m_model(model)
{
}

QWidget * CallHistoryItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option,
                       const QModelIndex & index) const {
    return new CallHistoryItemEditor(parent);
}

void CallHistoryItemDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
    if (!index.isValid() || index.row() < 0 || index.row() > m_model->rowCount()) {
        return;
    }

    //CallHistoryItem item = m_model->d->items.at(index.row());
    //((CallHistoryItemEditor *) editor)->labelName->setText(item->name);
}

void CallHistoryItemDelegate::setModelData(QWidget * editor, QAbstractItemModel * model,
        const QModelIndex & index) const
{
    // this is really a read only class,
    // but we use a delegate for fancy display
}

void CallHistoryItemDelegate::updateEditorGeometry(QWidget * editor,
     const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QRect rect = option.rect;
    rect.setLeft(rect.right() - 32);
    editor->setGeometry(rect);
}

CallHistoryModel::CallHistoryModel(QListView * parent)
    : QAbstractListModel(parent), d(new Private())
{
    d->items.append(CallHistoryItem("test1", "id1", QDateTime::currentDateTime(), CallHistoryItem::Unknown));
    d->items.append(CallHistoryItem("test2", "id2", QDateTime::currentDateTime(), CallHistoryItem::Received));
    d->items.append(CallHistoryItem("test3", "id3", QDateTime::currentDateTime(), CallHistoryItem::Rejected));

    d->list = parent;
    d->list->setModel(this);
}

CallHistoryModel::~CallHistoryModel()
{
    delete d;
}

int CallHistoryModel::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);
    return d->items.count();
}

int CallHistoryModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

// bool CallHistoryModel::removeRows(int row, int count, const QModelIndex & parent)
// {
//     beginRemoveRows(parent, row, count);
//
//     endRemoveRows();
// }
//
// bool CallHistoryModel::insertRows(int row, int count, const QModelIndex & parent)
// {
//     beginInsertRows(parent, row, count);
//
//     endInsertRows();
// }

bool CallHistoryModel::setData(const QModelIndex & index, const QVariant & value, int role)
{

}

QVariant CallHistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(role);
    return tr("History Items");
}

QVariant CallHistoryModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= d->items.count() || index.row() < 0) {
        return QVariant();
    }

    int i = index.row();
    switch (role) {
        case Qt::ToolTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
        case Qt::DisplayRole:
            return d->items.at(i).name + "\n"
                + d->items.at(i).time.toString();
        case Qt::DecorationRole:
            switch (d->items.at(i).status) {
                case CallHistoryItem::Invalid:
                    return QIcon(":/customwidgets/data/callhistory/blank.png");
                case CallHistoryItem::Unknown:
                    return QIcon(":/customwidgets/data/callhistory/blank.png");
                case CallHistoryItem::Sent:
                    return QIcon(":/customwidgets/data/callhistory/sent.png");
                case CallHistoryItem::Received:
                    return QIcon(":/customwidgets/data/callhistory/received.png");
                case CallHistoryItem::Missed:
                    return QIcon(":/customwidgets/data/callhistory/missed.png");
                case CallHistoryItem::Rejected:
                    return QIcon(":/customwidgets/data/callhistory/rejected.png");
            }
        case Qt::SizeHintRole:
            return QSize(100, 48);
        case CallHistoryItem::Id:
            return d->items.at(i).id;
        default:
            return QVariant();
    }

}

Qt::ItemFlags CallHistoryModel::flags(const QModelIndex & index) const
{
    return Qt::ItemIsEditable | QAbstractListModel::flags(index);
}
