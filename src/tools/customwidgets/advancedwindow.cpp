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

#include "advancedwindow.h"
#include "psioptions.h"
#include <QDebug>

// AdvancedWindow::Private
class AdvancedWindow::Private {
public:
    Private(AdvancedWindow * parent)
        : q(parent), buttonConfig(NULL)
    {
    }

    ~Private()
    {
    }

    void init()
    {
        if (buttonConfig) return;

        q->setUseCustomDecorations(true);
        buttonConfig = q->addTitlebarItem(StyledWindow::Title + 0.5,
                QIcon(":/customwidgets/generic/data/window_button_config.png"), tr("Preferences"));

    }

    AdvancedWindow * q;
    QToolButton * buttonConfig;
    QString geometryOptionPath;
};

// AdvancedWindow
AdvancedWindow::AdvancedWindow(QWidget *parent, Qt::WindowFlags f)
    : StyledWindow(parent, f), d(new Private(this))
{
}

AdvancedWindow::~AdvancedWindow()
{
    delete d;
}

void AdvancedWindow::setGeometryOptionPath(const QString & optionPath)
{
    d->geometryOptionPath = optionPath;

    QRect savedGeometry = PsiOptions::instance()->getOption(d->geometryOptionPath).toRect();
    if (savedGeometry.isValid()) {
        setGeometry(savedGeometry);
    }
}

void AdvancedWindow::setOpacityOptionPath(const QString & optionPath)
{
    // TODO:
}

void AdvancedWindow::setMinimizeEnabled(bool enabled)
{
    d->init();
    showTitlebarItem(StyledWindow::Minimize, enabled);
}

void AdvancedWindow::setMaximizeEnabled(bool enabled)
{
    d->init();
    showTitlebarItem(StyledWindow::Maximize, enabled);
}

void AdvancedWindow::setStaysOnTop(bool staysOnTop)
{
    // TODO:
}

void AdvancedWindow::optionChanged(const QString & option)
{
    // TODO:
}

QToolButton * AdvancedWindow::getConfigButton() const
{
    d->init();
    return d->buttonConfig;
}

