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
#include "applicationinfo.h"

#include <QTime>
#include <QDebug>
#include <QSize>
#include <QIcon>
#include <QFile>
#include <QItemDelegate>
#include <QPainter>
#include <QEvent>
#include <QHoverEvent>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include "ui_phonebookitemdelegate_base.h"

#define ICON_WIDTH 32
#define EDITOR_WIDTH 32
#define GRADIENT_WIDTH 20
#define EDITOR_COLOR_S "235, 244, 250"
#define EDITOR_COLOR QColor(235, 244, 250)

#define PHONEBOOKFILE ApplicationInfo::homeDir() + "/phonebook.xml"

class PhoneBookItem {
public:
    enum Roles {
        PrettyDisplay = Qt::UserRole
    };

    PhoneBookItem(QString _name = QString(),
            PhoneBookModel::Gender _gender = PhoneBookModel::Unknown,
            PhoneBookModel::PhoneList _phones = PhoneBookModel::PhoneList()
            )
        : name(_name), gender(_gender), phones(_phones)
    {
    }

    // Name of the contact
    QString name;

    // Gender
    PhoneBookModel::Gender gender;

    // Phones
    PhoneBookModel::PhoneList phones;

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
    load();

    addItem("Ana",   Female, PhoneList());
    addItem("Zarko", Male, PhoneList());
    addItem("Rista", Male, PhoneList());
    addItem("Anica", Female, PhoneList());

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
            return d->items.at(i).name;
        case PhoneBookItem::PrettyDisplay:
            return "<span style=\"font-size: 12pt; font-weight:bold;\">" + d->items.at(i).name + "</span>\n";
        case Qt::DecorationRole:
            switch (d->items.at(i).gender) {
                case Unknown:
                    return QIcon(":/customwidgets/data/phonebook/undefined.png");
                case Male:
                    return QIcon(":/customwidgets/data/phonebook/male.png");
                case Female:
                    return QIcon(":/customwidgets/data/phonebook/female.png");
            }
        case Qt::ToolTipRole:
            return QString();
        case Qt::SizeHintRole:
            return QSize(100, 48);
        default:
            return QVariant();
    }

}

Qt::ItemFlags PhoneBookModel::flags(const QModelIndex & index) const
{
    return Qt::ItemIsEditable | QAbstractListModel::flags(index);
}

void PhoneBookModel::load()
{
    QFile file(PHONEBOOKFILE);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QXmlStreamReader reader(&file);

    if (reader.atEnd()) {
        return;
    }

    reader.readNext();
    if (reader.isStartElement()) {
        if (reader.name() != "phonebook" || reader.attributes().value("version") != "1.0") {
            reader.raiseError(QObject::tr("The file is not an phonebook version 1.0 file."));
            return;
        }
    }

    PhoneBookItem item;
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement()) {
            if (reader.name() == "contact") {
                item.name = reader.attributes().value("name").toString();
                item.gender = (Gender) reader.attributes().value("gender").toString().toInt();

            } else if (reader.name() == "phone") {
                item.phones[
                    (PhoneType)
                    reader.attributes().value("type").toString().toInt()
                    ] = reader.attributes().value("value").toString();
            }
        } else if (reader.isEndElement()) {
            if (reader.name() == "contact") {
                d->items.append(item);
            }
        }
    }
}

void PhoneBookModel::save()
{
    QFile file(PHONEBOOKFILE);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);

    writer.writeStartDocument();
    writer.writeDTD("<!DOCTYPE joimphonebook>");
    writer.writeStartElement("phonebook");
    writer.writeAttribute("version", "1.0");

    foreach (PhoneBookItem item, d->items) {
        writer.writeStartElement("contact");
        writer.writeAttribute("name",    item.name);
        writer.writeAttribute("gender",  QString::number(item.gender));

        QMapIterator < PhoneType, QString > i(item.phones);
        while (i.hasNext()) {
            i.next();
            writer.writeStartElement("phone");
            writer.writeAttribute("type", QString::number(i.key()));
            writer.writeAttribute("value", i.value());
            writer.writeEndElement();
        }
        writer.writeEndElement();
    }

    writer.writeEndElement();
    writer.writeEndDocument();
}

void PhoneBookModel::addItem(const QString & name,
        Gender gender,
        const PhoneBookModel::PhoneList & phones
        )
{
    // we want to maintain the list sorted
    int i;
    for (i = 0; i < d->items.count(); i++) {
        if (d->items.at(i).name > name) {
            break;
        }
    }

    beginInsertRows(QModelIndex(), i, i);
    d->items.insert(i, PhoneBookItem(name, gender, phones));
    endInsertRows();
}

void PhoneBookModel::setItemData(int index, const QString & name,
        Gender gender,
        const PhoneBookModel::PhoneList & phones
        )
{
    d->items.removeAt(index);
    d->items.insert(index, PhoneBookItem(name, gender, phones));
}

void PhoneBookModel::getItemData(int index, QString & name,
        Gender gender,
        PhoneBookModel::PhoneList & phones
        )
{

}

void PhoneBookModel::deleteItem(int index)
{

}

