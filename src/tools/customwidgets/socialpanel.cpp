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
#include "applicationinfo.h"
#include "contacttooltip.h"

#include "avatars.h"
#include <QDebug>
#include <QDateTime>
#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>
#include <QDesktopServices>
#include <QWebSettings>
#include <QNetworkDiskCache>
#include <QDir>
#include <QToolButton>

#define TIMER_INTERVAL 300000

using XMPP::Jid;

void SocialPanel::Private::processScriptValue(QScriptValue value)
{
    name = value.property("feed_user").toString();
    if (name.contains("@jabbin.com")) {
        name = name.replace(QRegExp("@jabbin[.]com.*$"), "");
    }

    jid = name + "@jabbin.com";

    // name

    PsiContact * contact = account->findContact(XMPP::Jid(jid));
    if (contact) {
        name = contact->name();
        name = name.replace("@jabbin.com", QString());
    }

    // date
    QDateTime dateObject
        = QDateTime::fromString(
                value.property("item_date").toString(),
                "yyyy-MM-dd hh:mm:ss"
        );
    date = dateObject.toString(Qt::DefaultLocaleShortDate);

    // avatar
    qDebug() << "SocialPanel::Private::processScriptValue:"
                << jid;
    avatar = account->avatarFactory()->getCachedAvatarFileName(
            XMPP::Jid(jid));
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

    timer.stop();

    web->setHtml("Loading...");

    QString user = account->jid().user();
    // user = "stefanogrini";
    QUrl url(
        "http://www.jabbin.com/life/services/" + user + "/friends/" + filter + "/json/p/" + QString::number(currentPage)
        );
    qDebug() << "SocialPanel::Private::reload: " << url;

    httpreader.read(url);
}

void SocialPanel::Private::setFilter(const QString & item)
{
    filter = item.toLower();
    // if (item == tr("All")) {
    //     filter = QString();
    // } else {
    //     filter = item + "/";
    // }
    currentPage = 1;
    reload();
}

void SocialPanel::Private::buttonMoodClicked()
{
    QDesktopServices::openUrl(QUrl("http://www.jabbin.com/life/#mood_container"));
}

void SocialPanel::Private::buttonLifestreamClicked()
{
    QDesktopServices::openUrl(QUrl("http://www.jabbin.com/life/networks"));
}

void SocialPanel::Private::buttonStatusClicked()
{
    QDesktopServices::openUrl(QUrl("http://www.jabbin.com/life/#status_container"));
}


void SocialPanel::Private::linkClicked(const QUrl & url)
{
    qDebug() << "SocialPanel::Private::linkClicked" << url << " -- " << url.scheme() << url.authority() << url.path();
    if (url.scheme() == "go") {
        QString command = url.authority();
        if (command == "first") {
            currentPage = 1;
        } else if (command == "previous") {
            currentPage--;
        } else if (command == "next") {
            currentPage++;
        } else if (command == "last") {
            currentPage = totalPages;
        } else {
            currentPage = command.toInt();
        }
        reload();
    } else if (url.scheme() == "action") {
        // nothing
    } else
        QDesktopServices::openUrl(url);
}

