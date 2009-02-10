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

#ifndef C_CONTACTTOOLTIP_H
#define C_CONTACTTOOLTIP_H

#include <QFrame>
#include <QResizeEvent>
#include "psicontact.h"

/**
 * This class is implementing a tooltip with contact
 * info and actions
 */
class ContactTooltip: public QFrame {
    Q_OBJECT
public:
    static ContactTooltip * instance();

    /**
     * Destroys this ContactTooltip
     */
    ~ContactTooltip();

    /**
     * This shows the ContactTooltip with the specified contact
     */
    void showContact(PsiContact * contact = NULL, const QRect & parent = QRect());

protected:
    void timerEvent(QTimerEvent * event);
    void leaveEvent(QEvent * event);
    void enterEvent(QEvent * event);
    void resizeEvent(QResizeEvent * event);

private:
    ContactTooltip(QWidget * parent = 0);

    class Private;
    Private * const d;

    static ContactTooltip * m_instance;
};

#endif // C_CONTACTTOOLTIP_H

