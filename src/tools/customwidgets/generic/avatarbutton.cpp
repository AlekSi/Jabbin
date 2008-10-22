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

#include "avatarbutton.h"

namespace CustomWidgets {

class AvatarButton::Private {
public:
    Private()
        : inResizing(false)
    {
    }

    QString mainstyle;
    bool inResizing;

};

AvatarButton::AvatarButton(QWidget *parent)
    : QPushButton(parent), d(new Private())
{
    d->mainstyle =
        Common::readFile("images/avatarbutton-style.css");
    setStyleSheet(d->mainstyle);
    setFocusPolicy(Qt::NoFocus);
}

AvatarButton::~AvatarButton()
{
    delete d;
}

void AvatarButton::resizeEvent(QResizeEvent * event)
{
    if (d->inResizing) return;
    d->inResizing = true;

    QSize newSize = QSize(
            qMin(size().height(), size().width()),
            qMin(size().height(), size().width())
            );

    resize(newSize);
    setIconSize(newSize - QSize(12, 12));
    d->inResizing = false;
}

void AvatarButton::enterEvent(QEvent * event)
{
    emit mouseHovered();
}

void AvatarButton::leaveEvent(QEvent * event)
{
    emit mouseLeft();
}

} // namespace CustomWidgets
