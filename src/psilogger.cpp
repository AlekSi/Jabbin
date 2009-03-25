/*
 * psilogger.cpp - a simple logger class
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

#include "psilogger.h"

#include <QApplication>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QSettings>

#include "applicationinfo.h"
#include "yadayuse.h"

PsiLogger* PsiLogger::instance_ = 0;

// TODO: Look at Q_FUNC_INFO macro as a possible improvement to LOG_TRACE

PsiLogger::PsiLogger():QObject(QCoreApplication::instance()), file_(0), stream_(0)
{
	bool enableLogging = false;
#ifdef Q_WS_WIN
	{
		QSettings sUser(QSettings::UserScope, "Jabberout", "Jabbin");
		QString log = sUser.value("log").toString();
		if (!log.isEmpty())
			enableLogging = true;
	}
#endif

	{
		char* p = getenv("ENABLE_LOGGING");
		if (p) {
			enableLogging = true;
		}
	}

	if (!enableLogging)
		return;

	QString fileName = ApplicationInfo::homeDir() + "/jabbinchat-log.txt";
	QFile::remove(fileName);
	file_ = new QFile(fileName);
	if (!file_->open(QIODevice::WriteOnly)) {
		qWarning("unable to open log file");
	}
	
	stream_ = new QTextStream();
	stream_->setDevice(file_);
	stream_->setCodec("UTF-8");
	log(QString("*** LOG STARTED %1 (%2 / %3)")
	    .arg(YaDayUse::ver())
	    .arg(YaDayUse::osId())
	    .arg(YaDayUse::osVer()));
}

PsiLogger::~PsiLogger()
{
}

PsiLogger* PsiLogger::instance()
{
	if (!instance_)
		instance_ = new PsiLogger();
	return instance_;
}

void PsiLogger::log(const QString& _msg)
{
	if (!stream_)
		return;

	QDateTime time = QDateTime::currentDateTime();
	QString msg = QString().sprintf("%02d:%02d:%02d]", time.time().hour(), time.time().minute(), time.time().second());
	msg += _msg;

	*stream_ << msg << "\n";
	stream_->flush();
}

void PsiLogger::trace(const char* file, int line, const char* func_info)
{
	log(QString("%1:%2 (%3)").arg(file).arg(line).arg(func_info));
}
