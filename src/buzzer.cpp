/*
 * buzzer.cpp - buzzes a window
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

#include <stdlib.h>

#include <QApplication>
#include <QDesktopWidget>
#include <QTimer>
#include <QWidget>

#include "buzzer.h"
#include "yatrayicon.h"

#define DEFAULT_DURATION 2000
#define PERCENT 100
#define DEFAULT_SIGNIFICANCE Normal
#define BUZZ_SIZE 20.0
#define BUZZ_HALFSIZE BUZZ_SIZE/2.0

#define ICON(x)  QIcon(QString(":/images/trayicon-%1.png").arg(x))

Buzzer::Buzzer()
	: QObject()
	, icon_(0)
	, widget_(0)
	, timer_(0)
	, origin_(QPoint(0,0))
{
	resetDuration();
	resetSignificance();
}


void Buzzer::buzz()
{
	time.start();
	timerOn();
}

Buzzer::~Buzzer()
{
	timerOff();
}

void Buzzer::buzzStart()
{
	emit buzzing();
	if ( widget() ) {
		origin_ = widget()->pos();
	}
}

void Buzzer::buzzFinish()
{
	if ( widget() ) {
		widget()->move(origin_);
	}
	if ( trayIcon() ) {
		trayIcon()->updateIcon();
	}
	emit buzzed();
}

void Buzzer::buzzOn(int percentage)
{
	if ( widget() ) {
		QPoint newPort (
			(int)(BUZZ_SIZE*rand()/RAND_MAX-BUZZ_HALFSIZE),
			(int)(BUZZ_SIZE*rand()/RAND_MAX-BUZZ_HALFSIZE)
		);
		newPort += origin_;
		widget()->move(newPort);
	}
	if ( trayIcon() ) {
		trayIcon()->setIcon(ICON(1 + ((percentage / 5) % 8)));
	}
}

void Buzzer::onTimer()
{
	if (time.elapsed() >= duration()) { // enough
		timerOff();
	} else {
		buzzOn(100 * time.elapsed() / duration());
	}
}

void Buzzer::timerOn()
{
	timerOff();
	timer_ = new QTimer(this);
	connect(timer_, SIGNAL(timeout()), this, SLOT(onTimer()));
	buzzStart();
	timer_->start(duration()/PERCENT); // PERCENT is morelike 100
}

void Buzzer::timerOff()
{
	if (timer_) {
		disconnect(timer_, SIGNAL(timeout()), this, SLOT(onTimer()));
		timer_->stop();
		delete timer_;
		timer_ = 0;
		buzzFinish();
	}
}

void Buzzer::setTrayIcon(YaTrayIcon* icon)
{
	if (this->icon_ != icon) {
		this->icon_ = icon;
		// @todo signal here
	}
}

YaTrayIcon* Buzzer::trayIcon() const
{
	return this->icon_;
}

void Buzzer::setWidget(QWidget* widget)
{
	if (this->widget_ != widget) {
		this->widget_ = widget;
		// TODO: should connect to destroyed(QObject*) and
		// install event filter on a widget to monitor
		// QEvent::Show and QEvent::Hide signals in order
		// to avoid crashes
		// @todo signal here
	}
}

QWidget* Buzzer::widget() const
{
	return this->widget_;
}

void Buzzer::setSignificance(Significance significance)
{
	if (this->significance_ != significance) {
		this->significance_ = significance;
		// @todo signal here
	}
}

void Buzzer::resetSignificance()
{
	this->significance_ = DEFAULT_SIGNIFICANCE;
}

Buzzer::Significance Buzzer::significance() const
{
	return this->significance_;
}

void Buzzer::setDuration(int duration)
{
	if (this->duration_ != duration) {
		this->duration_ = duration;
		// @todo signal here
	}
}

void Buzzer::resetDuration()
{
	this->duration_ = DEFAULT_DURATION;
}

int Buzzer::duration() const
{
	return this->duration_;
}


