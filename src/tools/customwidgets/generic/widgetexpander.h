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

#ifndef WIDGET_EXPANDER_H
#define WIDGET_EXPANDER_H

#include <QFrame>
#include <QWidget>
#include <QEvent>

/**
 * This class expands a widget when it has the focus, and
 * collapses it when it doesn't have it.
 */
class WidgetExpander : public QWidget
{
Q_OBJECT
public:
    WidgetExpander(QWidget * child = 0, QWidget * parent = 0);
    ~WidgetExpander();

    /**
     * Sets the widget to expand/collapse based on its
     * focus
     * @param child child widget
     */
    void setChildWidget(QWidget * child);

    /**
     * Sets the minimum width / full width ratio
     */
    void setMinimumWidthRatio(qreal ratio);

protected:
    void resizeEvent(QResizeEvent *);
    bool eventFilter(QObject * object, QEvent * event);


public Q_SLOTS:
    void expand();
    void contract();
    void setChildWidth(int i);

private:
    class Private;
    Private * const d;
};

#endif // WIDGET_EXPANDER_H

