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

#include "optionsdialog.h"
#include "ui_optionsdialog_base.h"
#include <QIcon>

class OptionsDialog::Private : public QObject, public Ui::OptionsDialogBase {
    Q_OBJECT
public:
    Private(OptionsDialog * parent);

    OptionsDialog * q;
    PsiCon * controller;

public Q_SLOTS:
    void buttonClicked(QAbstractButton * button);
    void tableNotificationsCurrentCellChanged(int currentRow,
            int currentColumn, int previousRow, int previousColumn);
    void tableNotificationsUpdate();
    void buttonNotificationSoundBrowseClicked();
    void buttonSampleTooltipClicked();

    // void buttonServiceProvidersAddClicked();
    // void buttonServiceProvidersRemoveClicked();

public:
    QIcon iconNotification;
    QIcon iconSound;
    QIcon iconJabbin;
    int   tableNotificationsCurrentRow;
};

