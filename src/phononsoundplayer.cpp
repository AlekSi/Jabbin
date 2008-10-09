/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#define MIN_REPEAT_INTERVAL 1000

#include "phononsoundplayer.h"
#include <QApplication>
#include <QTimer>
#include <QFile>
#include <phonon/mediaobject.h>

SoundPlayer * SoundPlayer::m_instance = NULL;

class SoundPlayer::Private
{
public:
    Private()
        : music(NULL), interval(0)
    {
        timer.setSingleShot(false);
    }

    Phonon::MediaObject * music;
    int interval;
    int soundLength;
    QTimer timer;
};

SoundPlayer::SoundPlayer(QObject * parent)
    : QObject(parent), d(new Private())
{
    connect(&(d->timer), SIGNAL(timeout()),
            this, SLOT(replay()));
}

SoundPlayer::~SoundPlayer()
{
    delete d;
}

SoundPlayer * SoundPlayer::instance()
{
    if (!m_instance) {
        m_instance = new SoundPlayer(qApp);
    }
    return m_instance;
}

void SoundPlayer::playContinuosSound(QString file, int msec)
{
    if (d->music) {
        delete d->music;
    }

    if (file == "!beep") {
        d->music = NULL;
        QApplication::beep();
        d->soundLength = MIN_REPEAT_INTERVAL;
        d->timer.start(d->soundLength);

    } else if (QFile::exists(file)) {
        d->music = Phonon::createPlayer(Phonon::MusicCategory,
            Phonon::MediaSource(file));

        connect(d->music, SIGNAL(aboutToFinish()),
                this, SLOT(rewind()));
        connect(d->music, SIGNAL(totalTimeChanged(qint64)),
                this, SLOT(totalTimeChanged(qint64)));
        d->music->play();
    }

    d->interval = msec;
}

void SoundPlayer::totalTimeChanged(qint64 i)
{
    d->soundLength = (i < MIN_REPEAT_INTERVAL) ?
        MIN_REPEAT_INTERVAL : i;
    d->timer.start(d->soundLength);
}

void SoundPlayer::rewind()
{
    if (!d->music) {
        return;
    }

    d->music->pause();
    d->music->seek(0);
}

void SoundPlayer::replay()
{
    d->interval -= d->soundLength;
    if (d->interval < 0) {
        d->timer.stop();
    } else  {
        if (!d->music) {
            QApplication::beep();
        } else {
            d->music->seek(0);
            d->music->play();
        }
    }
}

void SoundPlayer::stop()
{
    if (!d->music) {
        return;
    }

    d->music->stop();
}

