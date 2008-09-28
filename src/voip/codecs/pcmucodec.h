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
 
 #ifndef PCMUCODEC_H
#define PCMUCODEC_H

#include "voicecodec.h"


class PCMUEncoder: public VoiceEncoder {
public:
	PCMUEncoder();
	virtual ~PCMUEncoder();

    // encode chunk of audio data
    // return value: the size of *res
    virtual int encode( const short *data, int size, char **res, int *samplesProcessed  );

};

class PCMUDecoder: public VoiceDecoder {
public:
	PCMUDecoder();
	virtual ~PCMUDecoder();

    // decode chunk of audio data
    // return value: the size of *res
    virtual int decode( const char *data, int size, short **res );

};

class PCMUCodecFactory : public VoiceCodecFactory {
public:
	PCMUCodecFactory();
	virtual ~PCMUCodecFactory();
    
    virtual QString name() { return "PCMU"; }
    virtual double bandwidth() { return 64.0; }
    virtual QString description() { return ""; }

    virtual int payload() { return 0; };

    virtual QString rtmap() { return QString("PCMU/8000/1"); } ;

    virtual VoiceEncoder *encoder() { return new PCMUEncoder(); }
    virtual VoiceDecoder *decoder() { return new PCMUDecoder(); }
};


#endif

