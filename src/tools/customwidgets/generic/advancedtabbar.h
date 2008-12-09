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

#ifndef ADVANCED_TABBAR_H_
#define ADVANCED_TABBAR_H_

#include <QTabBar>

/**
 * The class that provides the tabs for AdvancedTabWidget.
 */
class AdvancedTabBar: public QTabBar {
    Q_OBJECT;

    Q_PROPERTY ( ViewType viewType READ viewType WRITE setViewType )

public:
    /**
     * This enum is used to specify how the tabs are displayed
     */
    enum ViewType {
        None = 0,
        TabBar = 1,
        ListView = 2,
    };

    /**
     * Creates a new AdvancedTabBar
     */
    AdvancedTabBar(QWidget * parent);

    /**
     * Sets the view type
     * @param viewType new value
     */
    void setViewType(ViewType viewType);

    /**
     * @returns the view type
     */
    ViewType viewType() const;

    /**
     * Destroys this AdvancedTabBar
     */
    ~AdvancedTabBar();

    /**
     * Sets the tab bar thickness - width if vertical, height if horizontal
     */
    void setThickness(int value);

    // reimplemented
    QSize tabSizeHint(int index) const;

    // reimplemented
    void paintEvent(QPaintEvent * event);

    // reimplemented
    void resizeEvent(QResizeEvent * event);

    // reimplemented
    void tabLayoutChange();

protected Q_SLOTS:
    void currentIndexChanged(int index);

private:
    class Private;
    Private * const d;
};

#endif // ADVANCED_TABBAR_H_
