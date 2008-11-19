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

#include "phonebook.h"
#include <QTime>
#include <QDebug>
#include <QSize>
#include <QIcon>
#include <QItemDelegate>
#include <QPainter>
#include <QEvent>
#include <QHoverEvent>
#include "ui_phonebookitemdelegate_base.h"

#define ICON_WIDTH 32
#define EDITOR_WIDTH 32
#define GRADIENT_WIDTH 20
#define EDITOR_COLOR_S "235, 244, 250"
#define EDITOR_COLOR QColor(235, 244, 250)

class PhoneBookItem {
public:
    enum Gender {
        Unknown = 0,
        Male,
        Female
    };

    enum Roles {
        Id = Qt::UserRole,
        PrettyDisplay = Qt::UserRole + 1
    };

    PhoneBookItem()
        : name(QString()), id(QString()), time(QDateTime()), gender(Unknown)
    {

    }

    PhoneBookItem(QString _name, QString _id, QDateTime _time, Gender _gender)
        : name(_name), id(_id), time(_time), gender(_gender)
    {

    }

    // Name of the caller
    QString name;

    // Jabber ID or phone number of the caller
    QString id;

    // Timestamp
    QDateTime time;

    // Gender
    Gender gender;

};

class PhoneBookItemEditor: public QWidget, Ui::PhoneBookItemDelegateBase {
public:
    PhoneBookItemEditor(QWidget * parent)
        : QWidget(parent)
    {
        editor = this;
        setupUi(this);
        setVisible(false);
        setStyleSheet(QString() + "QWidget#PhoneBookItemDelegateBase { background: rgb(" + EDITOR_COLOR_S + "); }");
    }

    ~PhoneBookItemEditor()
    {
        editor = NULL;
    }

    static PhoneBookItemEditor * editor;
    QModelIndex editingIndex;

    friend class PhoneBookItemDelegate;
};

PhoneBookItemEditor * PhoneBookItemEditor::editor = NULL;

class PhoneBookModel::Private {
public:
    Private(QListView * m_list)
        : list(m_list)
    {
        new PhoneBookItemEditor(list);
    }

    QList < PhoneBookItem > items;
    QListView * list;
    PhoneBookItemDelegate * delegate;
};

class PhoneBookItemDelegate::Private {
public:
    Private()
    {
        label = new QLabel();
        label->setName("PhoneBookItemDelegateLabel");
        label->setStyleSheet("QLabel { background-color: white; }");
        label->setAutoFillBackground(true);
    }

    ~Private()
    {
        delete label;
    }

    PhoneBookModel * model;
    QLabel * label;
};

PhoneBookItemDelegate::PhoneBookItemDelegate(PhoneBookModel * model)
    : d(new Private())
{
    d->model = model;
}

PhoneBookItemDelegate::~PhoneBookItemDelegate()
{
    delete d;
}

void PhoneBookItemDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option,
            const QModelIndex & index) const
{
    QItemDelegate::paint(painter, option, index);

    d->label->setText(d->model->data(index, PhoneBookItem::PrettyDisplay).toString());
    QRect rect = option.rect;
    rect.setLeft(rect.left() + ICON_WIDTH);

    painter->save();
    painter->translate(option.rect.x(), option.rect.y());

    d->label->resize(option.rect.size());
    d->label->render(painter, QPoint(ICON_WIDTH, 0));

    QLinearGradient gradient;

    if (option.state & QStyle::State_MouseOver || index == PhoneBookItemEditor::editor->editingIndex) {
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

PhoneBookModel::PhoneBookModel(QListView * parent)
    : QAbstractListModel(parent), d(new Private(parent))
{
    d->items.append(PhoneBookItem("test1 asdoiu aoisdu aoisd uaosi duaosi du", "id1", QDateTime::currentDateTime(), PhoneBookItem::Unknown));
    d->items.append(PhoneBookItem("test2", "id2", QDateTime::currentDateTime(), PhoneBookItem::Male));
    d->items.append(PhoneBookItem("test3", "id3", QDateTime::currentDateTime(), PhoneBookItem::Female));
    d->items.append(PhoneBookItem("test2", "id2", QDateTime::currentDateTime(), PhoneBookItem::Male));
    d->items.append(PhoneBookItem("test3", "id3", QDateTime::currentDateTime(), PhoneBookItem::Female));
    d->items.append(PhoneBookItem("test2", "id2", QDateTime::currentDateTime(), PhoneBookItem::Male));
    d->items.append(PhoneBookItem("test3", "id3", QDateTime::currentDateTime(), PhoneBookItem::Female));
    d->items.append(PhoneBookItem("test2", "id2", QDateTime::currentDateTime(), PhoneBookItem::Male));
    d->items.append(PhoneBookItem("test3", "id3", QDateTime::currentDateTime(), PhoneBookItem::Male));
    d->items.append(PhoneBookItem("test2", "id2", QDateTime::currentDateTime(), PhoneBookItem::Male));
    d->items.append(PhoneBookItem("test3", "id3", QDateTime::currentDateTime(), PhoneBookItem::Male));
    d->items.append(PhoneBookItem("test2", "id2", QDateTime::currentDateTime(), PhoneBookItem::Male));
    d->items.append(PhoneBookItem("test3", "id3", QDateTime::currentDateTime(), PhoneBookItem::Male));
    d->items.append(PhoneBookItem("test2", "id2", QDateTime::currentDateTime(), PhoneBookItem::Male));
    d->items.append(PhoneBookItem("test3", "id3", QDateTime::currentDateTime(), PhoneBookItem::Male));
    d->items.append(PhoneBookItem("test2", "id2", QDateTime::currentDateTime(), PhoneBookItem::Male));
    d->items.append(PhoneBookItem("test3", "id3", QDateTime::currentDateTime(), PhoneBookItem::Male));
    d->items.append(PhoneBookItem("test3", "id3", QDateTime::currentDateTime(), PhoneBookItem::Male));
    d->items.append(PhoneBookItem("test2", "id2", QDateTime::currentDateTime(), PhoneBookItem::Male));
    d->items.append(PhoneBookItem("test3", "id3", QDateTime::currentDateTime(), PhoneBookItem::Male));

    d->list->setModel(this);
    d->list->installEventFilter(this);
    d->list->setItemDelegate(d->delegate = new PhoneBookItemDelegate(this));
}

PhoneBookModel::~PhoneBookModel()
{
    delete d;
}

bool PhoneBookModel::eventFilter(QObject * obj, QEvent * event)
{
    if (obj == d->list && event->type() == QEvent::HoverMove) {
        QHoverEvent * mouseEvent = static_cast<QHoverEvent *>(event);
        QModelIndex index = d->list->indexAt(mouseEvent->pos());
        if (index != PhoneBookItemEditor::editor->editingIndex) {
            PhoneBookItemEditor::editor->editingIndex = index;
            QRect rect = d->list->visualRect(index);
            rect.setLeft(rect.right() - EDITOR_WIDTH);
            PhoneBookItemEditor::editor->setGeometry(rect);
            PhoneBookItemEditor::editor->setVisible(true);
        }
    } else if (obj == d->list && event->type() == QEvent::HoverLeave) {
        PhoneBookItemEditor::editor->setVisible(false);
        PhoneBookItemEditor::editor->editingIndex = QModelIndex();
    }
    return QObject::eventFilter(obj, event);
}

int PhoneBookModel::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);
    return d->items.count();
}

int PhoneBookModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant PhoneBookModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(role);
    return tr("History Items");
}

QVariant PhoneBookModel::data(const QModelIndex & index, int role) const
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
        case PhoneBookItem::PrettyDisplay:
            return "<span style=\"font-size: 12pt; font-weight:bold;\">" + d->items.at(i).name + "</span><br>\n"
                + "<span style=\"font-size: 8pt; color: grey;\">" + d->items.at(i).time.toString() + "</span>";
        case Qt::DecorationRole:
            switch (d->items.at(i).gender) {
                case PhoneBookItem::Unknown:
                    return QIcon(":/customwidgets/data/phonebook/undefined.png");
                case PhoneBookItem::Male:
                    return QIcon(":/customwidgets/data/phonebook/male.png");
                case PhoneBookItem::Female:
                    return QIcon(":/customwidgets/data/phonebook/female.png");
            }
        case Qt::ToolTipRole:
            return QString();
        case Qt::SizeHintRole:
            return QSize(100, 48);
        case PhoneBookItem::Id:
            return d->items.at(i).id;
        default:
            return QVariant();
    }

}

Qt::ItemFlags PhoneBookModel::flags(const QModelIndex & index) const
{
    return Qt::ItemIsEditable | QAbstractListModel::flags(index);
}
