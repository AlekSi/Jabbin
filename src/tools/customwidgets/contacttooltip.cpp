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

#include "contacttooltip.h"
#include "contacttooltip_p.h"

#include <QDesktopWidget>
#include <QApplication>

#include "yacommon.h"

// #define JIDTEXT ((phone == QString())? phone : (jid.bare()))
#define JIDTEXT jid.bare()
#define SLEEPINTERVAL 2000
#define CORNERDIAMETER 4
#define FRAMECOUNT 20
#define TIMEINTERVAL 300

using XMPP::Jid;

ContactTooltip * ContactTooltip::m_instance = NULL;

ContactTooltip::Private::Private(ContactTooltip * parent)
    : q(parent), oldContact(NULL), flipped(false)
{
    setupUi(parent);

    timeline.setFrameRange(0, FRAMECOUNT);
    timeline.setDuration(TIMEINTERVAL);
    connect(& timeline, SIGNAL(frameChanged(int)),
            this, SLOT(timelineFrameChanged(int)));

    buttonList << (buttonChat = new QPushButton(QIcon(":/images/chat_windowicon.png"), tr("Chat")));
    buttonList << (buttonCall = new QPushButton(QIcon(":/customicons/CalPad-Icon16x16"), tr("Call")));
    buttonList << (buttonProfile = new QPushButton(QIcon(":/images/user-actions/button_glyph_profile.png"), tr("Profile")));
    buttonList << (buttonInfo = new QPushButton(QIcon(":/iconsets/chatwindow/profile.png"), tr("Info")));
    buttonList << (buttonHistory = new QPushButton(QIcon(":/images/user-actions/button_glyph_history.png"), tr("History")));

    foreach (QPushButton * button, buttonList) {
        buttons->addButton(button, QDialogButtonBox::AcceptRole);
        connect(button, SIGNAL(clicked()),
            parent, SLOT(hide()));
    }
}

ContactTooltip::Private::~Private()
{
    qDeleteAll(buttonList);
}

void ContactTooltip::Private::timelineFrameChanged(int frame)
{
    qreal factor = frame / (qreal) FRAMECOUNT;

    QRect g = q->geometry();
    g.moveTopLeft(
            moveTo -
            (1 - factor) * (moveTo - moveFrom));
    q->setGeometry(g);
}

ContactTooltip * ContactTooltip::instance()
{
    if (!m_instance) {
        m_instance = new ContactTooltip();
    }
    return m_instance;
}

ContactTooltip::ContactTooltip(QWidget * parent)
    : QFrame(parent), d(new Private(this))
{
    Qt::WindowFlags flags = windowFlags();
    flags |= Qt::FramelessWindowHint | Qt::ToolTip;
    setWindowFlags(flags);
}

ContactTooltip::~ContactTooltip()
{
    delete d;
}

void ContactTooltip::showContact(PsiContact * contact, const QRect & parent)
{
    if (!contact) {
        return;
    }

    // Connecting the buttons
    if (d->oldContact) {
        foreach (QPushButton * button, d->buttonList) {
            button->disconnect(d->oldContact);
        }
    }
    d->oldContact = contact;

    connect(d->buttonChat, SIGNAL(clicked()),
            contact, SLOT(openChat()));
    connect(d->buttonCall, SIGNAL(clicked()),
            contact, SLOT(openCall()));
    connect(d->buttonProfile, SIGNAL(clicked()),
            contact, SLOT(yaProfile()));
    connect(d->buttonInfo, SIGNAL(clicked()),
            contact, SLOT(userInfo()));
    connect(d->buttonHistory, SIGNAL(clicked()),
            contact, SLOT(history()));

    // Setting the data
    d->labelName->setText(contact->name());
    if (contact->name() != contact->jid().bare()) {
        d->labelJID->setText(
                "<a href='mailto:" +
                contact->jid().bare() + "'>" +
                contact->jid().bare() + "</a>"
                );
    } else {
        d->labelJID->setText(QString());
    }
    d->labelStatus->setText(Ya::statusFullText(contact->status().type()));
    if (!contact->picture().isNull()) {
        d->buttonAvatar->setIcon(contact->picture());
    } else {
        d->buttonAvatar->setIcon(QIcon(":/images/gender/default_avatar.png"));
    }
    d->labelDescription->setText(QString());

    d->buttonChat->setVisible(true);
    d->buttonHistory->setVisible(true);
    d->buttonCall->setVisible(true);
    d->buttonCall->setVisible(contact->isCallable());

    d->buttonProfile->setVisible(
        contact->jid().bare().endsWith("@jabbin.com")
        ||
        contact->jid().bare().endsWith("@jabberout.com")
    );

    // Moving the window
    bool flip = false;

    QRect g = geometry();
    g.moveTopLeft(parent.topLeft() - QPoint(g.width(), 0));
    g.moveTop(g.top() - (g.height() - parent.height()) / 2);

    QRect screen = QApplication::desktop()->geometry();
    if (g.left() < screen.left()) {
        g.moveLeft(parent.right());
        if (!d->flipped) {
            flip = true;
            d->flipped = true;
        }
    } else {
        if (d->flipped) {
            flip = true;
            d->flipped = false;
        }
    }

    if (flip) {
        d->horizontalLayout->removeWidget(d->buttons);
        d->horizontalLayout->insertWidget(
                d->flipped ? 0 : 1,
                d->buttons);
    }

    if (g.top() < screen.top()) {
        g.moveTop(screen.top());
    }

    if (g.bottom() > screen.bottom()) {
        g.moveBottom(screen.bottom());
    }

    if (isVisible()) {
        d->timeline.stop();
        d->moveFrom = geometry().topLeft();
        d->moveTo = g.topLeft();
        d->timeline.start();
    } else {
        move(g.topLeft());
        show();
    }
    d->timer.start(SLEEPINTERVAL, this);
}

void ContactTooltip::showSelf(PsiContact * contact, const QRect & parent)
{
    showContact(contact, parent);
    d->buttonChat->setVisible(false);
    d->buttonHistory->setVisible(false);
    d->buttonCall->setVisible(false);
}

void ContactTooltip::timerEvent(QTimerEvent * event)
{
    if (event->timerId() == d->timer.timerId()) {
        d->timer.stop();
        hide();
    }
    QFrame::timerEvent(event);
}

void ContactTooltip::leaveEvent(QEvent * event)
{
    if (isVisible()) {
        d->timer.start(SLEEPINTERVAL, this);
    }
    QFrame::leaveEvent(event);
}

void ContactTooltip::enterEvent(QEvent * event)
{
    d->timer.stop();
    QFrame::enterEvent(event);
}

void ContactTooltip::resizeEvent(QResizeEvent * event)
{
    QWidget::resizeEvent(event);

    QRegion region;
    QRect rect(QPoint(), size());

    region += rect.adjusted(CORNERDIAMETER, 0, - CORNERDIAMETER, 0);
    region += rect.adjusted(0, CORNERDIAMETER, 0, - CORNERDIAMETER);

    QRect corner(rect.topLeft(), QSize(2 * CORNERDIAMETER, 2 * CORNERDIAMETER));
    region += QRegion(corner, QRegion::Ellipse);
    corner.moveTopRight(rect.topRight());
    region += QRegion(corner, QRegion::Ellipse);
    corner.moveBottomRight(rect.bottomRight());
    region += QRegion(corner, QRegion::Ellipse);
    corner.moveBottomLeft(rect.bottomLeft());
    region += QRegion(corner, QRegion::Ellipse);
    setMask(region);
}


