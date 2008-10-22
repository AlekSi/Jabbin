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

#ifndef ADVANCED_LIENEDIT_H
#define ADVANCED_LIENEDIT_H

#include <QLineEdit>
#include "customwidgetscommon.h"

class QToolButton;

namespace CustomWidgets {

/**
 * This class is implementing a line edit with a few custom functions
 * like the buttons on the start and the end of the widget
 */
class AdvancedLineEdit : public QLineEdit
{
    Q_OBJECT

    Q_PROPERTY ( QString emptyText READ emptyText WRITE setEmptyText )

public:
    /**
     * Creates a new AdvancedLineEdit
     */
    AdvancedLineEdit(QWidget *parent = 0);

    /**
     * Destroys this AdvancedLineEdit
     */
    ~AdvancedLineEdit();

    /**
     * @returns the empty text
     * @see setEmptyText
     */
    QString emptyText() const;

    /**
     * Sets the text to be displayed when the text is
     * empty and the widget
     * @param text text to display
     */
    void setEmptyText(const QString & text);

    /**
     * This enum is used to specify the behaviour of the
     * buttons in line edit. Always
     */
    enum ButtonFlag {
        Hide = 0,
        ShowWhenFocused = 1,
        ShowAlways = 2
    };

    /**
     * This enum specifies the button
     */
    enum Button {
        BeginButton = 0,
        EndButton = 1
    };

    /**
     * Sets the flags for the specified button
     * @param button button
     * @param flag new flag to set
     */
    void setButtonFlag(Button button, ButtonFlag flag);

    /**
     * @returns the flags of the specified button
     * @param button button
     */
    ButtonFlag buttonFlag(Button button) const;

    /**
     * Sets the icon for the specified button
     * @param button button
     * @param icon new icon
     */
    void setButtonIcon(Button button, const QIcon & icon);

    /**
     * @returns the icon of the specified button
     * @param button button
     */
    QIcon buttonIcon(Button button) const;

protected:
    // Overridden
    void resizeEvent(QResizeEvent *);
    // Overridden
    void paintEvent(QPaintEvent * event);
    // Overridden
    void focusInEvent(QFocusEvent * event);
    // Overridden
    void focusOutEvent(QFocusEvent * event);

Q_SIGNALS:
    /**
     * This signal is emitted when the user
     * clicks the begin (left) button
     */
    void beginButtonClicked();

    /**
     * This signal is emitted when the user
     * clicks the end (right) button
     */
    void endButtonClicked();

private:
    class Private;
    Private * const d;
};

} // namespace CustomWidgets

#endif // ADVANCED_LIENEDIT_H

