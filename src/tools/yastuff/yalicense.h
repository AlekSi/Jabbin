/*
 * yalicense.h
 * Copyright (C) 2008  Yandex LLC (Michail Pishchagin)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef YALICENSE_H
#define YALICENSE_H

#include "yawindow.h"
#include "yawindowbackground.h"

#include "ui_yalicense.h"

class YaLicense : public YaWindow
{
	Q_OBJECT
public:
	static bool ensureLicenseAccepted();

protected:
	// reimplemented
	void paintEvent(QPaintEvent*);

private slots:
	void rejected();
	void accepted();

private:
	Ui::YaLicense ui_;
	YaWindowBackground background_;

	static QString licenseAcceptedFilePath();
	static bool licenseAccepted();
	void setLicenseAccepted(bool accepted);

	YaLicense();
	~YaLicense();
};

#endif
