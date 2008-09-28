/*
 * pathdir.cpp - jabbin' directory singleton handler class
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

// chmod
#include <sys/stat.h>

//getenv on win32
#include <stdlib.h>

#include "pathdir.h"
#include "common.h"
#include "qapplication.h"

#ifndef LIBDIR
#define LIBDIR "/usr/share/jabbin"
#endif

template<> PathDir * SPathDir::_instance = NULL;


PathDir::PathDir( void )
{
	// init with some standard directory names
	char * p = getenv( "JBINDATADIR" );
	if( p )
		m_homeDir = p;
	else
		m_homeDir = getHomeDir();
	m_resourcesDir = getResourcesDir();

	// helpers
	m_vcardDir = getInHomeDir( "vcard" );
	m_certsDir = getInHomeDir( "certs" );
	m_avatarsDir = getInHomeDir( "avatars" );
	m_cacheDir = getInHomeDir( "cache" );
	m_profilesDir = getInHomeDir( "jid" );
	m_tmpSounds = getInHomeDir( "tmp-sounds" );

	m_iconsetsDirs += "./iconsets/";
	m_iconsetsDirs += m_homeDir + "/iconsets/";
	m_iconsetsDirs += m_resourcesDir + "/iconsets/";
}

bool PathDir::profileExists( const QString & name )
{
	QStringList list = profilesList();
	for(QStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
		if((*it).lower() == name)
			return true;
	}
	return false;
}

bool PathDir::profileNew( const QString & name )
{
	if( name.isEmpty() )
		return false;

	// verify the string is sane
	int n, len;
	for( n = 0, len = ( int ) name.length(); n < len; ++n)
	{
		if( ! name.at( n ).isLetterOrNumber() && name.at( n ) != '@' && name.at( n ) != '.' )
			return false;
	}

	// make it
	QDir d( m_profilesDir );
	if( ! d.exists() )
		return false;
	QDir p( m_profilesDir + "/" + name );
	if( ! p.exists() )
	{
		d.mkdir( name );
		if( ! p.exists() )
			return false;
	}

	p.mkdir( "history" );
	p.mkdir( "avatars" );

	return true;
}

bool PathDir::profileDelete( const QString & name )
{
	QDir d( name );
	if( ! d.exists() )
		return true;

	return folderRemove( d );
}

QStringList PathDir::profilesList( void )
{
	QStringList list;

	QDir d( m_profilesDir );
	if( ! d.exists() )
		return list;

	QStringList entries = d.entryList();
	for( QStringList::Iterator it = entries.begin(); it != entries.end(); ++it )
	{
		if( *it == "." || *it == ".." )
			continue;
		QFileInfo info( d, *it );
		if( ! info.isDir() )
			continue;

		list.append( *it );
	}

	list.sort();

	return list;
}

QString PathDir::getProfileDir( QString name )
{
	QString str = m_profilesDir + "/" + name;
	QDir dirname( str );
	if( ! dirname.exists() )
		return QString::null;
	return str;
}

void PathDir::reinitProfile( QString & jidname )
{
	if( ( m_currentProfile = getProfileDir( jidname ) ) == QString::null )
	{
		profileNew( jidname );
		m_currentProfile = getProfileDir( jidname );
	}

	m_historyDir = getInProfileDir( "history" );
	m_customAvatarsDir = getInProfileDir( "avatars" );
}

QString & PathDir::resourcesDir( void )
{
	return m_resourcesDir;
}

QString & PathDir::homeDir( void )
{
	return m_homeDir;
}

QString & PathDir::vcardDir( void )
{
	ensureExists( m_homeDir, "vcard" );
	return m_vcardDir;
}

QString & PathDir::certsDir( void )
{
	ensureExists( m_homeDir, "certs" );
	return m_certsDir;
}

QString & PathDir::avatarsDir( void )
{
	ensureExists( m_homeDir, "avatars" );
	return m_avatarsDir;
}

QString & PathDir::avatarsCacheDir( void )
{
	ensureExists( m_homeDir, "cache" );
	return m_cacheDir;
}

QString & PathDir::profilesDir( void )
{
	ensureExists( m_homeDir, "jid" );
	return m_profilesDir;
}

QString & PathDir::tmpSoundsDir( void )
{
	ensureExists( m_homeDir, "tmp-sounds" );
	return m_tmpSounds;
}

QString & PathDir::currentProfile( void )
{
	return m_currentProfile;
}

QString & PathDir::historyDir( void )
{
	ensureExists( m_currentProfile, "history" );
	return m_historyDir;
}

QString & PathDir::customAvatarsDir( void )
{
	ensureExists( m_currentProfile, "avatars" );
	return m_customAvatarsDir;
}

QStringList & PathDir::iconsetsDirs( void )
{
	return m_iconsetsDirs;
}


QString PathDir::jabbinConfig( void )
{
	return ( homeDir() + "/config.xml" );
}

QString PathDir::profileConfig( const QString & name )
{
	return ( getProfileDir( name ) + "/config.xml" );
}

QString PathDir::capsFile( void )
{
	return ( m_homeDir + "/caps.xml" );
}

bool PathDir::folderRemove( const QDir & _d )
{
	// TODO - czy trzeba QDir'a przepisac?
	QDir d = _d;

	QStringList entries = d.entryList();
	for(QStringList::Iterator it = entries.begin(); it != entries.end(); ++it) {
		if(*it == "." || *it == "..")
			continue;
		QFileInfo info(d, *it);
		if(info.isDir()) {
			if(!folderRemove(QDir(info.filePath())))
				return false;
		}
		else {
			//printf("deleting [%s]\n", info.filePath().latin1());
			d.remove(info.fileName());
		}
	}
	QString name = d.dirName();
	if(!d.cdUp())
		return false;
	//printf("removing folder [%s]\n", d.filePath(name).latin1());
	d.rmdir(name);

	return true;
}


/******************************************************************************/
/* Get path to Resources directory as a string.                               */
/* Return an empty string if can't find it.                                   */
/******************************************************************************/
QString PathDir::getResourcesDir( void )
{
#if defined( Q_WS_X11 )
	return LIBDIR;
#elif defined ( Q_WS_WIN )
	#if QT_VERSION >= 0x030200
		return qApp->applicationDirPath();
	#else
		char baseName[MAX_PATH];
		GetModuleFileNameA(GetModuleHandle(NULL), baseName, MAX_PATH);
		QString base(baseName);
		int idx = base.findRev('\\');

		if (-1 == idx)
			return ".";

		base.truncate(idx);
		QDir baseDir(base);

		return baseDir.absPath();
	#endif
#elif defined ( Q_WS_MAC )
	// may be broken!!!
  // System routine locates resource files. We "know" that Jabbin.icns is
  // in the Resources directory.
  QString resourcePath;
  CFBundleRef mainBundle = CFBundleGetMainBundle();
  CFStringRef resourceCFStringRef
      = CFStringCreateWithCString( NULL, "application.icns",
                                   kCFStringEncodingASCII );
  CFURLRef resourceURLRef = CFBundleCopyResourceURL( mainBundle,
                                                     resourceCFStringRef,
                                                     NULL,
                                                     NULL );
  if ( resourceURLRef ) {
    CFStringRef resourcePathStringRef =
    CFURLCopyFileSystemPath( resourceURLRef, kCFURLPOSIXPathStyle );
    const char* resourcePathCString =
      CFStringGetCStringPtr( resourcePathStringRef, kCFStringEncodingASCII );
    if ( resourcePathCString ) {
      resourcePath.setLatin1( resourcePathCString );
    } else { // CFStringGetCStringPtr failed; use fallback conversion
      CFIndex bufferLength = CFStringGetLength( resourcePathStringRef ) + 1;
      char* resourcePathCString = new char[ bufferLength ];
      Boolean conversionSuccess =
        CFStringGetCString( resourcePathStringRef,
                            resourcePathCString, bufferLength,
                            kCFStringEncodingASCII );
      if ( conversionSuccess ) {
        resourcePath = resourcePathCString;
      }
      delete [] resourcePathCString;  // I own this
    }
    CFRelease( resourcePathStringRef ); // I own this
  }
  // Remove the tail component of the path
  if ( ! resourcePath.isNull() ) {
    QFileInfo fileInfo( resourcePath );
    resourcePath = fileInfo.dirPath( true );
  }
  return resourcePath;
#endif
}

