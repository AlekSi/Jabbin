/*
 * activeprofiles.h - Class for interacting with other psi instances
 * Copyright (C) 2006  Maciej Niedzielski
 * Copyright (C) 2006-2007  Martin Hostettler
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

#ifndef ACTIVEPSIPROFILES_H
#define ACTIVEPSIPROFILES_H

#include <QUrl>
#include <QStringList>

class ActiveProfiles: public QObject
{
	Q_OBJECT
public:
	static ActiveProfiles* instance();

	bool setThisProfile(const QString &profile);
	void unsetThisProfile();

	QString thisProfile() const;
	bool isActive(const QString &profile) const;

	QString pickProfile() const;

	bool raiseOther(QString profile, bool withUI) const;

	bool sendOpenUri(const QUrl &uri, const QString &profile = QString::null) const;
	bool sendOpenUri(const QString &uri, const QString &profile = QString::null) const;

	~ActiveProfiles();

signals:
	void openUri(const QUrl &uri);
	void raiseMainWindow();

protected:
	static ActiveProfiles *instance_;

private:
	class Private;
	Private *d;

	ActiveProfiles();
};

#endif
