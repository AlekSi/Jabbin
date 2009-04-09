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

#ifndef EMOTICONSPOPUPBUTTON_H_
#define EMOTICONSPOPUPBUTTON_H_

#include <QToolButton>
#include "iconset.h"

class QResizeEvent;
class PsiAccount;

/**
 * Button for choosing service
 */
class EmoticonsPopupButton: public QToolButton {
    Q_OBJECT

public:
    /**
     * Creates a new EmoticonsPopupButton
     */
    EmoticonsPopupButton(QWidget * parent = 0);

    /**
     * Destroys this EmoticonsPopupButton
     */
    ~EmoticonsPopupButton();

    void setIconset(const Iconset & iconset);

Q_SIGNALS:
    void textSelected(QString text);

private:
    class Private;
    Private * const d;
};

#endif // EMOTICONSPOPUPBUTTON_H_

