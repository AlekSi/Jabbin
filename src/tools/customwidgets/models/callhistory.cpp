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
#include "callhistory_p.h"
#include "applicationinfo.h"
#include "calldialog.h"
#include "psicontact.h"

#include <QDebug>
#include <QSize>
#include <QIcon>
#include <QItemDelegate>
#include <QPainter>
#include <QEvent>
#include <QFile>
#include <QHoverEvent>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QMessageBox>

#define ICON_WIDTH 32
#define EDITOR_WIDTH 32
#define GRADIENT_WIDTH 20
#define EDITOR_COLOR_S "235, 244, 250"
#define EDITOR_COLOR QColor(235, 244, 250)

#define CALLHISTORYFILE ApplicationInfo::homeDir() + "/callhistory.xml"

// CallHistoryItem
CallHistoryItem::CallHistoryItem()
    : name(QString()), id(QString()), time(QDateTime()), status(CallHistoryModel::Invalid)
{

}

CallHistoryItem::CallHistoryItem(QString _name, QString _id, QDateTime _time, CallHistoryModel::Status _status)
    : name(_name), id(_id), time(_time), status(_status)
{

}

// CallHistoryItemEditor
CallHistoryItemEditor::CallHistoryItemEditor(QWidget * parent, QAbstractItemModel * model)
    : QWidget(parent), m_model(model)
{
    editor = this;
    setupUi(this);
    setVisible(false);
    setStyleSheet(QString() + "QWidget#CallHistoryItemDelegateBase { background: rgb(" + EDITOR_COLOR_S + "); }");

    connect(buttonCall,        SIGNAL(clicked()), this, SLOT(call()));
    connect(buttonDelete,      SIGNAL(clicked()), this, SLOT(del()));
    connect(buttonToPhoneBook, SIGNAL(clicked()), this, SLOT(toPhoneBook()));
    buttonToPhoneBook->hide();
}

CallHistoryItemEditor::~CallHistoryItemEditor()
{
    editor = NULL;
}

CallHistoryItemEditor * CallHistoryItemEditor::editor = NULL;

void CallHistoryItemEditor::call()
{
    QString id = m_model->data(editingIndex, CallHistoryItem::Id).toString();
    ((CallHistoryModel *)m_model)->call(id);
}

void CallHistoryItemEditor::del()
{
    if (QMessageBox::question(this, tr("Are you sure?"),
            tr("Are you sure you want to delete this contact?"),
            QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Yes) {
        m_model->removeRow(editingIndex.row());
    }
}

void CallHistoryItemEditor::toPhoneBook()
{
    QString id = m_model->data(editingIndex, CallHistoryItem::Id).toString();
    qDebug() << "CallHistoryItemEditor::toPhoneBook() : ID is " << id;
    foreach (PsiContact * contact, CallDialog::contactList->contacts()) {
    }

}

// CallHistoryModel::Private
class CallHistoryModel::Private {
public:
    Private(QListView * m_list, QAbstractItemModel * m_model)
        : list(m_list), account(NULL)
    {
        new CallHistoryItemEditor(list, m_model);
    }

    QList < CallHistoryItem > items;
    QListView * list;
    PsiAccount * account;
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
    : QAbstractListModel(parent), d(new Private(parent, this))
{
    load();

    d->list->setModel(this);
    d->list->installEventFilter(this);
    d->list->setItemDelegate(d->delegate = new CallHistoryItemDelegate(this));
}

CallHistoryModel::~CallHistoryModel()
{
    delete d;
}

void CallHistoryModel::call(const QString & who)
{
    emit callRequested(who);
}

void CallHistoryModel::init(PsiAccount * account)
{
    d->account = account;
}

void CallHistoryModel::addEntry(const QString & name, const QString & id,
        Status status, QDateTime time)
{
    if (time.isNull()) {
        time = QDateTime::currentDateTime();
    }

    int row = 0;
    beginInsertRows(QModelIndex(), row, row);
    d->items.prepend(CallHistoryItem(name, id, time, status));
    endInsertRows();
    emit modelEmpty(d->items.size() == 0);
    save();
}

bool CallHistoryModel::removeRows(int row, int count, const QModelIndex & parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    while (count--) {
        d->items.removeAt(row);
    }
    endRemoveRows();
    save();
    emit modelEmpty(d->items.size() == 0);
    return true;
}

void CallHistoryModel::clear()
{
    if (d->items.count() == 0) {
        QMessageBox::information(d->list, tr("Information"), tr("Call history is already empty"));
        return;
    }
    if (QMessageBox::question(d->list, tr("Are you sure?"),
            tr("Are you sure you want to clear the call history?"),
            QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Yes) {
        removeRows(0, d->items.count(), QModelIndex());
    }
}

void CallHistoryModel::updateLastEntryStatus(Status status) {
    if (d->items.count() == 0) {
        return;
    }
    d->items.first().status = status;

    QModelIndex i = index(0);
    dataChanged(i, i);
    save();
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
                case Invalid:
                case Unknown:
                    return QIcon(":/customwidgets/data/callhistory/blank.png");
                case Sent:
                    return QIcon(":/customwidgets/data/callhistory/sent.png");
                case Received:
                    return QIcon(":/customwidgets/data/callhistory/received.png");
                case Missed:
                    return QIcon(":/customwidgets/data/callhistory/missed.png");
                case Rejected:
                    return QIcon(":/customwidgets/data/callhistory/rejected.png");
            }
        case Qt::ToolTipRole:
            switch (d->items.at(i).status) {
                case Invalid:
                case Unknown:
                    return QString();
                case Sent:
                    what = tr("You have called %1 on %2");
                    break;
                case Received:
                    what = tr("You have received a call from %1 on %2");
                    break;
                case Missed:
                    what = tr("You have missed a call from %1 on %2");
                    break;
                case Rejected:
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

void CallHistoryModel::load()
{
    QFile file(CALLHISTORYFILE);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QXmlStreamReader reader(&file);

    if (reader.atEnd()) {
        return;
    }

    reader.readNext();
    if (reader.isStartElement()) {
        if (reader.name() != "callhistory" || reader.attributes().value("version") != "1.0") {
            reader.raiseError(QObject::tr("The file is not an callhistory version 1.0 file."));
            return;
        }
    }

    CallHistoryItem item;
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement()) {
            if (reader.name() == "event") {
                item.name = reader.attributes().value("name").toString();
                item.id = reader.attributes().value("id").toString();
                qDebug() << "CallHistoryModel::load()" << item.name << item.id;
                item.time = QDateTime::fromString((reader.attributes().value("time").toString()), Qt::ISODate);
                item.status = (Status) reader.attributes().value("status").toString().toInt();
                d->items.append(item);
            }
        }
    }
    emit modelEmpty(d->items.size() == 0);
}

void CallHistoryModel::save()
{
    QFile file(CALLHISTORYFILE);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);

    writer.writeStartDocument();
    writer.writeDTD("<!DOCTYPE jabbincallhistory>");
    writer.writeStartElement("callhistory");
    writer.writeAttribute("version", "1.0");

    foreach (CallHistoryItem item, d->items) {
        writer.writeStartElement("event");
        writer.writeAttribute("name",    item.name);
        writer.writeAttribute("id",      item.id);
        writer.writeAttribute("time",    item.time.toString(Qt::ISODate));
        writer.writeAttribute("status",  QString::number(item.status));
        writer.writeEndElement();
    }

    writer.writeEndElement();
    writer.writeEndDocument();
}

