/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *   Copyright (C) 2005 Oleksandr Yakovlyev <me@yshurik.kiev.ua>
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

#include "calldialog.h"
#include "ui_calldialog_base.h"
#include "generic/customwidgetscommon.h"

#include <QRegExpValidator>
#include <QRegExp>
#include <QBasicTimer>
#include <QTime>
#include "im.h"
#include "psiaccount.h"
#include "voicecaller.h"
#include "models/callhistory.h"
#include "models/phonebook.h"

class CallDialog::Private : public QObject, public Ui::CallDialogBase {
    Q_OBJECT
public:
    Private(CallDialog * parent);
    void setStatus(Status value);

    QMap < QPushButton *, char > buttons;
    Status status;
    QBasicTimer timer;
    QTime time;

    PsiAccount * account;
    VoiceCaller * caller;
    Jid jid;
    QString phone;

    PhoneBookModel * phonebook;
    CallHistoryModel * callhistory;
    CallDialog * q;

public Q_SLOTS:
    void doAction(const QString & buttonData);
    void call(const QString & who);

};

