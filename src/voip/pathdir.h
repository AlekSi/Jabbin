/*
 * pathdir.h - jabbin' directory singleton handler class
 * Copyright (C) 2007  Przemyslaw Maciag
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

/* TODO:
 * - add rootCerts
 * - certsdir is wrong - should point to resourcesdir certs!!!
 */

#ifndef PATHDIR_H
#define PATHDIR_H

#include <qstring.h>
#include <qstringlist.h>
#include <qdir.h>

#include "singleton.h"

class PathDir
{
public:
	PathDir( void );

	// after changing current profile - reinit
	bool profileExists( const QString & name );
	bool profileNew( const QString &name );
	bool profileDelete( const QString & name );

	QStringList profilesList( void );
	QString getProfileDir( QString name );
	void reinitProfile( QString & jidname );

	QString & resourcesDir( void );
	QString & homeDir( void );
	QString & vcardDir( void );
	QString & certsDir( void );
	QString & avatarsDir( void );
	QString & avatarsCacheDir( void );
	QString & profilesDir( void );
	QString & tmpSoundsDir( void );

	QString & currentProfile( void );
	QString & historyDir( void );
	QString & customAvatarsDir( void );

	QStringList & iconsetsDirs( void );

	QString jabbinConfig( void );
	QString profileConfig( const QString & name );
	QString capsFile( void );

	// other
	static bool folderRemove( const QDir & _d );

protected:
	QString getResourcesDir( void );
	QString getHomeDir();

	// if directory do not exists - create it
	bool ensureExists( QString & dir, QString subdir );
	QString getInHomeDir( QString name );
	QString getInProfileDir( QString name );

	QString getHistoryDir();

protected:
	QString m_resourcesDir;
	QString m_homeDir;

	// helpers
	QString m_vcardDir;
	QString m_certsDir;
	QString m_avatarsDir;
	QString m_profilesDir;
	QString m_cacheDir;

	// profile dependend directories
	QString m_currentProfile;
	QString m_historyDir;
	QString m_customAvatarsDir;
	QString m_tmpSounds;

	QStringList m_iconsetsDirs;

//	QString pathBase;
};

typedef Singleton< PathDir > SPathDir;

#endif
