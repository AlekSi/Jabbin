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

#ifndef ADVANCEDWINDOW_H_
#define ADVANCEDWINDOW_H_
#include "generic/styledwindow.h"

#include <QTabWidget>

class AdvancedWindow: public CustomWidgets::StyledWindow {
    Q_OBJECT

public:
    /**
     * Creates a new AdvancedWindow
     */
    AdvancedWindow(QWidget *parent = 0, Qt::WindowFlags f = 0);

    /**
     * Destroys this StyledWindow
     */
    ~AdvancedWindow();

    void setGeometryOptionPath(const QString & optionPath);
    void setOpacityOptionPath(const QString & optionPath);

    void setMinimizeEnabled(bool enabled);
    void setMaximizeEnabled(bool enabled);
    void setStaysOnTop(bool staysOnTop);

    QToolButton * getConfigButton() const;

public Q_SLOTS:
    void optionChanged(const QString & option);

private:
    class Private;
    Private * const d;
};

#endif // ADVANCEDWINDOW_H_
