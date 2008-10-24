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

#ifndef SELFAVATAR_H
#define SELFAVATAR_H

#include <QWidget>
#include "xmpp_status.h"
#include "generic/avatarbutton.h"
#include "psicontact.h"
#include "psicontactlist.h"

class SelfAvatar : public CustomWidgets::AvatarButton {
    Q_OBJECT
public:
    SelfAvatar(QWidget * parent = 0);
    ~SelfAvatar();

    virtual void setContactList(const PsiContactList * contactList);
    virtual void setSelfContact(PsiContact * contact);

public Q_SLOTS:
    void popupAvatarsMenu();
    void setAvatar(const QIcon & avatar);
    void accountCountChanged();
    void accountActivityChanged();

protected:
    // Overridden
    void enterEvent(QEvent *);

private:
    class Private;
    Private * const d;
};

#endif // SELFAVATAR_H

