/*
 * singleton.h - template pattern for singleton objects
 * Copyright (C) 2007  Przemyslaw Maciag
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

#ifndef SINGLETON_H
#define SINGLETON_H

#include <qmutex.h>

template< class T >
class Singleton
{
private:
	Singleton( void );
	~Singleton( void );

public:
	static T * instance( void );

private:
	static T * _instance;
};

template< class T >
Singleton<T>::Singleton( void )
{
}


template< class T >
Singleton<T>::~Singleton( void )
{
}

template< class T >
T * Singleton<T>::instance( void )
{
	static QMutex _mutex;
	_mutex.lock();
	if( ! _instance )
		_instance = new T;
	_mutex.unlock();

	return _instance;
}

#endif

