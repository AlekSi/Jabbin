#include <QString>
#include <QDir>
#include <QFile>
#include <QSettings>

#if defined(Q_WS_X11) || defined(Q_WS_MAC)
#include <sys/stat.h> // chmod
#endif

#ifdef Q_WS_WIN
#if __GNUC__ >= 3
#	define WINVER    0x0500
#	define _WIN32_IE 0x0500
#endif
#include <windows.h>
#include <shlobj.h>
#endif

#ifdef Q_WS_MAC
#include <CoreServices/CoreServices.h>
#endif

#include "applicationinfo.h"
#include "profiles.h"
#ifdef HAVE_CONFIG
#include "config.h"
#endif

#include "joimpsi_revision.h"

// Constants. These should be moved to a more 'dynamically changeable'
// place (like an external file loaded through the resources system)
// Should also be overridable through an optional file.

#define PROG_NAME QString::fromUtf8("Joim")
#define PROG_VERSION JOIMPSI_VERSION;
#define PROG_CAPS_NODE "http://www.jabberout.com/caps";
#define PROG_CAPS_VERSION JOIMPSI_VERSION;
#define PROG_IPC_NAME "com.jabberout.joim"
#define PROG_OPTIONS_NS "http://www.jabberout.com/options";
#define PROG_STORAGE_NS "http://www.jabberout.com/storage";

#if defined(Q_WS_X11) && !defined(PSI_DATADIR)
#define PSI_DATADIR "/usr/local/share/psi"
#endif


QString ApplicationInfo::name()
{
	return PROG_NAME;
}


QString ApplicationInfo::version()
{
	return PROG_VERSION;
}

QString ApplicationInfo::capsNode()
{
	return PROG_CAPS_NODE;
}

QString ApplicationInfo::capsVersion()
{
	return PROG_CAPS_VERSION;
}

QString ApplicationInfo::IPCName()
{
	return PROG_IPC_NAME;
}

QString ApplicationInfo::optionsNS()
{
	return PROG_OPTIONS_NS;
}

QString ApplicationInfo::storageNS()
{
	return PROG_STORAGE_NS;
}	

QString ApplicationInfo::resourcesDir()
{
#if defined(Q_WS_X11)
	return PSI_DATADIR;
#elif defined(Q_WS_WIN)
	return qApp->applicationDirPath();
#elif defined(Q_WS_MAC)
	return QCoreApplication::instance()->applicationDirPath() + "/../Resources";
#endif
}


/** \brief return psi's private read write data directory
  * unix+mac: $HOME/.psi
  * environment variable "PSIDATADIR" overrides
  */
QString ApplicationInfo::homeDir()
{
	// Try the environment override first
	char *p = getenv("YACHATDATADIR");
	if(p)
		return p;

#if defined(Q_WS_X11) || defined(Q_WS_MAC)
	QDir proghome(QDir::homeDirPath() + "/.joimchat");
	if(!proghome.exists()) {
		QDir home = QDir::home();
		home.mkdir(".joimchat");
		chmod(QFile::encodeName(proghome.path()), 0700);
	}
	return proghome.path();
#elif defined(Q_WS_WIN)
	QString base = QDir::homePath();
	WCHAR str[MAX_PATH+1] = { 0 };
	if (SHGetSpecialFolderPathW(0, str, CSIDL_APPDATA, true))
		base = QString::fromWCharArray(str);

	QDir proghome(base + "/JoimChatData");
	if(!proghome.exists()) {
		QDir home(base);
		home.mkdir("JoimChatData");
	}

	return proghome.path();
#endif
}

QString ApplicationInfo::historyDir()
{
	QDir history(pathToProfile(activeProfile) + "/history");
	if (!history.exists()) {
		QDir profile(pathToProfile(activeProfile));
		profile.mkdir("history");
	}

	return history.path();
}

QString ApplicationInfo::vCardDir()
{
	QDir vcard(pathToProfile(activeProfile) + "/vcard");
	if (!vcard.exists()) {
		QDir profile(pathToProfile(activeProfile));
		profile.mkdir("vcard");
	}

	return vcard.path();
}

QString ApplicationInfo::profilesDir()
{
	QString profiles_dir(homeDir() + "/profiles");
	QDir d(profiles_dir);
	if(!d.exists()) {
		QDir d(homeDir());
		d.mkdir("profiles");
	}
	return profiles_dir;
}

QString ApplicationInfo::documentsDir()
{
#ifdef Q_WS_WIN
	// http://lists.trolltech.com/qt-interest/2006-10/thread00018-0.html
	// TODO: haven't a better way been added to Qt since?
	QSettings settings(QSettings::UserScope, "Microsoft", "Windows");
	settings.beginGroup("CurrentVersion/Explorer/Shell Folders");
	return settings.value("Personal").toString();
#else
	return QDir::homePath();
#endif
}

#ifdef YAPSI
QString ApplicationInfo::yavatarsDir()
{
	QDir yavatars(pathToProfile(activeProfile) + "/yavatars");
	if (!yavatars.exists()) {
		QDir profile(pathToProfile(activeProfile));
		profile.mkdir("yavatars");
	}

	return yavatars.path();
}
#endif

#ifdef HAVE_BREAKPAD
QString ApplicationInfo::crashReporterDir()
{
	QString crashReporterDir(homeDir() + "/crash_reporter");
	QDir d(crashReporterDir);
	if (!d.exists()) {
		QDir d(homeDir());
		d.mkdir("crash_reporter");
	}
	return crashReporterDir;
}
#endif
