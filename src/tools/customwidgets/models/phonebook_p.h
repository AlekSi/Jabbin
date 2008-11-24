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

#ifndef PHONEBOOK_P_H
#define PHONEBOOK_P_H

#include <QModelIndex>
#include <QDialog>
#include "phonebook.h"
#include "ui_phonebookitemdelegate_base.h"
#include "ui_phonebookedit_base.h"

class PhoneBookEditorForm;

class PhoneBookItem {
public:
    enum Roles {
        PrettyDisplay = Qt::UserRole,
        Name = Qt::UserRole + 1,
        Gender = Qt::UserRole + 2,
        PhoneList = Qt::UserRole + 3
    };

    PhoneBookItem(QString _name = QString(),
            PhoneBookModel::Gender _gender = PhoneBookModel::Unknown,
            PhoneBookModel::PhoneList _phones = PhoneBookModel::PhoneList()
            );

    static PhoneBookItem fromVariant(const QVariant & data);
    void loadFromVariant(const QVariant & data);
    QVariant toVariant() const;

    // Name of the contact
    QString name;

    // Gender
    PhoneBookModel::Gender gender;

    // Phones
    PhoneBookModel::PhoneList phones;
};

class PhoneBookItemEditor: public QWidget, Ui::PhoneBookItemDelegateBase {
    Q_OBJECT
public:
    PhoneBookItemEditor(QWidget * parent, QAbstractItemModel * model);

    ~PhoneBookItemEditor();

    QModelIndex editingIndex;

    PhoneBookEditorForm * editorForm;

public Q_SLOTS:
    void call();
    void edit();
    void add(const QString & name = QString(), const QString & phone = QString());
    void del();
    void save();

private:
    QAbstractItemModel * m_model;

    friend class PhoneBookItemDelegate;
};

class PhoneBookEditorForm: public QDialog, Ui::PhoneBookEditBase {
    Q_OBJECT
public:
    PhoneBookEditorForm();
    ~PhoneBookEditorForm();

    friend class PhoneBookItemEditor;
};

#endif // PHONEBOOK_P_H

