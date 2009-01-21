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

#include "servicespanel.h"
#include "servicespanel_p.h"
#include "servicesmodel.h"

#include <QDebug>

using XMPP::Jid;

ServicesPanel::Private::Private(ServicesPanel * parent)
    : account(NULL), q(parent)
{
    setupUi(parent);
    ServicesModel * model = new ServicesModel();
    treeServices->setModel(model);
}

ServicesPanel * ServicesPanel::m_instance = NULL;

ServicesPanel * ServicesPanel::instance()
{
    return m_instance;
}

void ServicesPanel::init(const Jid & jid, PsiAccount * account)
{
    d->jid = jid;
    d->account = account;
}

ServicesPanel::ServicesPanel(QWidget * parent)
    : QWidget(parent), d(new Private(this))
{
    m_instance = this;
}

ServicesPanel::~ServicesPanel()
{
    delete d;
}

