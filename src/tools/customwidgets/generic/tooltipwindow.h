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

#ifndef TOOLTIPWINDOW_H_
#define TOOLTIPWINDOW_H_

#include <QWidget>

namespace CustomWidgets {

/**
 * This class is implementing a notification tooltip
 */
class TooltipWindow: public QWidget {
    Q_OBJECT

public:
    /**
     * Creates a new Tooltip
     */
    TooltipWindow(QWidget *parent = 0);

    /**
     * Destroys this Tooltip
     */
    ~TooltipWindow();

    void setTitle(const QString & title);
    QString title() const;

    void setText(const QString & text);
    QString text() const;

    void setIcon(const QPixmap & icon);
    const QPixmap * icon() const;

    void setActions(const QMap < QString, QString > & actions);

    void setTimeout(int timeout);
    int timeout() const;

protected:
    void showEvent(QShowEvent * event);
    void timerEvent(QTimerEvent * event);
    void leaveEvent(QEvent * event);
    void enterEvent(QEvent * event);

Q_SIGNALS:
    void actionChosen(const QString & id);

private:
    class Private;
    Private * const d;
};

} // namespace CustomWidgets

#endif // TOOLTIPWINDOW_H_

