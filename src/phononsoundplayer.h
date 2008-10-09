/*
 *   phononsoundplayer.h - contains reimplementation of Justin Karneges'
 *   SoundPlayer class which uses Phonon multimedia framework.
 *
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

#ifndef PHONON_SOUND_PLAYER_H
#define PHONON_SOUND_PLAYER_H

#include <QObject>

class SoundPlayer: public QObject {
    Q_OBJECT
public:
    SoundPlayer(QObject * parent);
    virtual ~SoundPlayer();

    static SoundPlayer * instance();

    void playContinuosSound(QString file, int msec);
    void stop();

protected slots:
    void replay();
    void rewind();
    void totalTimeChanged(qint64 i);

private:
    class Private;
    Private * const d;

    static SoundPlayer * m_instance;
};

#endif /* PHONON_SOUND_PLAYER_H */

