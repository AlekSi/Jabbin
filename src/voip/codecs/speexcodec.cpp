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

#include "speexcodec.h"
#include "ringbuffer.h"

#include <string.h>
#include <math.h>
#include <QDebug>


#define FRAME_SIZE 160

extern "C" {
#include <speex/speex.h>
};


SpeexCodecFactory::SpeexCodecFactory() {}
SpeexCodecFactory::~SpeexCodecFactory() {}

class SpeexEncoder::Private {
public:
    SpeexBits bits;
    void * state;

    RingBuffer *inputBuffer;
    RingBuffer *outputBuffer;

};


class SpeexDecoder::Private {
public:
    SpeexBits bits;
    void * state;

    //RingBuffer *inputBuffer;
    RingBuffer *outputBuffer;
};


SpeexEncoder::SpeexEncoder()
{
    qDebug() << "SpeexEncoder::SpeexEncoder";

    d = new Private;
    //d->framesPerPacket = 1;
    d->state = 0;
    speex_bits_init(&(d->bits));
    d->inputBuffer = new RingBuffer(1024);
    d->outputBuffer = new RingBuffer(1024);

    speex_bits_init(&(d->bits));
    d->state = speex_encoder_init(&speex_nb_mode);

    int quality=8;
    speex_encoder_ctl(d->state, SPEEX_SET_QUALITY, &quality);
    int complexity=8;
    speex_encoder_ctl(d->state, SPEEX_SET_COMPLEXITY, &complexity);
    int vad=1;
    speex_encoder_ctl(d->state, SPEEX_SET_VAD, &vad);

}

SpeexDecoder::SpeexDecoder()
{
    d = new Private;
    d->state = 0;
    speex_bits_init(&(d->bits));
    //d->inputBuffer = new RingBuffer(1024);
    d->outputBuffer = new RingBuffer(1024);


    speex_bits_init(&(d->bits));
    d->state = speex_decoder_init(&speex_nb_mode);

    int enh = 1;//enable a perceptual post-filter
    speex_decoder_ctl(d->state, SPEEX_SET_ENH, &enh);
}


SpeexEncoder::~SpeexEncoder()
{
    speex_encoder_destroy(d->state);
    speex_bits_destroy(&(d->bits));

    delete d->inputBuffer;
    delete d->outputBuffer;
    delete d;
}

SpeexDecoder::~SpeexDecoder()
{
    speex_decoder_destroy(d->state);
    speex_bits_destroy(&(d->bits));

    //delete d->inputBuffer;
    delete d->outputBuffer;
    delete d;
}


int SpeexEncoder::encode( const short *data, int size, char **res, int *samplesProcessed )
{
    qDebug("speex encode %d samples", size);

    *samplesProcessed = 0;

    d->inputBuffer->put( (char*)data, size*2 );

    //qDebug("get %d samples to encode", size);

    while ( d->inputBuffer->size() >= FRAME_SIZE*2 )
    {
        char output_frame[FRAME_SIZE*2];
        short* input_frame = (short*)d->inputBuffer->data();

        /*Flush all the bits in the struct so we can encode a new frame*/
        speex_bits_reset(&(d->bits));

        speex_encode_int(d->state, input_frame, &(d->bits));

        d->inputBuffer->fetch( FRAME_SIZE*2 );

        char bytes = speex_bits_write(&(d->bits), output_frame, FRAME_SIZE*2);
        d->outputBuffer->put(output_frame, bytes);

        *samplesProcessed += FRAME_SIZE;
    }

    int resSize = d->outputBuffer->size();
    if ( resSize>0 )
    {
        *res = new char[resSize];
        memcpy(*res, d->outputBuffer->data(), resSize );
        d->outputBuffer->clear();
        return resSize;
    }

    return 0;
}


int SpeexDecoder::decode( const char *data, int size, short **res )
{
    qDebug("speex decode %d bytes", size);

    short output_frame[FRAME_SIZE];

    speex_bits_read_from(&(d->bits), (char*)data, size );
    //qDebug( "remaining bits: %d", speex_bits_remaining(&(d->bits)) );
    while ( speex_bits_remaining(&(d->bits)) > 0 ) {
        int ret = speex_decode_int(d->state, &(d->bits), output_frame);

        // advance to the next byte
        int remainingBits = speex_bits_remaining(&(d->bits));
        speex_bits_advance( &(d->bits), remainingBits % 8 );

        //qDebug( "remaining bits: %d", speex_bits_remaining(&(d->bits)) );

        if(ret == -1)
        {
            //debug("End of stream");
            break;
        } else if(ret == -2)
        {
            qDebug("Corrupted stream");
            break;
        }

        //debug("decoded frame");

        d->outputBuffer->put( (char*)output_frame, FRAME_SIZE*2 );

    }

    int resSize = d->outputBuffer->size();

    if (  resSize > 0 ) {
        *res = new short[resSize/2];
        memcpy(*res, d->outputBuffer->data(), (resSize/2)*2 );
        d->outputBuffer->clear();
        //qDebug("speex: decoded %d samples", resSize/2 );
        return resSize/2;
    }

    return 0;
}


