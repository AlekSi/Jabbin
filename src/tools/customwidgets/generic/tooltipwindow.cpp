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

#include "tooltipwindow.h"
#include "ui_tooltipwindow_base.h"

#include <QDesktopWidget>
#include <QDebug>

#include <QSignalMapper>
#include <QPushButton>
#include <QBasicTimer>

namespace CustomWidgets {

// TooltipWindow::Private
class TooltipWindow::Private: public Ui::TooltipWindowBase {
public:
    Private(TooltipWindow * parent)
        : timeout(0), hovered(false)
    {
        setupUi(parent);

        connect(&mapper, SIGNAL(mapped(const QString &)),
             parent, SIGNAL(actionChosen(const QString &)));
        connect(&mapper, SIGNAL(mapped(const QString &)),
             parent, SLOT(close()));

        timebar->hide();
    }

    ~Private()
    {
    }

    int timeout;
    QList < QPushButton * > actions;
    QSignalMapper mapper;
    QBasicTimer timer;

    bool hovered : 1;
};

TooltipWindow::TooltipWindow(QWidget *parent)
    : QWidget(parent), d(new Private(this))
{
    Qt::WindowFlags flags = windowFlags();
    flags |= Qt::FramelessWindowHint | Qt::ToolTip;
    setWindowFlags(flags);
}

TooltipWindow::~TooltipWindow()
{
    delete d;
}

void TooltipWindow::setTitle(const QString & title)
{
    d->labelTitle->setText(title);
}

QString TooltipWindow::title() const
{
    return d->labelTitle->text();
}

void TooltipWindow::setText(const QString & text)
{
    d->labelText->setText(text);
}

QString TooltipWindow::text() const
{
    return d->labelText->text();
}

void TooltipWindow::setIcon(const QPixmap & icon)
{
    d->icon->setPixmap(icon);
}

const QPixmap * TooltipWindow::icon() const
{
    return d->icon->pixmap();
}

void TooltipWindow::showEvent(QShowEvent * event)
{
    // Q_UNUSED(event);

    // QRect g = geometry();
    // g.moveTop(0);
    // g.moveRight(qApp->desktop()->width());
    // setGeometry(g);

    QWidget::showEvent(event);
}

void TooltipWindow::setActions(const QMap < QString, QString > & actions)
{
    foreach (QPushButton * button, d->actions) {
        d->buttons->removeButton(button);
    }

    QMapIterator < QString, QString > i (actions);
    while (i.hasNext()) {
        i.next();
        QPushButton * button = new QPushButton(i.value());
        d->buttons->addButton(button, QDialogButtonBox::AcceptRole);
        d->actions << button;

        connect(button, SIGNAL(clicked()), &(d->mapper), SLOT(map()));
        d->mapper.setMapping(button, i.key());
    }
}

void TooltipWindow::setTimeout(int timeout)
{
    if (timeout) {
        d->timebar->show();
        d->timeout = timeout;
        d->timebar->setMaximum(timeout * 10);
        d->timebar->setValue(timeout * 10);
        d->timer.start(100, this);
    } else {
        d->timer.stop();
        d->timebar->hide();
    }
}

void TooltipWindow::timerEvent(QTimerEvent * event)
{
    if (event->timerId() == d->timer.timerId() && !d->hovered) {
        int val = d->timebar->value();
        val--;
        if (val < 0) {
            close();
            actionChosen(QString());
            d->timer.stop();
            return;
        }
        d->timebar->setValue(val);
        d->timer.start(100, this);
    }

    QWidget::timerEvent(event);
}

void TooltipWindow::leaveEvent(QEvent * event)
{
    d->hovered = false;
    QWidget::leaveEvent(event);
}

void TooltipWindow::enterEvent(QEvent * event)
{
    d->hovered = true;
    QWidget::enterEvent(event);
}

} // namespace CustomWidgets
