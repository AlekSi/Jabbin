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

#ifndef __SPEEX_CODEC__H__
#define __SPEEX_CODEC__H__

#include "voicecodec.h"

class SpeexEncoder: public VoiceEncoder {
public:
	SpeexEncoder();
	virtual ~SpeexEncoder();

    // encode chunk of audio data
    // return value: the size of *res
    virtual int encode( const short *data, int size, char **res );

private:
	class Private;
	Private *d;

};

class SpeexDecoder: public VoiceDecoder {
public:
	SpeexDecoder();
	virtual ~SpeexDecoder();

    // decode chunk of audio data
    // return value: the size of *res
    virtual int decode( const char *data, int size, short **res );

private:
	class Private;
	Private *d;

};

class SpeexCodecFactory : public VoiceCodecFactory {
public:
	SpeexCodecFactory(); 
	virtual ~SpeexCodecFactory();
    
    virtual QString name() { return "Speex"; }
    virtual double bandwidth() { return 8.0; }
    virtual QString description() { return ""; }

    virtual int payload() { return 97; };

    virtual QString rtmap() { return QString("speex/8000/1"); } ;

    virtual VoiceEncoder *encoder() { return new SpeexEncoder(); }
    virtual VoiceDecoder *decoder() { return new SpeexDecoder(); }
};




#endif // __SPEEX_CODEC__H__

