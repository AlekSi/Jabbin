/*
 * yawindowbackground.cpp
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

#include "yawindowbackground.h"

#include <QDir>
#include <QPainter>

#include "psioptions.h"
#include "yavisualutil.h"

YaWindowBackground::Background::Background(const QString& fileName)
	: fileName_(fileName)
{
	static QMap<QString, QColor> tabBackgroundColors;
	if (tabBackgroundColors.isEmpty()) {
		tabBackgroundColors["carbon.png"]   = QColor(0xE5, 0xE5, 0xE5);
		tabBackgroundColors["orange.png"]   = QColor(0xFF, 0xF4, 0xC2);
		tabBackgroundColors["pinky.png"]    = QColor(0xFF, 0xDB, 0xE5);
		tabBackgroundColors["seawater.png"] = QColor(0xDC, 0xFF, 0xDE);
		tabBackgroundColors["silver.png"]   = QColor(0xDA, 0xD0, 0xE8);
		tabBackgroundColors["sky.png"]      = QColor(0xD5, 0xFC, 0xFF);
		tabBackgroundColors["office.png"]   = Ya::VisualUtil::blueBackgroundColor();
	}

	top_ = QPixmap(fileName);
	bottom_ = top_.copy(0, top_.height() - 1,
	                    top_.width(), 1);

	QFileInfo fi(fileName);
	Q_ASSERT(tabBackgroundColors.contains(fi.fileName()));
	tabBackgroundColor_ = tabBackgroundColors[fi.fileName()];
}

static QList<YaWindowBackground::Mode> initializedModes_;
QVector<YaWindowBackground::Background> YaWindowBackground::backgrounds_;
int YaWindowBackground::officeBackground_ = -1;

YaWindowBackground::YaWindowBackground()
{
	init(Random);
}

YaWindowBackground::YaWindowBackground(Mode mode)
{
	init(mode);
}

QDir YaWindowBackground::dir()
{
	return QDir(":images/chat-window");
}

QString YaWindowBackground::firstBackground()
{
	return dir().absoluteFilePath("orange.png");
}

QString YaWindowBackground::officeBackground()
{
	return dir().absoluteFilePath("office.png");
}

QString YaWindowBackground::randomBackground()
{
	QStringList nameFilters;
	nameFilters << "*.png";
	QStringList entries = dir().entryList(nameFilters);

	QFileInfo fi(officeBackground());
	entries.remove(fi.fileName());

	return dir().absoluteFilePath(entries[rand() % entries.count()]);
}

void YaWindowBackground::init(Mode mode)
{
	mode_ = mode;
	background_ = -1;

	if (mode == Random) {
		init(randomBackground());
	}
	else if (mode == Roster) {
		init(firstBackground());
	}
	else if (mode == Chat) {
		// first opened chat uses firstBackground()
		if (!initializedModes_.contains(mode)) {
			initializedModes_.append(mode);
			init(firstBackground());
			mode_ = ChatFirst;
		}
		else {
			init(randomBackground());
		}
	}
	Q_ASSERT(background_ >= 0);
}

void YaWindowBackground::ensureBackgrounds()
{
	if (!backgrounds_.isEmpty())
		return;

	QStringList nameFilters;
	nameFilters << "*.png";
	foreach(QString entry, dir().entryList(nameFilters)) {
		Background background(dir().absoluteFilePath(entry));

		if (background.fileName().startsWith("office") && background.fileName() != officeBackground()) {
			Q_ASSERT(officeBackground().startsWith("office"));
			continue;
		}

		if (background.fileName() == officeBackground()) {
			officeBackground_ = backgrounds_.count();
		}

		backgrounds_.append(background);
	}
	Q_ASSERT(officeBackground_ != -1);
}

void YaWindowBackground::init(const QString& fileName)
{
	ensureBackgrounds();
	for (int i = 0; i < backgrounds_.count(); ++i) {
		if (backgrounds_[i].fileName() == fileName) {
			background_ = i;
			break;
		}
	}
	Q_ASSERT(background_ >= 0);
}

const YaWindowBackground::Background& YaWindowBackground::background() const
{
	currentBackground_ = backgrounds_[background_];

	if (mode_ == Chat || mode_ == ChatFirst) {
		QString chatBackground = PsiOptions::instance()->getOption("options.ya.chat-background").toString();

		if (chatBackground != "random") {
			foreach(Background b, backgrounds_) {
				QFileInfo fi(b.fileName());
				if (fi.fileName() == chatBackground) {
					currentBackground_ = b;
					break;
				}
			}
		}
	}
	else {
		if (PsiOptions::instance()->getOption("options.ya.office-background").toBool()) {
			return backgrounds_[officeBackground_];
		}
	}

	return currentBackground_;
}

void YaWindowBackground::paint(QPainter* p, const QRect& rect, bool isActiveWindow) const
{
	QRect pixmapRect(rect);
	pixmapRect.setHeight(background().top().height());

	p->save();
	if (!isActiveWindow) {
		p->fillRect(rect, Qt::white);
		p->setOpacity(0.75);
	}

	QRect plainRect(rect);
	plainRect.setTop(pixmapRect.bottom());

	p->drawTiledPixmap(pixmapRect, background().top());
	p->drawTiledPixmap(plainRect, background().bottom());

	p->restore();
}

QString YaWindowBackground::name() const
{
	QFileInfo fi(background().fileName());
	return fi.fileName();
}

QColor YaWindowBackground::tabBackgroundColor() const
{
	return background().tabBackgroundColor();
}

QStringList YaWindowBackground::funnyBackgrounds()
{
	QStringList result;

	foreach(Background b, backgrounds_) {
		if (b.fileName() == officeBackground())
			continue;

		QFileInfo fi(b.fileName());
		result << fi.fileName();
	}

	return result;
}
