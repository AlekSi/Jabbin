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

#include "servicespanel.h"
#include "ui_servicespanel_base.h"
#include <QModelIndex>

class ServicesModel;

class ServicesPanel::Private : public QObject, public Ui::ServicesPanelBase {
    Q_OBJECT
public:
    Private(ServicesPanel * parent);

    PsiAccount * account;
    ServicesModel * model;
    Jid jid;

    ServicesPanel * q;
    QModelIndex clickedItem;
    Qt::MouseButton clickedWithButton;

public Q_SLOTS:
    void itemClicked(const QModelIndex & index);
    bool eventFilter(QObject * object, QEvent * event);

    void serviceLogout();
    void serviceLogin();

    void joinService();
    void leaveService();
    void reloadItem();
};

