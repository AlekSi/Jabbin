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

#include "emoticonspopupbutton.h"
#include <QAction>
#include <QHash>
#include <QMenu>
#include <QGridLayout>

class EmoticonsPopupMenu: public QMenu {
public:
    EmoticonsPopupMenu(EmoticonsPopupButton * parent);
    ~EmoticonsPopupMenu();

    void updateMenu(Iconset * iconset);
    bool eventFilter(QObject * object, QEvent * event);

private:
    QHash < QToolButton *, QString > buttons;
    EmoticonsPopupButton * q;
    QGridLayout * grid;
};

// EmoticonsPopupButton::Private
class EmoticonsPopupButton::Private: public QObject {
    Q_OBJECT

public:
    Private(EmoticonsPopupButton * parent);

public Q_SLOTS:
    void menuAboutToBeShown();

public:
    EmoticonsPopupMenu * menu;
    Iconset iconset;

    EmoticonsPopupButton * const q;
};
