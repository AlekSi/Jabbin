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
#include <QPainter>
#include <QEvent>
#include <QHoverEvent>
#include "ui_callhistoryitemdelegate_base.h"

#define ICON_WIDTH 32
#define EDITOR_WIDTH 32
#define GRADIENT_WIDTH 20
#define EDITOR_COLOR_S "235, 244, 250"
#define EDITOR_COLOR QColor(235, 244, 250)

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
        Id = Qt::UserRole,
        PrettyDisplay = Qt::UserRole + 1
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

class CallHistoryItemEditor: public QWidget, Ui::CallHistoryItemDelegateBase {
public:
    CallHistoryItemEditor(QWidget * parent)
        : QWidget(parent)
    {
        editor = this;
        setupUi(this);
        setVisible(false);
        setStyleSheet(QString() + "QWidget#CallHistoryItemDelegateBase { background: rgb(" + EDITOR_COLOR_S + "); }");
    }

    ~CallHistoryItemEditor()
    {
        editor = NULL;
    }

    static CallHistoryItemEditor * editor;
    QModelIndex editingIndex;

    friend class CallHistoryItemDelegate;
};

CallHistoryItemEditor * CallHistoryItemEditor::editor = NULL;

class CallHistoryModel::Private {
public:
    Private(QListView * m_list)
        : list(m_list)
    {
        new CallHistoryItemEditor(list);
    }

    QList < CallHistoryItem > items;
    QListView * list;
    CallHistoryItemDelegate * delegate;
};

class CallHistoryItemDelegate::Private {
public:
    Private()
    {
        label = new QLabel();
        label->setName("CallHistoryItemDelegateLabel");
        label->setStyleSheet("QLabel { background-color: white; }");
        label->setAutoFillBackground(true);
    }

    ~Private()
    {
        delete label;
    }

    CallHistoryModel * model;
    QLabel * label;
};

CallHistoryItemDelegate::CallHistoryItemDelegate(CallHistoryModel * model)
    : d(new Private())
{
    d->model = model;
}

CallHistoryItemDelegate::~CallHistoryItemDelegate()
{
    delete d;
}

void CallHistoryItemDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option,
            const QModelIndex & index) const
{
    QItemDelegate::paint(painter, option, index);

    d->label->setText(d->model->data(index, CallHistoryItem::PrettyDisplay).toString());
    QRect rect = option.rect;
    rect.setLeft(rect.left() + ICON_WIDTH);

    painter->save();
    painter->translate(option.rect.x(), option.rect.y());

    d->label->resize(option.rect.size());
    d->label->render(painter, QPoint(ICON_WIDTH, 0));

    QLinearGradient gradient;

    if (option.state & QStyle::State_MouseOver || index == CallHistoryItemEditor::editor->editingIndex) {
        gradient = QLinearGradient(option.rect.size().width() - GRADIENT_WIDTH - EDITOR_WIDTH, 0,
                option.rect.size().width() - EDITOR_WIDTH, 0);
        gradient.setColorAt(0, Qt::transparent);
        gradient.setColorAt(1, EDITOR_COLOR);
    } else {
        gradient = QLinearGradient(option.rect.size().width() - GRADIENT_WIDTH, 0,
                option.rect.size().width(), 0);
        gradient.setColorAt(0, Qt::transparent);
        gradient.setColorAt(1, Qt::white);
    }
    painter->fillRect(QRect(QPoint(), option.rect.size()), gradient);

    painter->restore();

}

