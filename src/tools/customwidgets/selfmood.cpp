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

#include "selfmood.h"
#include "generic/advancedlineedit.h"
#include "generic/widgetexpander.h"

#include <QKeyEvent>
#include <QIcon>
#include <QMenu>
#include <QVariant>
#include <QDebug>

#define SET_CUSTOM_MOOD -1

using CustomWidgets::AdvancedLineEdit;
using CustomWidgets::WidgetExpander;

class SelfMood::Private {
public:
    Private(SelfMood * parent)
        : q(parent), menu(NULL)
    {
        ale = new AdvancedLineEdit();
        we = new WidgetExpander();

        we->setParent(q);
        we->setChildWidget(ale);

        ale->installEventFilter(parent);
        ale->setEmptyText(tr("Enter your mood"));

        ale->setButtonFlag(AdvancedLineEdit::BeginButton,
                AdvancedLineEdit::ShowAlways);
        ale->setButtonFlag(AdvancedLineEdit::EndButton,
                AdvancedLineEdit::ShowWhenFocused);

        connect(ale, SIGNAL(endButtonClicked()),
                parent, SLOT(acceptMoodText()));
        connect(ale, SIGNAL(beginButtonClicked()),
                parent, SLOT(popupMenu()));

    }

    QIcon iconForStatus(XMPP::Status::Type status) {
        switch (status) {
            case XMPP::Status::Online:
                return QIcon(":/iconsets/roster/roster-big/online.png");
            case XMPP::Status::Away:
                return QIcon(":/iconsets/roster/roster-big/away.png");
            case XMPP::Status::DND:
                return QIcon(":/iconsets/roster/roster-big/dnd.png");
            case XMPP::Status::Invisible:
                return QIcon(":/iconsets/roster/roster-big/invisible.png");
            case XMPP::Status::Offline:
                return QIcon(":/iconsets/roster/roster-big/offline.png");
            default:
                return QIcon();
        }
    }

    QString textForStatus(XMPP::Status::Type status) {
        switch (status) {
            case XMPP::Status::Online:
                return tr("Online");
            case XMPP::Status::Away:
                return tr("Away");
            case XMPP::Status::DND:
                return tr("Don't disturb");
            case XMPP::Status::Invisible:
                return tr("Invisible");
            case XMPP::Status::Offline:
                return tr("Offline");
            default:
                return QString();
        }
    }

    #define addToMenu(A) \
        menu->addAction(iconForStatus(A), textForStatus(A)) \
            ->setData(QVariant(A));

    void createMenu() {
        menu = new QMenu(q);

        // Online, Away, DND, Invisible, Offline
        addToMenu(XMPP::Status::Online);
        // addToMenu(XMPP::Status::Away); // TODO: We don't support these!?
        addToMenu(XMPP::Status::DND);
        // addToMenu(XMPP::Status::Invisible); // TODO: We don't support these!?
        addToMenu(XMPP::Status::Offline);

        menu->addSeparator();
        menu->addAction(tr("Set custom mood..."))
            ->setData(QVariant(SET_CUSTOM_MOOD));
    }

    void popupMenu() {
        if (!menu) {
            createMenu();
        }

        QPoint point = ale->mapToGlobal(
                QPoint(0, ale->height()));

        QAction * chosen = menu->exec(point);
        int result = SET_CUSTOM_MOOD;
        if (chosen) {
            result = chosen->data().toInt();
        }

        if (result != SET_CUSTOM_MOOD) {
            q->setStatusType((XMPP::Status::Type)result);
            ale->clearFocus();
        } else if (chosen) {
            ale->setFocus();
        }
    }

    QString statusText;
    XMPP::Status::Type statusType;

    AdvancedLineEdit * ale;
    WidgetExpander * we;
    SelfMood * q;
    QMenu * menu;
};

SelfMood::SelfMood(QWidget * parent)
    : QWidget(parent), d(new Private(this))
{
}

SelfMood::~SelfMood()
{
    delete d;
}

QString SelfMood::statusText() const
{
    return d->statusText;
}

XMPP::Status::Type SelfMood::statusType() const
{
    return d->statusType;
}

void SelfMood::resizeEvent(QResizeEvent * event)
{
    d->we->resize(size());
}

void SelfMood::setStatusText(const QString & text)
{
    if (d->statusText == text) return;

    d->statusText = text;
    d->ale->setText(text);
    emit statusChanged(text);
}

void SelfMood::setStatusType(XMPP::Status::Type type)
{
    if (d->statusType == type) return;

    d->statusType = type;
    d->ale->setButtonIcon(AdvancedLineEdit::BeginButton, d->iconForStatus(type));
    emit statusChanged(type);
}

void SelfMood::setHaveConnectingAccounts(bool haveConnectingAccounts)
{
    // TODO
}

void SelfMood::clearMoods()
{
    setStatusText(QString());
}

bool SelfMood::eventFilter(QObject * object, QEvent * event)
{
    if (!d) return false;

    if (event->type() == QEvent::KeyPress && object == d->ale) {
        QKeyEvent * keyEvent = static_cast<QKeyEvent *> (event);
        if (keyEvent->key() == Qt::Key_Return ||
                keyEvent->key() == Qt::Key_Enter) {
            acceptMoodText();
        }
    }
    return false;
}

void SelfMood::acceptMoodText() {
    qDebug() << "SelfMood::acceptMoodText";
    setStatusText(d->ale->text());
    d->ale->setSelection(0, 0);
    d->ale->clearFocus();
}

void SelfMood::popupMenu() {
    d->popupMenu();
}

