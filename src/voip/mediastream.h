/* vim:tabstop=4:shiftwidth=4:expandtab:cinoptions=(s,U1,m1
 * Copyright (C) 2005 Dmitry Poplavsky <dima@thekompany.com>
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
 
#ifndef MEDIASTREAM_H
#define MEDIASTREAM_H

#include <QObject>
#include <QQueue>

namespace cricket {
    class MediaChannel;
	class JabbinMediaEngine;
};

/*!
 \class MediaStream mediastream.h
 \brief stub
 */

class MediaStream : public QObject {
Q_OBJECT
public:
	MediaStream(QObject * parent = 0L, const char * name = 0L);
	virtual ~MediaStream();

    bool isRunning();
    
public slots:   
    void start( QQueue<QByteArray> incomingPackets, cricket::MediaChannel *mediaChannel, int codecPayload);
    void stop();
    
    void timerClick();

    //send packets to network or not
    void setSend(bool send);

signals:
    void finished();

    //show middle and maximum level in range 0..100
    void micLevel( int mid, int max );
    void dspLevel( int mid, int max );

public:    
	class Private;

private:
	Private *d;

	// Allow JabbinMediaEngine to access private
	friend class cricket::JabbinMediaEngine;

	//! Initializes media-backend (opens audio devices, make them ready for streams management)
	static bool _BackendInitialize();
	
	//! Destroys media-backend (closes opened audio devices)
	static void _BackendDestroy();

public:
	void _MicLevel(int avrg, int peak) { emit micLevel(avrg, peak); }
    void _DspLevel(int avrg, int peak) { emit dspLevel(avrg, peak); }
}; 

#endif // MEDIASTREAM_H
