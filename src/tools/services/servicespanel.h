/*
 *   Copyright (C) 2009 Ivan Cukic <ivan.cukic+kde@gmail.com>
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

#ifndef C_SERVICESPANEL_H
#define C_SERVICESPANEL_H

#include <QWidget>
#include "im.h"
#include "psiaccount.h"

/**
 * This class is implementing a service browser
 */
class ServicesPanel: public QWidget {
    Q_OBJECT

public:
    /**
     * Creates a new ServicesPanel
     */
    explicit ServicesPanel(QWidget * parent = 0);

    /**
     * Destroys this ServicesPanel
     */
    ~ServicesPanel();

    /**
     * @returns a pointer to the last created instance of
     * ServicesPanel. NULL if there is no ServicesPanel created.
     */
    static ServicesPanel * instance();

    /**
     * Since we have only one instance of ServicesPanel, this method
     * sets the needed parameters for it.
     */
    void init(/*const XMPP::Jid & jid,*/ PsiAccount * account);

    /**
     * @returns Jabber ID
     */
    const XMPP::Jid & jid() const;

    /**
     * Sets the Jabber ID
     */
    void setJid(const XMPP::Jid & jid);

private:
    class Private;
    Private * const d;

    static ServicesPanel * m_instance;
};

#endif // C_SERVICESPANEL_H

