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

#include "customwidgetscommon.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

namespace CustomWidgets {

QString Common::readFile(const QString & fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "File not found " << fileName;
        return QString();
    }

    QString result;

    QTextStream in(&file);
    QString line = in.readLine();
    while (!line.isNull()) {
        result += line;
        line = in.readLine();
    }

    return result;
}

// HttpReader
class HttpReader::Private {
public:
    QNetworkAccessManager manager;
};

HttpReader::HttpReader()
    : d(new Private())
{
    connect(&(d->manager), SIGNAL(finished(QNetworkReply*)),
            this, SLOT(networkReplyFinished(QNetworkReply*)));
}

void HttpReader::read(const QUrl & url)
{
    d->manager.get(QNetworkRequest(url));
}

void HttpReader::networkReplyFinished(QNetworkReply * reply)
{
    QByteArray data = reply->readAll();
    emit finished(data.trimmed());
}

} // namespace CustomWidgets

