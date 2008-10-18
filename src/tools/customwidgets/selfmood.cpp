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

class SelfMood::Private {
public:
    Private(SelfMood * parent)
        : q(parent)
    {
        ale = new AdvancedLineEdit();
        we = new WidgetExpander();

        we->setParent(q);
        we->setChildWidget(ale);

        ale->setButtonFlag(AdvancedLineEdit::BeginButton,
                AdvancedLineEdit::ShowAlways);
        ale->setButtonFlag(AdvancedLineEdit::EndButton,
                AdvancedLineEdit::ShowWhenFocused);

    }

    QString statusText;
    XMPP::Status::Type statusType;

    AdvancedLineEdit * ale;
    WidgetExpander * we;
    SelfMood * q;
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
    d->statusText = text;
    d->ale->setText(text);
}

void SelfMood::setStatusType(XMPP::Status::Type type)
{
    d->statusType = type;
}

void SelfMood::setHaveConnectingAccounts(bool haveConnectingAccounts)
{
    // TODO
}

void SelfMood::clearMoods()
{
    setStatusText(QString());
}