void SocialPanel::Private::linkHovered(const QString & urlString)
{
    QUrl url(urlString);
    qDebug() << "SocialPanel::Private::linkHovered" << url << " -- " << url.scheme() << url.authority() << url.path();
    if (url.scheme() == "action") {
        QString command = url.authority();
        if (command == "usertooltip") {
            QRect windowRect = QRect(web->window()->mapToGlobal(web->window()->rect().topLeft()),
                    web->window()->mapToGlobal(web->window()->rect().bottomRight()));
            QRect globalRect = QRect(web->mapToGlobal(web->rect().topLeft()),
                    web->mapToGlobal(web->rect().bottomRight()));

            globalRect.setRight(windowRect.right());

            PsiContact * contact = account->findContact(XMPP::Jid(url.path().replace("/", "")));

            globalRect.setTop(QCursor::pos().y());
            globalRect.setHeight(10);

            ContactTooltip::instance()->showContact(contact, globalRect);
        }

    }
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
    QString pager = "";

    if (!value.property("data").isValid()) {
        QString item = itemPattern;
        html += item
            .replace("$NAME", "")
            .replace("$TIME", "")
            .replace("$TITLE", "")
            .replace("$CONTENT", tr("Corrupt data retrieved from the server"))
            .replace("$AVATAR", "")
            .replace("$JID", "")
            .replace("$LINK", "http://www.jabbin.com")
            .replace("$SERVICE", "http://www.jabbin.com/life/images/icons/error.png")
            .replace("$THUMBNAILS", "")
        ;
    } else if (!it.hasNext()) {
        QString item = itemPattern;
        html += item
            .replace("$NAME", "")
            .replace("$TIME", "")
            .replace("$TITLE", "")
            .replace("$CONTENT", tr("No items in your friend's lifestreams at the moment"))
            .replace("$AVATAR", "")
            .replace("$JID", "")
            .replace("$LINK", "http://www.jabbin.com")
            .replace("$SERVICE", "http://www.jabbin.com/life/images/icons/error.png")
            .replace("$THUMBNAILS", "")
        ;
    } else {
        QScriptValue meta = value.property("meta");
        int total_records = meta.property("total_records").toInt32();
        int items_per_page = meta.property("items_per_page").toInt32();
        int current_page = meta.property("current_page").toInt32();

        totalPages = total_records / items_per_page;
        while (totalPages * items_per_page < total_records) {
            totalPages++;
        }

        if (current_page > 1) {
            pager += "<a href=\"go://first\">&lt;&lt;</a>";
            pager += "<a href=\"go://previous\">&lt;</a>";
        }

        int page = current_page - 2;
        if (page < 1) { page = 1; }

        if (page > 1) { pager += "..."; }

        for (int i = 1; i <= 5; i++) {
            if (page == current_page) {
                pager += "<span>" + QString::number(page) + "</span>";
            } else {
                pager += "<a href=\"go://" + QString::number(page) + "\">" +
                     QString::number(page) + "</a>";
            }
            page++;
            if (page > totalPages) {
               break;
            }
        }
        page--;

        if (page < totalPages) { pager += "..."; }

        if (current_page < totalPages) {
            pager += "<a href=\"go://next\">&gt;</a>";
            pager += "<a href=\"go://last\">&gt;&gt;</a>";
        }

        pager = "<div class=\"pager\">" + pager + "</div>";

    }



    while (it.hasNext()) {
        QString item = itemPattern;
        it.next();
        QScriptValue itemValue = it.value();
        processScriptValue(itemValue);

        QString title = itemValue.property("title").toString();
        if (title == "null") title = QString();

        QString content = itemValue.property("item_name").toString();
        if (content == "null" || content.isEmpty()) {
            content = itemValue.property("item_content").toString();
        }

        if (content == "null") content = QString();

        if (content.isEmpty()) {
            content = title;
        }
        if (title.contains(content) || content.contains(title)) {
            title = QString();
        }

        QString domain = itemValue.property("feed_domain").toString();
        qDebug() << "SocialPanel::finishedJsonRead: Social domain: " << domain;

        if (!domain.contains("http://jabber.org/protocol/")) {
            domain.replace(QRegExp("[.](com|org)"), "");
            domain.replace(".", "");
            domain = "http://www.jabbin.com/life/images/icons/" + domain + ".png";
        } else {
            domain.replace("http://jabber.org/protocol/", "");
            domain = "qrc:/customwidgets/data/social/" + domain + ".png";
        }

        qDebug() << "SocialPanel::finishedJsonRead: Social domain: " << domain;
        html += item
            .replace("$NAME", name)
            .replace("$TIME", date)
            .replace("$TITLE", title)
            .replace("$CONTENT", content)
            .replace("$AVATAR", avatar)
            .replace("$JID", jid)
            .replace("$LINK", itemValue.property("permalink").toString())
            .replace("$SERVICE", domain)
            .replace("$THUMBNAILS", itemValue.property("thumb_data").toString())
        ;
    }

    html += items[2];

    html = html.replace("$PAGER", pager);

    web->setHtml(html);
    web->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    timer.start(TIMER_INTERVAL, this);
}

