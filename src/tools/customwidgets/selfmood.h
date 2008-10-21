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

#ifndef SELFMOOD_H
#define SELFMOOD_H

#include <QWidget>
#include "xmpp_status.h"

class SelfMood : public QWidget
{
    Q_OBJECT
public:
    SelfMood(QWidget * parent = 0);
    ~SelfMood();

    QString statusText() const;
    XMPP::Status::Type statusType() const;

protected:
    // Overridden
    void resizeEvent(QResizeEvent *);

    // Overridden
    bool eventFilter(QObject * object, QEvent * event);

public Q_SLOTS:
    void setStatusText(const QString&);
    void setStatusType(XMPP::Status::Type type);
    void setHaveConnectingAccounts(bool haveConnectingAccounts);
    void clearMoods();

    void acceptMoodText();
    void popupMenu();

Q_SIGNALS:
    void statusChanged(const QString & status);
    void statusChanged(XMPP::Status::Type type);

private:
    class Private;
    Private * const d;
};

#endif // WIDGET_EXPANDER_H

