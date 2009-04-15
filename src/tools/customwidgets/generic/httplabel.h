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

#ifndef C_HTTPLABEL_H
#define C_HTTPLABEL_H

#include <QLabel>
#include <QNetworkReply>
#include <QUrl>
#include <QTimerEvent>
#include "customwidgetscommon.h"

class QToolButton;

namespace CustomWidgets {

/**
 * This class is implementing a label that loads its contents
 * from an url
 */
class HttpLabel : public QLabel
{
    Q_OBJECT

    Q_PROPERTY ( QString patternText READ patternText WRITE setPatternText )
    Q_PROPERTY ( QUrl url READ url WRITE setUrl )
    Q_PROPERTY ( int interval READ interval WRITE setInterval )

public:
    /**
     * Creates a new HttpLabel
     */
    HttpLabel(QWidget *parent = 0);

    /**
     * Destroys this HttpLabel
     */
    ~HttpLabel();

    /**
     * @returns the patternText
     * @see setEmptyText
     */
    QString patternText() const;

    /**
     * Sets the text to be used as a pattern for data loaded from url
     * @param text text to display
     */
    void setPatternText(const QString & text);

    QUrl url() const;

    void setUrl(const QUrl & url);

    int interval() const;

    void setInterval(int interval);

public Q_SLOTS:
    void reload();

protected Q_SLOTS:
    void networkReplyFinished(QNetworkReply * reply);

protected:
    void timerEvent(QTimerEvent * event);

private:
    class Private;
    Private * const d;
};

} // namespace CustomWidgets

#endif // C_HTTPLABEL_H

