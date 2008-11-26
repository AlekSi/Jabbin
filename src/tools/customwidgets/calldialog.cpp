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

#define JIDTEXT jid.bare()

using XMPP::Jid;

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

    frameInCall->addButton("Hangup", QIcon("help"), QString("hangup"));
    frameInCall->setPixmap(QPixmap(":/customwidgets/data/phone.png"));
    connect(frameInCall, SIGNAL(buttonClicked(const QString & )),
            this, SLOT(doAction(const QString & )));

    frameIncomingCall->addButton(tr("Accept"), QIcon("help"), QString("accept"));
    frameIncomingCall->addButton(tr("Reject"), QIcon("help"), QString("reject"));
    frameIncomingCall->setPixmap(QPixmap(":/customwidgets/data/phone.png"));
    connect(frameIncomingCall, SIGNAL(buttonClicked(const QString & )),
            this, SLOT(doAction(const QString & )));

    stacked->setCurrentIndex(0);

    callhistory = new CallHistoryModel(listHistory);

    phonebook = new PhoneBookModel(listPhoneBook, editFilterPhoneBook);
    connect(buttonAddContact, SIGNAL(clicked()),
            phonebook, SLOT(addContact()));

    editFilterPhoneBook->setEmptyText(tr("Search"));

    time.start();
    timer.start(1000, parent);
}

void CallDialog::Private::setStatus(Status value)
{
    qDebug() << "CallDialog :: Status changed to: " << value;
    status = value;

    switch (status) {
        case Normal:
            timer.stop();
            break;
        case Calling:
            frameInCall->setTitle(tr("Calling ..."));
            frameInCall->setMessage(JIDTEXT);
            caller->call(jid);
            callhistory->addEntry(JIDTEXT, jid.full(), CallHistoryModel::Sent);
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
            callhistory->addEntry(JIDTEXT, jid.full(), CallHistoryModel::Missed);
            break;
        case Accepting:
            frameIncomingCall->setTitle(tr("Starting call..."));
            frameIncomingCall->setMessage(JIDTEXT);
            caller->accept(jid);
            callhistory->updateLastEntryStatus(CallHistoryModel::Received);
            break;
        case Rejecting:
            frameIncomingCall->setTitle(tr("Rejecting call..."));
            frameIncomingCall->setMessage(JIDTEXT);
            caller->reject(jid);
            callhistory->updateLastEntryStatus(CallHistoryModel::Rejected);
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

void CallDialog::accepted(const Jid & jid)
{
    qDebug() << "CallDialog::Private::accepted";
    d->setStatus(InCall);
}

void CallDialog::rejected(const Jid & jid)
{
    qDebug() << "CallDialog::Private::rejected";
    d->setStatus(Normal);
}

void CallDialog::terminated(const Jid & jid)
{
    qDebug() << "CallDialog::Private::terminated";
    d->setStatus(Normal);
}

void CallDialog::inProgress(const Jid & jid)
{
    qDebug() << "CallDialog::Private::inProgress";
    d->setStatus(InCall);
}

void CallDialog::Private::doAction(const QString & buttonData)
{
    qDebug() << buttonData;
    if (buttonData == "hangup") {
        setStatus(Terminating);
    } else if (buttonData == "accept") {
        setStatus(Accepting);
    } else if (buttonData == "reject") {
        setStatus(Rejecting);
    }
}

CallDialog * CallDialog::m_instance = NULL;

CallDialog * CallDialog::instance()
{
    return m_instance;
}

void CallDialog::init(const Jid & jid, PsiAccount * account, VoiceCaller * caller)
{
    qDebug() << "CallDialog::Init " << jid.full() <<
        (void *) caller;
    d->jid = jid;
    d->account = account;

    if (caller != d->caller) {
        if (d->caller) {
            qDebug() << "CallDialog::Disconnecting old JingleVoiceCaller";
            // disconnect(caller, NULL, d, NULL);
        }

        d->caller = caller;

        qDebug() << "CallDialog::connecting " <<
        connect(caller, SIGNAL(accepted(Jid)),
                this,   SLOT(accepted(Jid)),
                Qt::QueuedConnection);
        qDebug() << "CallDialog::connecting " <<
        connect(caller, SIGNAL(rejected(const Jid & )),
                this,   SLOT(rejected(const Jid & )),
                Qt::QueuedConnection);
        qDebug() << "CallDialog::connecting " <<
        connect(caller, SIGNAL(terminated(const Jid & )),
                this,   SLOT(terminated(const Jid & )),
                Qt::QueuedConnection);
        qDebug() << "CallDialog::connecting " <<
        connect(caller, SIGNAL(inProgress(const Jid & )),
                this,   SLOT(inProgress(const Jid & )),
                Qt::QueuedConnection);
        }
}

CallDialog::CallDialog(QWidget * parent)
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

