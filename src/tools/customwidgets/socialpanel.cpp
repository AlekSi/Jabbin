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

#include "socialpanel.h"
#include "socialpanel_p.h"
#include "generic/customwidgetscommon.h"
#include "psicontact.h"

#include "avatars.h"
#include <QDebug>
#include <QDateTime>
#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>
#include <QDesktopServices>

using XMPP::Jid;

void SocialPanel::Private::processScriptValue(QScriptValue value)
{
    jid = value.property("feed_user").toString() + "@jabbin.com";

    // name
    name = value.property("feed_user").toString();

    PsiContact * contact = account->findContact(XMPP::Jid(jid));
    if (contact) {
        name = contact->name();
    }

    qDebug() << "## name ##" << name << "####";

    // date
    QDateTime dateObject
        = QDateTime::fromString(
                value.property("item_date").toString(),
                "yyyy-MM-dd hh:mm:ss"
        );
    date = dateObject.toString(Qt::DefaultLocaleShortDate);
    qDebug() << "## date ##" << date << "####";

    // avatar
    avatar = account->avatarFactory()->getCachedAvatarFileName(
            XMPP::Jid(jid));
    qDebug() << "## avatar ##" << avatar << "####";
}

QString getAvatar(QScriptValue value)
{
    return "";
}

SocialPanel::Private::Private()
{
    connect(&httpreader, SIGNAL(finished(const QString &)),
            this, SLOT(finishedJsonRead(const QString &)));
    account = NULL;
}

void SocialPanel::Private::reload()
{
    if (!account) {
        return;
    }

    QUrl url(
        "http://www.jabbin.com/life/services/" + account->jid().user() + "/friends/json/p/1"
        );
    qDebug() << "SocialPanel::Private::reload: " << url;

    httpreader.read(url);
}

void SocialPanel::Private::linkClicked(const QUrl & url)
{
    QDesktopServices::openUrl(url);
}

void SocialPanel::Private::finishedJsonRead(const QString & data)
{
    QScriptEngine engine;

    qDebug() << "################################# finishedJsonRead";


//    QScriptValue value = engine.evaluate(CustomWidgets::Common::readFile("/tmp/json"));
    QScriptValue value = engine.evaluate(data);
    QString html = CustomWidgets::Common::readFile(
        ":/customwidgets/data/social_template.html");

    QStringList items = html.split(QRegExp("<!-- ITEM:[^ ]* -->"));

    html = items[0];
    QString itemPattern = items[1];

    QScriptValueIterator it(value.property("data"));
    while (it.hasNext()) {
        QString item = itemPattern;
        qDebug() << "#################################";
        it.next();
        QScriptValue itemValue = it.value();
        processScriptValue(itemValue);

        qDebug() << it.name() << ": " << itemValue.toString();

        QString title = itemValue.property("item_title").toString();
        QString content = itemValue.property("item_content").toString();
        qDebug() << "## title ##" << title << "####";
        qDebug() << "## content ##" << content << "####";
        if (content.isEmpty()) {
            content = title;
        }
        if (title.contains(content) || content.contains(title)) {
            title = QString();
        }

        QString domain = itemValue.property("feed_domain").toString();
        domain.replace(QRegExp("[.](com|org)"), "");
        domain.replace(".", "");
        qDebug() << "## domain ##" << domain << "####";
        qDebug() << "## link ##" <<
             itemValue.property("item_permalink").toString() << "####";

        html += item
            .replace("$NAME", name)
            .replace("$TIME", date)
            .replace("$TITLE", title)
            .replace("$CONTENT", content)
            .replace("$AVATAR", avatar)
            .replace("$LINK", itemValue.property("item_permalink").toString())
            .replace("$SERVICE", "http://www.jabbin.com/life/images/icons/" + domain + ".png")
        ;

        QScriptValueIterator inner(itemValue);
        while (inner.hasNext()) {
            qDebug() << "    ####";
            qDebug() << inner.name() << ": "
                     << inner.value().toString();
            if (inner.name().isEmpty()) break;
        }
    }

    html += items[2];

    qDebug() << "################################# finishedJsonRead";
    web->setHtml(html);
    web->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(web, SIGNAL(linkClicked(const QUrl &)),
            this, SLOT(linkClicked(const QUrl &)));
}

SocialPanel * SocialPanel::m_instance = NULL;

SocialPanel * SocialPanel::instance()
{
    return m_instance;
}

void SocialPanel::init(PsiAccount * account)
{
    d->account = account;
    d->reload();
}

void SocialPanel::showEvent(QShowEvent * event)
{
    QWidget::showEvent(event);
}

SocialPanel::SocialPanel(QWidget * parent)
    : QWidget(parent), d(new Private())
{
    m_instance = this;

    d->web = new QWebView(this);
    d->layout = new QStackedLayout(this);
    d->layout->addWidget(d->web);

    d->web->setHtml("Loading...");
}

SocialPanel::~SocialPanel()
{
    delete d;
}

