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

#ifndef C_CALLDIALOG_H
#define C_CALLDIALOG_H

#include <QFrame>
#include "im.h"
#include "psiaccount.h"
#include "voicecaller.h"

/**
 * This class is implementing a call dialog which
 * contains a dial pad, and volume controls,
 * status reporting...
 */
class CallDialog : public QFrame {
    Q_OBJECT

public:
    /** The call state */
    enum Status {
        Normal,
        Calling,
        InCall,
        Terminating,
        Incoming,
        Accepting,
        Rejecting
    };

    /**
     * Creates a new CallDialog
     */
    explicit CallDialog(QWidget * parent = 0);

    /**
     * Destroys this CallDialog
     */
    ~CallDialog();

    /**
     * @returns a pointer to the last created instance of
     * CallDialog. NULL if there is no CallDialog created.
     */
    static CallDialog * instance();

    /**
     * Since we have only one instance of CallDialog, this method
     * sets the needed parameters for it.
     */
    void init(const XMPP::Jid & jid, PsiAccount * account, VoiceCaller * caller);

    /**
     * @returns Jabber ID
     */
    const XMPP::Jid & jid() const;

    /**
     * Sets the Jabber ID
     */
    void setJid(const XMPP::Jid & jid);

    /**
     * Returns the current state
     */
    Status status() const;

public Q_SLOTS:
    /**
     * Calls the currently entered number
     */
    void call();

    /**
     * Ends the current call
     */
    void hangup();

    /**
     * Call is incoming
     */
    void incoming();

    /**
     * Accepts incoming call
     * @param accept if true, the call is accepted, else the call is rejected
     */
    void acceptIncoming(bool accept = true);

    /**
     * Rejects the incoming call. The same as acceptIncoming(false);
     */
    void rejectIncoming();

    /**
     * Sets the microphone volume
     * @param value new value in range from 0 .. 100
     */
    void changeMicrophoneVolume(int value);

    /**
     * Sets the speaker volume
     * @param value new value in range from 0 .. 100
     */
    void changeSpeakerVolume(int value);

    /**
     * One of the dialpad buttons is clicked
     * @param key the key that the clicked button represents
     */
    void dialpadButtonClicked(char key);

    void accepted(const Jid & );
    void rejected(const Jid & );
    void terminated(const Jid & );
    void inProgress(const Jid & );

protected:
    void timerEvent(QTimerEvent * event);

private:
    class Private;
    Private * const d;

    static CallDialog * m_instance;
};

#endif // C_CALLDIALOG_H

