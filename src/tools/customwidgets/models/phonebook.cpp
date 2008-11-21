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
#include "phonebook_p.h"
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
#include <QMessageBox>

#define ICON_WIDTH 32
#define EDITOR_WIDTH 32
#define GRADIENT_WIDTH 20
#define EDITOR_COLOR_S "235, 244, 250"
#define EDITOR_COLOR QColor(235, 244, 250)

#define PHONEBOOKFILE ApplicationInfo::homeDir() + "/phonebook.xml"

// PhoneBookItem
PhoneBookItem::PhoneBookItem(QString _name, PhoneBookModel::Gender _gender,
        PhoneBookModel::PhoneList _phones)
    : name(_name), gender(_gender), phones(_phones)
{
}

// PhoneBookEditorForm
PhoneBookEditorForm::PhoneBookEditorForm()
{
    setupUi(this);
    setModal(true);

    connect(buttons, SIGNAL(accepted()), this, SLOT(close()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(close()));
}

PhoneBookEditorForm::~PhoneBookEditorForm()
{
}

// PhoneBookItemEditor
PhoneBookItemEditor::PhoneBookItemEditor(QWidget * parent, PhoneBookModel * model)
    : QWidget(parent), m_model(model)
{
    setupUi(this);
    setVisible(false);
    setStyleSheet(QString() + "QWidget#PhoneBookItemDelegateBase { background: rgb(" + EDITOR_COLOR_S + "); }");

    editorForm = new PhoneBookEditorForm();
    connect(editorForm->buttons, SIGNAL(accepted()), this, SLOT(save()));

    connect(buttonCall,   SIGNAL(clicked()), this, SLOT(call()));
    connect(buttonDelete, SIGNAL(clicked()), this, SLOT(del()));
    connect(buttonEdit,   SIGNAL(clicked()), this, SLOT(edit()));
}

PhoneBookItemEditor::~PhoneBookItemEditor()
{
    delete editorForm;
}

void PhoneBookItemEditor::save()
{
    QString name;
    PhoneBookModel::Gender gender;
    PhoneBookModel::PhoneList phones;

    name = editorForm->editName->text();

    if (editorForm->radioFemale->isChecked()) {
        gender = PhoneBookModel::Female;
    } else if (editorForm->radioMale->isChecked()) {
        gender = PhoneBookModel::Male;
    }

    #define SAVE_PHONE(Type, Edit) if (Edit->text() != QString()) phones[Type] = Edit->text()
    SAVE_PHONE(PhoneBookModel::CellPhone, editorForm->editCellPhone);
    SAVE_PHONE(PhoneBookModel::LandLine,  editorForm->editLandLine);
    SAVE_PHONE(PhoneBookModel::Office,    editorForm->editOffice);
    #undef SAVE_PHONE

    m_model->setItemData(editingIndex.row(), name, gender, phones);
}

void PhoneBookItemEditor::call()
{

}

void PhoneBookItemEditor::edit()
{
    QString name;
    PhoneBookModel::Gender gender;
    PhoneBookModel::PhoneList phones;
    m_model->getItemData(editingIndex.row(), name, gender, phones);

    editorForm->editName->setText(name);

    editorForm->radioMale->setChecked(false);
    editorForm->radioFemale->setChecked(false);
    switch (gender) {
        case PhoneBookModel::Male:
            editorForm->radioMale->setChecked(true);
            break;
        case PhoneBookModel::Female:
            editorForm->radioFemale->setChecked(true);
            break;
        default:
            break;
    }

    #define LOAD_PHONE(Type, Edit) Edit->setText( ( phones.contains(Type) )? phones[Type] : "" )
    LOAD_PHONE(PhoneBookModel::CellPhone, editorForm->editCellPhone);
    LOAD_PHONE(PhoneBookModel::LandLine,  editorForm->editLandLine);
    LOAD_PHONE(PhoneBookModel::Office,    editorForm->editOffice);
    #undef LOAD_PHONE

    editorForm->show();
}

void PhoneBookItemEditor::del()
{
    if (QMessageBox::question(this, tr("Are you sure?"),
            tr("Are you sure you want to delete this contact?"),
            QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Yes) {
        m_model->deleteItem(editingIndex.row());
    }
}

// PhoneBookModel::Private
class PhoneBookModel::Private {
public:
    Private(QListView * _list, PhoneBookModel * _model)
        : list(_list)
    {
        editor = new PhoneBookItemEditor(_list, _model);
    }

    QList < PhoneBookItem > items;
    QListView * list;
    PhoneBookItemDelegate * delegate;
    PhoneBookItemEditor * editor;
};

// PhoneBookItemDelegate::Private
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

// PhoneBookItemDelegate
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

    if (option.state & QStyle::State_MouseOver || index ==  d->model->d->editor->editingIndex) {
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

// PhoneBookModel
PhoneBookModel::PhoneBookModel(QListView * parent)
    : QAbstractListModel(parent), d(new Private(parent, this))
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
        if (index != d->editor->editingIndex) {
            d->editor->editingIndex = index;
            QRect rect = d->list->visualRect(index);
            rect.setLeft(rect.right() - EDITOR_WIDTH);
            d->editor->setGeometry(rect);
            d->editor->setVisible(true);
        }
    } else if (obj == d->list && event->type() == QEvent::HoverLeave) {
        d->editor->setVisible(false);
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

void PhoneBookModel::setItemData(int i, const QString & name,
        Gender gender,
        const PhoneBookModel::PhoneList & phones
        )
{
    deleteItem(i);
    addItem(name, gender, phones);
}

void PhoneBookModel::getItemData(int index, QString & name,
        Gender & gender,
        PhoneBookModel::PhoneList & phones
        )
{
    name = d->items.at(index).name;
    gender = d->items.at(index).gender;
    phones = d->items.at(index).phones;
}

void PhoneBookModel::deleteItem(int index)
{
    beginRemoveRows(QModelIndex(), index, index);
    d->items.removeAt(index);
    endRemoveRows();
}

