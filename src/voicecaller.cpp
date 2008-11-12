/*
 * Copyright (C) 2008  Ivan Cukic
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "voicecaller.h"
#include <QDebug>
#include "tools/customwidgets/calldialog.h"

VoiceCaller::VoiceCaller(PsiAccount * account)
    : account_(account)
{
    qRegisterMetaType<XMPP::Jid>("Jid");
}

PsiAccount * VoiceCaller::account()
{
    return account_;
}

/*
VoiceCallerSignalPropagator * VoiceCallerSignalPropagator::m_instance = NULL;

VoiceCallerSignalPropagator * VoiceCallerSignalPropagator::instance()
{
    if (m_instance == NULL) {
        qRegisterMetaType<XMPP::Jid>("Jid");
        m_instance = new VoiceCallerSignalPropagator();
    }

    return m_instance;
}

VoiceCallerSignalPropagator::VoiceCallerSignalPropagator()
    : QObject()
{

}

void VoiceCallerSignalPropagator::sendIncoming(const Jid & jid)
{
    qDebug() << "void VoiceCallerSignalPropagator::sendIncoming ";
//    CallDialog::instance()->incoming();
    emit incoming(jid);
}

void VoiceCallerSignalPropagator::sendAccepted(const Jid & jid)
{
    qDebug() << "void VoiceCallerSignalPropagator::sendAccepted ";
//    CallDialog::instance()->accepted(jid);
    emit accepted(jid);
}

void VoiceCallerSignalPropagator::sendRejected(const Jid & jid)
{
    qDebug() << "void VoiceCallerSignalPropagator::sendRejected ";
//    CallDialog::instance()->rejected(jid);
    emit rejected(jid);
}

void VoiceCallerSignalPropagator::sendInProgress(const Jid & jid)
{
    qDebug() << "void VoiceCallerSignalPropagator::sendInProgress ";
//    CallDialog::instance()->inProgress(jid);
    emit inProgress(jid);
}

void VoiceCallerSignalPropagator::sendTerminated(const Jid & jid)
{
    qDebug() << "void VoiceCallerSignalPropagator::sendTerminated ";
//    CallDialog::instance()->terminated(jid);
    emit terminated(jid);
}
*/
