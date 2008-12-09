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

#ifndef C_OPTIONSDIALOG_H
#define C_OPTIONSDIALOG_H

#include <QDialog>
#include "yawindow.h"
#include "psicon.h"

class OptionsDialog : public YaWindow {
    Q_OBJECT

public:
    /**
     * Creates a new OptionsDialog
     */
    explicit OptionsDialog(QWidget * parent = 0);

    /**
     * Destroys this OptionsDialog
     */
    ~OptionsDialog();

    void setController(PsiCon * controller);

public Q_SLOTS:
    void cancel();
    void apply();
    void ok();

    void openPreferences();
    void openAccounts();

    void load();
    void save();

protected:
    void showEvent(QShowEvent * event);

private:
    class Private;
    Private * const d;
};

#endif // C_OPTIONSDIALOG_H

