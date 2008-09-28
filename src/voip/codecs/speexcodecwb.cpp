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


#define FRAME_SIZE 320

extern "C" {
#include <speex.h>
};


SpeexCodecFactory::SpeexCodecFactory() {}
SpeexCodecFactory::~SpeexCodecFactory() {}

class SpeexEncoder::Private {
public:
    SpeexBits bits;
    void * state;

    RingBuffer *inputBuffer;
    RingBuffer *outputBuffer;

    // how many packets pack to one RTP frame
    // bigger value increases latency, but reduces bandwidth,
    // not used yet
    int framesPerPacket; 
};


class SpeexDecoder::Private {
public:
    SpeexBits bits;
    void * state;

    RingBuffer *inputBuffer;
    RingBuffer *outputBuffer;
};


SpeexEncoder::SpeexEncoder()
{
    d = new Private;
    d->framesPerPacket = 5;
    d->state = 0;
    speex_bits_init(&(d->bits)); 
    d->inputBuffer = new RingBuffer(1024);
    d->outputBuffer = new RingBuffer(1024);
    
    speex_bits_init(&(d->bits)); 
    d->state = speex_encoder_init(&speex_wb_mode); 

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
    d->inputBuffer = new RingBuffer(1024);
    d->outputBuffer = new RingBuffer(1024);


    speex_bits_init(&(d->bits)); 
    d->state = speex_decoder_init(&speex_wb_mode); 
    
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
    
    delete d->inputBuffer;
    delete d->outputBuffer;
    delete d;
}


int SpeexEncoder::encode( const short *data, int size, char **res )
{

    d->inputBuffer->put( (char*)data, size*2 );

    //qDebug("get %d samples to encode", size);

    while ( d->inputBuffer->size() >= FRAME_SIZE*2 )
    {
        float input_frame[FRAME_SIZE];
        char output_frame[FRAME_SIZE*2];
        short * tmp = (short*)d->inputBuffer->data(); 
        
        /*Copy the 16 bits values to float so Speex can work on them*/
        for(int i=0;i<FRAME_SIZE;i++)
            input_frame[i] = tmp[i];

        
        /*Flush all the bits in the struct so we can encode a new frame*/
        speex_bits_reset(&(d->bits));
        
        speex_encode(d->state, input_frame, &(d->bits));
        
        d->inputBuffer->fetch( FRAME_SIZE*2 );
        
        char bytes = speex_bits_write(&(d->bits), output_frame, FRAME_SIZE*2);
        
        d->outputBuffer->put( &bytes, 1 ); // put the size of frame
        d->outputBuffer->put(output_frame, bytes);
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
    //qDebug("speex decode %d bytes", size);
   
    d->inputBuffer->put( (char*)data, size );

    // may be it will use for VBR later
    while ( d->inputBuffer->size() > 1 ) {
        int frame_size = FRAME_SIZE;
        //speex_decoder_ctl(d->state, SPEEX_GET_FRAME_SIZE, &frame_size); // get frame size
        //qDebug("frame size:%d", frame_size);
        
        float * output_frame  = new float[frame_size];
        short * output  = new short[frame_size];

        char packetSize = *(d->inputBuffer->data());
        d->inputBuffer->fetch(1);
        
        speex_bits_read_from(&(d->bits), d->inputBuffer->data(), packetSize );
        
        int ret = speex_decode(d->state, &(d->bits), output_frame);
        d->inputBuffer->fetch(packetSize);
        if(ret == -1)
        {
            debug("End of stream");
        } else if(ret == -2)
        {
            debug("Corrupted stream");
        } else
        {
            // decoded ok
            for(int i=0;i<frame_size;i++)
            {
                /*
                if(output_frame[i] > 32000.)
                    output_frame[i] = 32000.;
                if(output_frame[i] < -32000.)
                    output_frame[i] = - 32000.;
                */
                output[i] = (short)output_frame[i];
            }
            //for(int i=0;i<frame_size;i++)
            //    output[i] = (short)floor(0.5 + output_frame[i]);
                
        }
        
        d->outputBuffer->put((char*)output, frame_size*2);
        delete output_frame;
        delete output;
    }

    d->inputBuffer->clear();

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


