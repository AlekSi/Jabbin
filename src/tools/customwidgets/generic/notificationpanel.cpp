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

#include "notificationpanel.h"
#include "ui_notificationpanel_base.h"
#include "generic/customwidgetscommon.h"
#include <QList>
#include <QSignalMapper>
#include <QPushButton>

namespace CustomWidgets {

class NotificationPanel::Private : public Ui::NotificationPanelBase {
public:

    Private(NotificationPanel * parent)
    {
        setupUi(parent);
        buttonStyle =
            Common::readFile(":/customwidgets/generic/data/notificationpanel_button_style.css");

        connect(& signalMapper, SIGNAL(mapped(const QString & )),
                parent, SIGNAL(buttonClicked(const QString & )));
    }

    QString buttonStyle;
    QSignalMapper signalMapper;
};

NotificationPanel::NotificationPanel(QWidget *parent)
    : QFrame(parent), d(new Private(this))
{
}

NotificationPanel::~NotificationPanel()
{
    delete d;
}

void NotificationPanel::addButton(const QString & text, const QIcon & icon, const QString & data)
{
    QPushButton * button = new QPushButton(icon, text, d->buttonBox);
    d->buttonBox->addButton(button, QDialogButtonBox::AcceptRole);

    connect(button, SIGNAL(clicked()), & (d->signalMapper), SLOT(map()));
    d->signalMapper.setMapping(button, data);
    button->setStyleSheet(d->buttonStyle);
}

QString NotificationPanel::title() const
{
    return d->labelTitle->text();
}

void NotificationPanel::setTitle(const QString & title)
{
    d->labelTitle->setText(title);
}

QString NotificationPanel::message() const
{
    return d->labelMessage->text();
}

void NotificationPanel::setMessage(const QString & message)
{
    d->labelMessage->setText(message);
}

void NotificationPanel::setPixmap(const QPixmap & pixmap)
{
    d->labelImage->setPixmap(pixmap);
}

} // namespace CustomWidgets