QString PathDir::getHomeDir( void )
{
#if defined( Q_WS_X11 ) || defined ( Q_WS_MAC )
	// may be broken for MAC OSX!!!
	QDir proghome( QDir::homeDirPath() + "/.jabbin" );
	if( ! proghome.exists() )
	{
		QDir home = QDir::home();
		home.mkdir( ".jabbin" );
		chmod( QFile::encodeName( proghome.path() ), 0700 );
	}

	return proghome.path();
#elif defined ( Q_WS_WIN )
	QString base;

	// Windows 9x
	if( QDir::homeDirPath() == QDir::rootDirPath() )
		base = ".";
	// Windows NT/2K/XP variant
	else
		base = QDir::homeDirPath() + "/Application Data";

	// no trailing slash
	if( base.at( base.length() - 1 ) == '/' )
		base.truncate( base.length() - 1 );

	QDir proghome( base + "/Jabbin" );
	if( ! proghome.exists() )
	{
		QDir home( base );
		home.mkdir( "Jabbin" );
	}

	return proghome.path();
#endif
}

bool PathDir::ensureExists( QString & dir, QString subdir )
{
	QDir dirname( dir + "/" + subdir );

	if( dirname.exists() )
		return true;

	QDir home( dir );
	home.mkdir( subdir );

	return dirname.exists();
}

QString PathDir::getInHomeDir( QString name )
{
	ensureExists( m_homeDir, name );
	QDir dirname( m_homeDir + "/" + name );
	return dirname.path();
}

QString PathDir::getInProfileDir( QString name )
{
	ensureExists( m_currentProfile, name );
	QDir dirname( m_currentProfile + "/" + name );
	return dirname.path();
}
