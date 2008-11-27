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

#ifndef PHONEBOOK_H
#define PHONEBOOK_H

#include <QAbstractListModel>
#include <QItemDelegate>
#include <QListView>
#include <QLabel>

class PhoneBookModel;

class PhoneBookItemDelegate: public QItemDelegate {
public:
    PhoneBookItemDelegate(QAbstractItemModel * model);
    ~PhoneBookItemDelegate();

    void paint(QPainter * painter, const QStyleOptionViewItem & option,
            const QModelIndex & index) const;

private:
    class Private;
    Private * const d;
};

class PhoneBookModel: public QAbstractListModel {
    Q_OBJECT
public:
    enum PhoneType {
        CellPhone,
        LandLine,
        Office
    };

    enum Gender {
        Unknown = 0,
        Male,
        Female
    };

    typedef QMap < PhoneType, QString > PhoneList;

    PhoneBookModel(QListView * list = NULL, QLineEdit * filter = NULL);
    ~PhoneBookModel();

    // Override
    int rowCount(const QModelIndex & parent = QModelIndex()) const;

    // Override
    int columnCount(const QModelIndex & parent = QModelIndex()) const;

    // Override
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    // Override
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    // Override
    Qt::ItemFlags flags(const QModelIndex & index) const;

    bool eventFilter(QObject * obj, QEvent * event);

    bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex());
    bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex());
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

public Q_SLOTS:
    /** opens a dialog for adding a contact */
    void addContact(const QString & name = QString(), const QString & phone = QString());
    void call(const QString & who);

Q_SIGNALS:
    /**
     * Call is requested.
     * @param who a jabber id or phone number (if phone number,
     * it is in format phone://phone-number
     */
    void callRequested(const QString & who);

protected:
    void load();
    void save();

private:
    class Private;
    Private * const d;

    friend class PhoneBookItemDelegate;
};

#endif // PHONEBOOK_H

