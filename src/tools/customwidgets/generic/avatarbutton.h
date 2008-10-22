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

#ifndef AVATAR_BUTTON_H
#define AVATAR_BUTTON_H

#include <QPushButton>
#include "customwidgetscommon.h"

namespace CustomWidgets {

/**
 * This class is implementing a button that displays an image
 * and has a few custom signals
 */
class AvatarButton : public QPushButton
{
    Q_OBJECT

public:
    /**
     * Creates a new AvatarButton
     */
    AvatarButton(QWidget *parent = 0);

    /**
     * Destroys this AvatarButton
     */
    ~AvatarButton();

protected:
    // Overridden
    void resizeEvent(QResizeEvent *);
    // Overridden
    void enterEvent(QEvent *);
    // Overridden
    void leaveEvent(QEvent *);

Q_SIGNALS:
    /**
     * This signal is emitted when the user
     * hovers the button with the mouse
     */
    void mouseHovered();

    /**
     * This signal is emitted when the user
     * moves the mouse away from the button
     */
    void mouseLeft();

private:
    class Private;
    Private * const d;
};

} // namespace CustomWidgets

#endif // AVATAR_BUTTON_H