SocialPanel * SocialPanel::m_instance = NULL;

SocialPanel * SocialPanel::instance()
{
    return m_instance;
}

void SocialPanel::init(PsiAccount * account)
{
    d->currentPage = 1;
    QString path = ApplicationInfo::homeDir() + "/webcache/";
    QDir().mkpath(path);

    QWebSettings::setOfflineStoragePath(path);

    QNetworkDiskCache * diskCache = new QNetworkDiskCache(this);
    diskCache->setCacheDirectory(path);

    QNetworkAccessManager * networkAccessManager = new QNetworkAccessManager();
    networkAccessManager->setCache(diskCache);
    d->web->page()->setNetworkAccessManager(networkAccessManager);

    QWebSettings::setMaximumPagesInCache(100);

    d->account = account;
    d->timer.start(TIMER_INTERVAL, this);

    connect(d->web, SIGNAL(linkClicked(const QUrl &)),
            d, SLOT(linkClicked(const QUrl &)));
    connect(d->web->page(), SIGNAL(linkHovered(const QString &, const QString &, const QString &)),
            d, SLOT(linkHovered(const QString &)));

    d->setFilter("All");
    d->reload();
}

void SocialPanel::timerEvent(QTimerEvent * event)
{
    if (event->timerId() == d->timer.timerId()) {
        d->timer.stop();
        d->reload();
        d->timer.start(TIMER_INTERVAL, this);
    }
    QWidget::timerEvent(event);
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
    d->layout = new QVBoxLayout(this);
    d->layout->setContentsMargins(0, 0, 0, 0);
    d->layout->setSpacing(0);

    QFrame * topFrame = new QFrame(this);
    topFrame->setName("topFrame");
    topFrame->setStyleSheet("\
            QFrame#topFrame { background: white; border: none; border-bottom: 1px solid silver; }\
            QToolButton { border: none; }");

    QHBoxLayout * layout = new QHBoxLayout(topFrame);
    layout->setSpacing(0);
    layout->setContentsMargins(4, 4, 4, 4);

    QToolButton * button = new QToolButton(topFrame);
    button->setIcon(QIcon(":/customwidgets/data/social/status.png"));
    button->setToolTip(tr("Status"));
    layout->addWidget(button);
    connect(button, SIGNAL(clicked()), d, SLOT(buttonStatusClicked()));

    button = new QToolButton(topFrame);
    button->setIcon(QIcon(":/customwidgets/data/social/lifestream.png"));
    button->setToolTip(tr("Add a new feed"));
    layout->addWidget(button);
    connect(button, SIGNAL(clicked()), d, SLOT(buttonLifestreamClicked()));

    layout->addStretch();

    QLabel * label = new QLabel(topFrame);
    label->setText(tr("Show:"));
    layout->addWidget(label);
    layout->addSpacing(10);

    QComboBox * combo = new QComboBox(topFrame);
    layout->addWidget(combo);
    connect(combo, SIGNAL(activated(const QString &)), d, SLOT(setFilter(const QString &)));

    QStringList services;
    services
        << "flickr"
        << "twitter"
        << "identi.ca"
        << "youtube"
        << "digg"
        << "delicious"
        << "last.fm"
        << "slideshare";

    combo->addItem("All");
    combo->insertSeparator(1);

    combo->addItem("Mood");
    combo->addItem("Status");
    combo->addItem("Activity");
    combo->insertSeparator(5);

    foreach (QString service, services) {
        combo->addItem(service);
    }

    d->layout->addWidget(topFrame);
    d->layout->addWidget(d->web);
}

SocialPanel::~SocialPanel()
{
    delete d;
}

