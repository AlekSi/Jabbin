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

#include "httplabel.h"
#include <QDebug>
#include <QNetworkAccessManager>
#include <QBasicTimer>
#include <QDesktopServices>
#include <QUrl>

namespace CustomWidgets {

class HttpLabel::Private {
public:
    Private()
        : interval(10000),
          manager(NULL),
          pattern("%1")
    {
    }

    int interval;
    QNetworkAccessManager * manager;
    QUrl url;
    QString pattern;
    QBasicTimer timer;
};

HttpLabel::HttpLabel(QWidget *parent)
    : QLabel(parent), d(new Private())
{
    d->manager = new QNetworkAccessManager(this);
    connect(d->manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(networkReplyFinished(QNetworkReply*)));
    connect(this, SIGNAL(linkActivated(const QString &)),
            this, SLOT(openLink(const QString &)));
}

HttpLabel::~HttpLabel()
{
    delete d->manager;
    delete d;
}

void HttpLabel::openLink(const QString & link)
{
    QDesktopServices::openUrl(QUrl(link));
}

void HttpLabel::networkReplyFinished(QNetworkReply * reply)
{
    QByteArray data = reply->readAll();
    qDebug() << "HttpLabel::networkReplyFinished" << data.trimmed();
    setText(d->pattern.arg(((QString)data).trimmed()));
    d->timer.start(d->interval, this);
}

void HttpLabel::reload()
{
    d->manager->get(QNetworkRequest(d->url));
}

void HttpLabel::timerEvent(QTimerEvent * event)
{
    if (event->timerId() != d->timer.timerId())
        return;

    d->timer.stop();
    reload();
}

void HttpLabel::setPatternText(const QString & text)
{
    d->pattern = text;
    if (d->pattern.isEmpty()) {
        d->pattern = "%1";
    }
}

QString HttpLabel::patternText() const
{
    return d->pattern;
}

void HttpLabel::setUrl(const QUrl & url)
{
    d->url = url;
}

QUrl HttpLabel::url() const
{
    return d->url;
}

void HttpLabel::setInterval(int interval)
{
    d->interval = interval;
}

int HttpLabel::interval() const
{
    return d->interval;
}

} // namespace CustomWidgets

