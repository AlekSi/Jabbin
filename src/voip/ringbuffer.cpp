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

#include "ringbuffer.h"

#include <stdlib.h> 
#include <string.h>

#include <qmutex.h>

class RingBuffer::Private {
public:
    char* data;
    int ringSize;
    
    int curIndex; // current buffer index
    int size;     // current data size
    QMutex mutex;
};

RingBuffer::RingBuffer( int size )
{
    d = new Private;
    d->data = (char*)malloc(size);
    d->ringSize = size;

    d->curIndex = 0;
    d->size = 0;
}

RingBuffer::~RingBuffer()
{
    free(d->data);
    delete d;
}


int RingBuffer::size()
{
    return d->size;
}

char* RingBuffer::data()
{
    return d->data + d->curIndex;
}

void RingBuffer::fetch(int size)
{
    if ( size>d->size ) {
        qDebug( "Fetched more then buffer size!" );
        d->curIndex = 0;
        d->size = 0;
        
    } else {
        d->size -= size;
        if ( d->size == 0 )
            d->curIndex = 0;
        else
            d->curIndex += size;
    }
}

void RingBuffer::put( char* newData, int size )
{
	if (!newData) return;
    while ( d->size+size > d->ringSize ) {
        d->data = (char*)realloc( d->data, d->ringSize*2 );
        d->ringSize *= 2;
    }

    if ( d->size + d->curIndex + size > d->ringSize ) {
        memmove( d->data, d->data + d->curIndex, d->size );
        d->curIndex = 0;
    }

    memcpy( d->data + d->curIndex + d->size, newData, size );

    d->size += size;
}

void RingBuffer::clear()
{
    d->size = 0;
    d->curIndex = 0;
}

void RingBuffer::lock()
{
    d->mutex.lock();
}

void RingBuffer::unlock()
{
    d->mutex.unlock();
}


