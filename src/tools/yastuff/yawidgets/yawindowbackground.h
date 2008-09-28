/*
 * yawindowbackground.h
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

#ifndef YAWINDOWBACKGROUND_H
#define YAWINDOWBACKGROUND_H

#include <QPixmap>
#include <QVector>

class QDir;

class YaWindowBackground
{
public:
	enum Mode {
		None = 0,
		Random,
		Roster,
		Chat,
		ChatFirst
	};

	YaWindowBackground();
	YaWindowBackground(Mode mode);

	static QStringList funnyBackgrounds();

	QString name() const;
	QColor tabBackgroundColor() const;
	void paint(QPainter* p, const QRect& rect, bool isActiveWindow) const;

private:
	class Background
	{
	public:
		Background() {}
		Background(const QString& fileName);

		const QString& fileName() const { return fileName_; }
		const QPixmap& top() const { return top_; }
		const QPixmap& bottom() const { return bottom_; }
		const QColor& tabBackgroundColor() const { return tabBackgroundColor_; }

	private:
		QString fileName_;
		QPixmap top_;
		QPixmap bottom_;
		QColor tabBackgroundColor_;
	};

	Mode mode_;
	int background_;
	static QVector<Background> backgrounds_;
	static int officeBackground_;
	mutable Background currentBackground_;

	static void ensureBackgrounds();
	void init(Mode mode);
	void init(const QString& fileName);

	const Background& background() const;

	static QDir dir();
	static QString firstBackground();
	static QString officeBackground();
	static QString randomBackground();
};

#endif
