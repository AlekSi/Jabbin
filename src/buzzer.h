/*
 * buzzer.h - buzzes a window
 * Copyright (C) 2008  Yandex LLC (Alexei Matiouchkine)
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

#ifndef BUZZER_H
#define BUZZER_H

#include <QObject>
#include <QIcon>
#include <QPoint>
#include <QTime>

class YaTrayIcon;
class QTimer;
class QWidget;

class Buzzer : public QObject
{
	Q_OBJECT
public:
	Buzzer();
	~Buzzer();

public slots:
	void buzz();
signals:
	void buzzing();
	void buzzed();

public:
	void setTrayIcon(YaTrayIcon*);
	YaTrayIcon* trayIcon() const;

	void setWidget(QWidget*);
	QWidget* widget() const;

	enum Significance { High, Normal, Low };
	void setSignificance(Significance);
	void resetSignificance();
	Significance significance() const;

	void setDuration(int);
	void resetDuration();
	int duration() const;
protected:
	YaTrayIcon* icon_;
	QWidget* widget_;

	Significance significance_;
	int duration_;

private slots:
	void onTimer();
private:
	QTimer * timer_;
	QTime time;
	void timerOn();
	void timerOff();
	void buzzStart();
	void buzzOn(int);
	void buzzFinish();

	QPoint origin_;
};

#endif
