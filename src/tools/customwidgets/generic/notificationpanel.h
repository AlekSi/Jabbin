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

#ifndef NOTIFICATION_PANEL_H_
#define NOTIFICATION_PANEL_H_

#include <QFrame>
#include <QPixmap>
#include <QIcon>

namespace CustomWidgets {

/**
 * This class is implementing a notification panel with one icon,
 * a title, a message, and action buttons
 */
class NotificationPanel : public QFrame {
    Q_OBJECT

public:
    enum CallStatus {
        Default,
        Calling, Accepting, Rejecting, Terminating,
        Accepted, Rejected, InProgress, Terminated, Incoming
    };

    /**
     * Creates a new NotificationPanel
     */
    NotificationPanel(QWidget * parent = 0);

    /**
     * Destroys this NotificationPanel
     */
    ~NotificationPanel();

    /**
     * Adds a button to the widget
     * @param text button caption
     * @param icon button icon
     * @param data to be assigned to the button
     */
    void addButton(const QString & text, const QIcon & icon, const QString & data);

    QString title() const;

    void setTitle(const QString & title);

    QString message() const;

    void setMessage(const QString & message);

    void setPixmap(const QPixmap & pixmap);

    void setIcon(const QIcon & icon);

Q_SIGNALS:
    /**
     * This signal is emitted when a button is clicked
     * @param data data assigned to that button
     */
    void buttonClicked(const QString & data);

private:
    class Private;
    Private * const d;
};

} // namespace CustomWidgets

#endif // NOTIFICATION_PANEL_H_

