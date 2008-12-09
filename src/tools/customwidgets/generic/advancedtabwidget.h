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

#ifndef ADVANCED_TABWIDGET_H_
#define ADVANCED_TABWIDGET_H_

#include <QTabWidget>
#include "advancedtabbar.h"

/**
 * This class is implementing a tab widget with some special
 * view modes
 */
class AdvancedTabWidget : public QTabWidget {
    Q_OBJECT

public:
    /**
     * Creates a new AdvancedTabWidget
     */
    AdvancedTabWidget(QWidget *parent = 0);

    /**
     * Destroys this AdvancedLineEdit
     */
    ~AdvancedTabWidget();

    /**
     * Sets the view type
     * @param viewType new value
     */
    void setViewType(AdvancedTabBar::ViewType viewType);

    /**
     * @returns the view type
     */
    AdvancedTabBar::ViewType viewType() const;

    /**
     * Sets the tab bar thickness - width if vertical, height if horizontal
     */
    void setTabBarThickness(int value);

protected:
    // Overridden
    void resizeEvent(QResizeEvent *);

private:
    class Private;
    Private * const d;
};

#endif // ADVANCED_TABWIDGET_H_

