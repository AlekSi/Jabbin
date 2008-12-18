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

#ifndef STYLEDWINDOW_H_
#define STYLEDWINDOW_H_

#include <QTabWidget>

class QResizeEvent;
class QToolButton;

/**
 * This class is implementing a window with custom style and
 * decorations
 */
class StyledWindow : public QWidget {
    Q_OBJECT

public:
    /**
     * This enum contains positions of the title bar items.
     */
    enum TitlebarItem {
        Icon = 0,
        Title = 1,
        Minimize = 2,
        Maximize = 3,
        Close = 4
    };

    /**
     * Creates a new StyledWindow
     */
    StyledWindow(QWidget *parent = 0, Qt::WindowFlags f = 0);

    /**
     * Destroys this StyledWindow
     */
    ~StyledWindow();

    /**
     * Sets whether the window uses custom decorations
     * @param value value
     */
    void setUseCustomDecorations(bool value);

    /**
     * @returns whetger the window uses custom decorations
     */
    bool usesCustomDecorations() const;

    /**
     * Sets the custom decoration border size of the window
     * @param size new size of the borders
     */
    void setBorderSize(int size);

    /**
     * Returns the border size of the custom decoration
     */
    int borderSize() const;

    /**
     * Sets whether the specified titlebar item is shown
     * @param item item to change the visibility of
     * @param show whether it should be shown
     */
    void showTitlebarItem(TitlebarItem item, bool show = true);

    /**
     * @returns whether the specified titlebar item is shown
     * @param item item to return the visibility of
     */
    bool isTitlebarItemVisible(TitlebarItem item) const;

    /**
     * Adds a titlebar item
     * @param coordinate relative to TitlebarItem values (will be used as an ID of this item)
     * @param icon icon for the item
     * @param tooltip tooltip for the item
     */
    QToolButton * addTitlebarItem(qreal coordinate, const QIcon & icon,
            const QString & tooltip = QString());

    /**
     * Sets whether the specified titlebar item is shown
     * @param coordinate coordinate used in addTitlebarItem
     * @param show whether it should be shown
     */
    void showTitlebarItem(qreal coordinate, bool show = true);

    /**
     * @returns whether the specified titlebar item is shown
     * @param coordinate coordinate used in addTitlebarItem
     */
    bool isTitlebarItemVisible(qreal coordinate) const;

    /**
     * @returns the pointer to the titlebar item
     * @param item item to return
     */
    QToolButton * titlebarItem(TitlebarItem item) const;

    /**
     * @returns the pointer to the titlebar item
     * @param coordinate coordinate of the item to return
     */
    QToolButton * titlebarItem(qreal coordinate) const;

    bool eventFilter(QObject * object, QEvent * event);

Q_SIGNALS:
    void titlebarItemClicked(qreal coordinate);

protected:
    void resizeEvent(QResizeEvent * event);

private:
    class Private;
    Private * const d;
};

#endif // STYLEDWINDOW_H_

