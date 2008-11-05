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
#include "calldialog_p.h"
#include "generic/customwidgetscommon.h"

#include <QRegExpValidator>
#include <QRegExp>
#include <QDebug>

#define JIDTEXT jid.full()

CallDialog::Private::Private(CallDialog * parent)
    : status(Normal), account(NULL), caller(NULL)
{
    setupUi(parent);
    frameVolumes->hide();
    editPhoneNumber->setEmptyText(tr("Enter phone number"));
    editPhoneNumber->setValidator(new QRegExpValidator(QRegExp("[+]?[0-9*#]*"), parent));

    connect(dialpad, SIGNAL(buttonClicked(char)),
            parent, SLOT(dialpadButtonClicked(char)));
    stacked->setCurrentWidget(pageDialpad);

    frameInCall->addButton("Hangup", QIcon("help"), QString());
    frameInCall->setPixmap(QPixmap(":/customwidgets/data/phone.png"));

    frameIncomingCall->addButton(tr("Accept"), QIcon("help"), QString());
    frameIncomingCall->addButton(tr("Reject"), QIcon("help"), QString());
    frameIncomingCall->setPixmap(QPixmap(":/customwidgets/data/phone.png"));
    stacked->setCurrentIndex(0);

    time.start();
    timer.start(1000, parent);
}

void CallDialog::Private::setStatus(Status value)
{
    qDebug() << "Status changed to: " << value;
    status = value;

    switch (status) {
        case Normal:
            timer.stop();
            break;
        case Calling:
            frameInCall->setTitle(tr("Calling ..."));
            frameInCall->setMessage(JIDTEXT);
            caller->call(jid);
            break;
        case InCall:
            frameInCall->setTitle(QString());
            frameInCall->setMessage(JIDTEXT);
            time.start();
            timer.start(1000, this);
            break;
        case Terminating:
            frameInCall->setTitle(tr("Ending call ..."));
            frameInCall->setMessage(JIDTEXT);
            caller->terminate(jid);
            break;
        case Incoming:
            frameIncomingCall->setTitle(tr("Incoming call"));
            frameIncomingCall->setMessage(JIDTEXT);
            break;
        case Accepting:
            frameIncomingCall->setTitle(tr("Starting call..."));
            frameIncomingCall->setMessage(JIDTEXT);
            caller->accept(jid);
            break;
        case Rejecting:
            frameIncomingCall->setTitle(tr("Rejecting call..."));
            frameIncomingCall->setMessage(JIDTEXT);
            caller->reject(jid);
            break;
    }

    switch (status) {
        case Normal:
            stacked->setCurrentWidget(pageDialpad);
            break;
        case Calling:
        case InCall:
        case Terminating:
            stacked->setCurrentWidget(pageInCall);
            break;
        case Incoming:
        case Accepting:
        case Rejecting:
            stacked->setCurrentWidget(pageIncomingCall);
            break;
    }

}

void CallDialog::Private::accepted(const Jid & jid)
{
    setStatus(InCall);
}

void CallDialog::Private::rejected(const Jid & jid)
{
    setStatus(Normal);
}

void CallDialog::Private::terminated(const Jid & jid)
{
    setStatus(Normal);
}

void CallDialog::Private::inProgress(const Jid & jid)
{
    setStatus(InCall);
}

void CallDialog::Private::clicked(const QString & buttonData)
{
    qDebug() << buttonData;
}

CallDialog * CallDialog::m_instance = NULL;

CallDialog * CallDialog::instance()
{
    return m_instance;
}

void CallDialog::init(const Jid & jid, PsiAccount * account, VoiceCaller * caller)
{
    d->jid = jid;
    d->account = account;

    if (caller != d->caller) {
        if (d->caller) {
            disconnect(caller, NULL, d, NULL);
        }

        d->caller = caller;

        connect(caller, SIGNAL(accepted(const Jid & )),
                d,      SLOT(accepted(const Jid & )));
        connect(caller, SIGNAL(rejected(const Jid & )),
                d,      SLOT(rejected(const Jid & )));
        connect(caller, SIGNAL(terminated(const Jid & )),
                d,      SLOT(terminated(const Jid & )));
        connect(caller, SIGNAL(in_progress(const Jid & )),
                d, SLOT(inProgress(const Jid & )));
    }
}

CallDialog::CallDialog(QWidget *parent)
    : QFrame(parent), d(new Private(this))
{
    m_instance = this;
}

CallDialog::~CallDialog()
{
    delete d;
}

void CallDialog::dialpadButtonClicked(char symbol) {
    d->editPhoneNumber->setText(
        d->editPhoneNumber->text() + symbol);
}

void CallDialog::changeSpeakerVolume(int val)
{
    // TODO:
}

void CallDialog::changeMicrophoneVolume(int val)
{
    // TODO:
}

void CallDialog::rejectIncoming()
{
    acceptIncoming(false);
}

void CallDialog::acceptIncoming(bool accept)
{
    if (accept) {
        d->setStatus(Accepting);
    } else {
        d->setStatus(Rejecting);
    }
}

void CallDialog::hangup()
{
    d->setStatus(Terminating);
}

void CallDialog::call()
{
    d->setStatus(Calling);
}

void CallDialog::incoming()
{
    d->setStatus(Incoming);
}

void CallDialog::timerEvent(QTimerEvent * event)
{
    d->timer.start(1000, this);

    int elapsed = d->time.elapsed();
    short int ss, mm, hh;
    ss = elapsed / 1000; // miliseconds

    mm = ss / 60;  ss %= 60;
    hh = mm / 60;  mm %= 60;
    QChar fillChar = '0';

    QString fmtTime = QString("%1:%2")
        .arg(mm, 2, 10, fillChar)
        .arg(ss, 2, 10, fillChar);

    if (hh) {
        fmtTime = QString::number(hh) + ":" + fmtTime;
    }

    d->frameInCall->setTitle(tr("Call duration: <b>%1</b>").arg(fmtTime));
}

