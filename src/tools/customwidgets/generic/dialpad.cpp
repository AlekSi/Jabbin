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

#include "dialpad.h"
#include "ui_dialpad_base.h"
#include "customwidgetscommon.h"

#include <QRegExpValidator>
#include <QRegExp>

namespace CustomWidgets {

class DialPad::Private : public Ui::DialPadBase {
public:

    // Associates a button with its character
    // and assigns the style to it
    #define SetButton(Button, Char, Name) \
        buttons [ Button ] = Char; \
        Button->setStyleSheet(buttonStyle.arg(Name)); \
        connect( Button, SIGNAL(clicked()), parent, SLOT(dialpadButtonClicked()));

    Private(DialPad * parent)
    {
        setupUi(parent);
        frameVolumes->hide();
        buttonStyle =
            Common::readFile(":/customwidgets/generic/data/dialpad_style.css");

        SetButton(btnDial0, '0', "0");
        SetButton(btnDial1, '1', "1");
        SetButton(btnDial2, '2', "2");
        SetButton(btnDial3, '3', "3");
        SetButton(btnDial4, '4', "4");
        SetButton(btnDial5, '5', "5");
        SetButton(btnDial6, '6', "6");
        SetButton(btnDial7, '7', "7");
        SetButton(btnDial8, '8', "8");
        SetButton(btnDial9, '9', "9");
        SetButton(btnDialAsterisk, '*', "asterisk");
        SetButton(btnDialPound, '#', "pound");

        editPhoneNumber->setEmptyText(tr("Enter phone number"));
        editPhoneNumber->setValidator(new QRegExpValidator(QRegExp("[+]?[0-9*#]*"), parent));
    }

    QString buttonStyle;
    QMap < QPushButton *, char > buttons;
};

DialPad::DialPad(QWidget *parent)
    : QFrame(parent), d(new Private(this))
{
}

DialPad::~DialPad()
{
    delete d;
}

void DialPad::setInCall(bool value) {
    d->buttonCall->setEnabled(!value);
    d->buttonHangup->setEnabled(value);
}

void DialPad::dialpadButtonClicked() {
    char symbol = d->buttons[ (QPushButton *) sender() ];
    d->editPhoneNumber->setText(
        d->editPhoneNumber->text() + symbol);
}

} // namespace CustomWidgets