CallHistoryModel::CallHistoryModel(QListView * parent)
    : QAbstractListModel(parent), d(new Private(parent))
{
    d->items.append(CallHistoryItem("test1 asdoiu aoisdu aoisd uaosi duaosi du", "id1", QDateTime::currentDateTime(), CallHistoryItem::Unknown));
    d->items.append(CallHistoryItem("test2", "id2", QDateTime::currentDateTime(), CallHistoryItem::Received));
    d->items.append(CallHistoryItem("test3", "id3", QDateTime::currentDateTime(), CallHistoryItem::Rejected));
    d->items.append(CallHistoryItem("test2", "id2", QDateTime::currentDateTime(), CallHistoryItem::Sent));
    d->items.append(CallHistoryItem("test3", "id3", QDateTime::currentDateTime(), CallHistoryItem::Missed));
    d->items.append(CallHistoryItem("test2", "id2", QDateTime::currentDateTime(), CallHistoryItem::Received));
    d->items.append(CallHistoryItem("test3", "id3", QDateTime::currentDateTime(), CallHistoryItem::Rejected));
    d->items.append(CallHistoryItem("test2", "id2", QDateTime::currentDateTime(), CallHistoryItem::Received));
    d->items.append(CallHistoryItem("test3", "id3", QDateTime::currentDateTime(), CallHistoryItem::Rejected));
    d->items.append(CallHistoryItem("test2", "id2", QDateTime::currentDateTime(), CallHistoryItem::Received));
    d->items.append(CallHistoryItem("test3", "id3", QDateTime::currentDateTime(), CallHistoryItem::Rejected));
    d->items.append(CallHistoryItem("test2", "id2", QDateTime::currentDateTime(), CallHistoryItem::Received));
    d->items.append(CallHistoryItem("test3", "id3", QDateTime::currentDateTime(), CallHistoryItem::Rejected));
    d->items.append(CallHistoryItem("test2", "id2", QDateTime::currentDateTime(), CallHistoryItem::Received));
    d->items.append(CallHistoryItem("test3", "id3", QDateTime::currentDateTime(), CallHistoryItem::Rejected));
    d->items.append(CallHistoryItem("test2", "id2", QDateTime::currentDateTime(), CallHistoryItem::Received));
    d->items.append(CallHistoryItem("test3", "id3", QDateTime::currentDateTime(), CallHistoryItem::Rejected));
    d->items.append(CallHistoryItem("test2", "id2", QDateTime::currentDateTime(), CallHistoryItem::Received));
    d->items.append(CallHistoryItem("test3", "id3", QDateTime::currentDateTime(), CallHistoryItem::Rejected));
    d->items.append(CallHistoryItem("test3", "id3", QDateTime::currentDateTime(), CallHistoryItem::Rejected));
    d->items.append(CallHistoryItem("test2", "id2", QDateTime::currentDateTime(), CallHistoryItem::Received));
    d->items.append(CallHistoryItem("test3", "id3", QDateTime::currentDateTime(), CallHistoryItem::Rejected));

    d->list->setModel(this);
    d->list->installEventFilter(this);
    d->list->setItemDelegate(d->delegate = new CallHistoryItemDelegate(this));
}

CallHistoryModel::~CallHistoryModel()
{
    delete d;
}

bool CallHistoryModel::eventFilter(QObject * obj, QEvent * event)
{
    if (obj == d->list && event->type() == QEvent::HoverMove) {
        QHoverEvent * mouseEvent = static_cast<QHoverEvent *>(event);
        QModelIndex index = d->list->indexAt(mouseEvent->pos());
        if (index != CallHistoryItemEditor::editor->editingIndex) {
            CallHistoryItemEditor::editor->editingIndex = index;
            QRect rect = d->list->visualRect(index);
            rect.setLeft(rect.right() - EDITOR_WIDTH);
            CallHistoryItemEditor::editor->setGeometry(rect);
            CallHistoryItemEditor::editor->setVisible(true);
        }
    } else if (obj == d->list && event->type() == QEvent::HoverLeave) {
        CallHistoryItemEditor::editor->setVisible(false);
        CallHistoryItemEditor::editor->editingIndex = QModelIndex();
    }
    return QObject::eventFilter(obj, event);
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

    QString what;
    int i = index.row();
    switch (role) {
        case Qt::WhatsThisRole:
            return QVariant();
        case Qt::DisplayRole:
            return d->items.at(i).name + " ("
                + d->items.at(i).time.toString() + ")";
        case CallHistoryItem::PrettyDisplay:
            return "<span style=\"font-size: 12pt; font-weight:bold;\">" + d->items.at(i).name + "</span><br>\n"
                + "<span style=\"font-size: 8pt; color: grey;\">" + d->items.at(i).time.toString() + "</span>";
        case Qt::DecorationRole:
            switch (d->items.at(i).status) {
                case CallHistoryItem::Invalid:
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
        case Qt::ToolTipRole:
            switch (d->items.at(i).status) {
                case CallHistoryItem::Invalid:
                case CallHistoryItem::Unknown:
                    return QString();
                case CallHistoryItem::Sent:
                    what = tr("You have called %1 on %2");
                    break;
                case CallHistoryItem::Received:
                    what = tr("You have received a call from %1 on %2");
                    break;
                case CallHistoryItem::Missed:
                    what = tr("You have missed a call from %1 on %2");
                    break;
                case CallHistoryItem::Rejected:
                    what = tr("You have rejected a call from %1 on %2");
                    break;
            }
            return what.arg(d->items.at(i).name, d->items.at(i).time.toString());
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
