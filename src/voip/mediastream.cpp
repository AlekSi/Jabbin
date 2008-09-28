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

#include "mediastream.h"

// jrtplib includes
#include "rtpsession.h"
#include "rtppacket.h"
#include "rtpudpv4transmitter.h"
#include "jabbintransmitter.h"
#include "rtpsessionparams.h"
#include "rtpipv4address.h"

// portaudio
#include "portaudio.h"



#include "voicecodec.h"
#include "sdp.h"
#include "ringbuffer.h"

#include <qtimer.h>
#include <qdatetime.h>
#include <qthread.h>
#include <Qt3Support/Q3PtrQueue>

#ifndef WIN32
	#include <netinet/in.h>
	#include <arpa/inet.h>
#else
	#include <winsock2.h>
#endif // WIN32

#include <stdio.h>

class MediaThread : public QThread {
public:
    MediaThread( MediaStream *mediaStream ):
        QThread() 
    {
        this->mediaStream = mediaStream;
    }

    virtual void run() {
        while ( mediaStream->isRunning() ) {
            mediaStream->timerClick();
            msleep(1);
        }
    }

    MediaStream *mediaStream;
};


//#define TEST_AUDIO

class MediaStream::Private
{
public:
    RTPSession *session;

    // output network packets buffer
    char outBuffer[ 160*2*512 ];
    int outBufferPos, outBufferTime;

    PortAudioStream *audioStream;

    RingBuffer *micBuffer, *dspBuffer;

    VoiceEncoder *encoder;
    VoiceDecoder *decoder;

    int codecPayload;


	JabbinTransmissionParams transparams;

    bool sendPacketsFlag;

    QTimer timer;
    bool isRunning;
    //QMutex mutex;
    MediaThread *processThread;

};

MediaStream::MediaStream(QObject * parent, const char * name)
: QObject( parent, name )
{
    d = new Private;
    d->micBuffer = new RingBuffer(4*2*8000);
    d->dspBuffer = new RingBuffer(4*2*8000);
    d->audioStream = 0;
    d->codecPayload = -1;
    d->isRunning = false;
    d->processThread = new MediaThread(this);

    d->session = new RTPSession( RTPTransmitter::JabbinProto );

    connect( &d->timer, SIGNAL(timeout()), SLOT(timerClick()) );
}

MediaStream::~MediaStream()
{
    if ( isRunning() )
        stop();

    delete d->micBuffer;
    delete d->dspBuffer;
    delete d->processThread;
    delete d;
    d = 0;
}


static int audioCallback( void *inputBuffer, void *outputBuffer, // {{{
                       unsigned long framesPerBuffer, PaTimestamp outTime, void *userData )
{
    //qDebug("audio callback");
    //char *out = (char *) outputBuffer;
    //char *in  = (char *) inputBuffer;
    MediaStream::Private *d = (MediaStream::Private*)userData;

    // to test audio, uncomment next 2 lines
    //memcpy( outputBuffer, inputBuffer, framesPerBuffer*2 );
    //return 0;

    Q_UNUSED( outTime );

    d->micBuffer->lock();
    d->micBuffer->put( (char*)inputBuffer, framesPerBuffer*2 );
    d->micBuffer->unlock();

    memset(outputBuffer, 0 , framesPerBuffer*2 );
    
    d->dspBuffer->lock();
    unsigned int dataSize = d->dspBuffer->size();
    //qDebug("send %d samples to dsp", dataSize/2 );
    
    if ( dataSize >= framesPerBuffer*2 ) {
        memcpy( outputBuffer, d->dspBuffer->data(), framesPerBuffer*2 );
        d->dspBuffer->fetch( framesPerBuffer*2 );
    } else {
        if ( dataSize > 0 ) {
            memcpy( outputBuffer, d->dspBuffer->data(), (dataSize/2)*2 );
            d->dspBuffer->fetch( dataSize );
            // Zero out remainder of buffer if we run out of data.
            // it's already 0 filled with prev memset
            //for( unsigned int i=dataSize/2; i<framesPerBuffer; i++ ) {
            //    out[i] = 0;
            //}
        }
    }
    
    d->dspBuffer->unlock();
        
    return 0;
} // }}}

 
void MediaStream::start( Q3PtrQueue<QByteArray> *incomingPackets, cricket::MediaChannel *mediaChannel, int codecPayload ) // {{{
{
    if ( isRunning() )
        stop();

    d->outBufferPos = 0;
    d->outBufferTime = 0;

    int localPort = 3000;

    VoiceCodecFactory *factory = CodecsManager::instance()->codecFactory(codecPayload);
    if ( !factory ) {
        return;
    }

    d->codecPayload = codecPayload;
    d->decoder =  factory->decoder();
    d->encoder =  factory->encoder();


    // Now, we'll create a RTP session and set the destination
    d->transparams.mediaChannel = mediaChannel;
    d->transparams.incomingPackets = incomingPackets;
	
    RTPSessionParams sessparams;

    sessparams.SetOwnTimestampUnit(1.0/8000.0); // 8KHz
	sessparams.SetAcceptOwnPackets(true);
	//transparams.SetBindIP(localIP);
	//transparams.SetPortbase(localPort);

    d->session->Destroy();
    
    int status = d->session->Create( sessparams, &d->transparams );

    if ( status<0 ) {
        qDebug("can't create RTP session, %s", RTPGetErrorString(status).c_str() );
        d->session->Destroy(); 
        return;
    }

    RTPIPv4Address addr;
	status = d->session->AddDestination(addr);
    
    if ( status<0 ) {
        qDebug("can't add rtp destination, %s", RTPGetErrorString(status).c_str() );
        d->session->Destroy();
        return;
    }

    //initialise audio

    status = Pa_Initialize();
    if( status != paNoError ) {
        qDebug( "PortAudio error: %s", Pa_GetErrorText(status) );
        stop();
        return;
    }

    status = Pa_OpenDefaultStream(
        &d->audioStream,/* passes back stream pointer */
        1,              /* 1 input channel */
        1,              /* mono output */
        paInt16,        /* 16 bit fixed point output */
        8000,           /* sample rate */
        240,            /* frames per buffer */
        16,             /* number of buffers, if zero then use default minimum */
        audioCallback,  /* specify our custom callback */
        d );            /* pass our data through to callback */

    status = Pa_StartStream( d->audioStream );
    if( status != paNoError ) {
        qDebug( "PortAudio error: %s", Pa_GetErrorText(status) );
        stop();
        return;
    }

    

    // put something to dsp buffer
    /*
    char emptyData[160*8];
    memset(  emptyData, 1, sizeof(emptyData) );
    d->dspBuffer->lock();
    d->dspBuffer->put( emptyData, sizeof(emptyData) );
    d->dspBuffer->unlock();
    */



    //d->timer.start(1,false);
    d->isRunning = true;
    d->processThread->start();
    
    qDebug("mediastream started");
        
} // }}}


void MediaStream::stop()
{
    d->isRunning = false;

    d->processThread->wait();

    
    //d->mutex.lock(); //wait for thread to stop
    
    //d->timer.stop();
    if ( d->audioStream ) {
        Pa_CloseStream( d->audioStream );
        d->session->Destroy(); 
    }
    d->audioStream = 0;
    
    //d->mutex.unlock();

    qDebug("mediastream terminated");
}
//mic/dsp mixer levels, 0..100
extern int call_dlg_mic_level;
extern int call_dlg_dsp_level;


bool MediaStream::isRunning()
{
    return d->isRunning;
}

void MediaStream::setSend(bool send)
{
    d->sendPacketsFlag = send;
    qDebug("MediaStream::setSend : %d", send);
}


//called on timer
//all data is processed here
void MediaStream::timerClick()
{
//d->mutex.lock();

#ifndef TEST_AUDIO    
    int status = d->session->Poll();
    if ( status<0 ) {
        qDebug("Poll: %s", RTPGetErrorString(status).c_str() );
    }

    //checkRtpError( status );
    // check incoming packets
    if ( d->session->GotoFirstSourceWithData() ) {

        //qDebug("have rtp data");
        do {
            //RTPSourceData *sourceData = d->session.GetCurrentSourceInfo();

            RTPPacket *pack;
            if ((pack = d->session->GetNextPacket()) != NULL) {
                //qDebug("Get packet N %ld", pack->GetExtendedSequenceNumber());

                //DKZM:to free debug log
                //debug("Received packet %ld with payload type %d, size %d",  
                  //     pack->GetExtendedSequenceNumber(), pack->GetPayloadType(), pack->GetPayloadLength() );

                // TODO initialise decoder here using pack payload type, maybe use QIntDict of decoders
                if ( d->decoder ) {
                    short* decodedData = 0;

                    int size = d->decoder->decode((char*)pack->GetPayloadData(), pack->GetPayloadLength(), &decodedData );

                    if ( size > 0 ) {
                        //DKZM:to free debug log
			 //qDebug("decoded data (%d byes) with payload type %d",  size*2, pack->GetPayloadType() );

                        // adjust the volume
                        for ( int i=0; i<size; i++ ) {
                            double val = double(decodedData[i]) * call_dlg_dsp_level / 50.0;
                            if ( val > 32700.0 )
                                val = 32700.0;
                            if ( val < -32700.0 )
                                val = -32700.0;

                            decodedData[i] = short(val);
                        }
                        
                        // write to DSP buffer
                        d->dspBuffer->lock();
                        d->dspBuffer->put( (char*)decodedData, size*2 );
                        d->dspBuffer->unlock();
                        
                        processDspData(decodedData,size);

                        delete[] decodedData;
                    }


                } else {
                    qDebug("can't decode data with payload type %d", pack->GetPayloadType() );
                }

                // we don't longer need the packet, so
                // we'll delete it
                delete pack;
            }
        } while ( d->session->GotoNextSourceWithData());
    }

    // send the packet
    // check for in data


    short *data = 0;
    int micDataSize = 0; // size of readed mic data in samples

    d->micBuffer->lock();
    micDataSize = d->micBuffer->size()/2;
    if ( micDataSize ) {
        data = new short[micDataSize];
        memcpy( data, d->micBuffer->data(), micDataSize*2 );
        d->micBuffer->fetch( micDataSize*2 );
    }
    d->micBuffer->unlock();

    // adjust mic volume
    for ( int i=0; i<micDataSize; i++ ) {
        double val = double(data[i]) * call_dlg_mic_level / 50.0;
        if ( val > 32700.0 )
            val = 32700.0;
        if ( val < -32700.0 )
            val = -32700.0;

        data[i] = short(val);
    }



    // examine the data here, to calculate levels
    processMicData(data, micDataSize);


    if ( data ) {
        char * encodedData = 0;
        //int readed = micDataSize;
        int size = 0;

        //qDebug("have mic data %d", micDataSize );
        
        
        do {
            int readed = 0;
            size = d->encoder->encode( data, micDataSize, &encodedData, &readed );

            int localPayload = d->codecPayload; // TODO get local payload here
             //DKZM:to free debug log
            //qDebug("readed %d  encoded %d", readed, size );

            delete[] data;
            data = 0;
            micDataSize = 0;

            // TODO: for pcmu packet (payload==0) send packets of certain size
            if ( size > 0 ) {
                memcpy( d->outBuffer+d->outBufferPos, encodedData, size );
                d->outBufferPos += size;
                d->outBufferTime += readed;
                if ( d->outBufferPos ) {
                    //checkRtpError( 

                    if ( d->session->IsActive() && d->sendPacketsFlag ) {
                        //DKZM:to free debug log
			//qDebug("send %d bytes packet", d->outBufferPos );
                        int status = d->session->SendPacket( (void *)d->outBuffer, (int)d->outBufferPos, (unsigned char)localPayload , false, (long)d->outBufferTime );
                        if ( status<0 ) {
                            qDebug("Can't SendPacket, %s", RTPGetErrorString(status).c_str() );
                        }
                    }
                    if ( !d->session->IsActive() )
                        qDebug("session is not active");
                    //qDebug("sent packet");
                }

                    

                d->outBufferPos = 0;
                d->outBufferTime = 0;
            }

            if ( encodedData ) {
                delete[] encodedData;
                encodedData = 0;
            }

        } while (size > 0);
    }
    
    status = d->session->Poll();
    if ( status<0 ) {
        qDebug("Poll: %s", RTPGetErrorString(status).c_str() );
    }
#else // TEST_AUDIO

    short *data = 0;
    int micDataSize = 0; // size of readed mic data in samples

    d->micBuffer->lock();
    micDataSize = d->micBuffer->size()/2;
    if ( micDataSize ) {
        data = new short[micDataSize];
        memcpy( data, d->micBuffer->data(), micDataSize*2 );
        d->micBuffer->fetch( micDataSize*2 );
    }
    d->micBuffer->unlock();

    if (data) {
        // write to DSP buffer
        d->dspBuffer->lock();
        d->dspBuffer->put( (char*)data,micDataSize*2 );
        d->dspBuffer->unlock();

    }

    static int totalSamples = 0;
    totalSamples += micDataSize;


    if ( micDataSize )
        printf("total audio samples: %d  %d   \r", micDataSize, totalSamples);
    

#endif // TEST_AUDIO
    
//    d->mutex->unlock();
}



void MediaStream::processMicData(short* data, int size)
{
    int max = 0;
    long sum = 0;
    if (!size) return;
    for (int i=0;i<size;i++) {
        sum += abs(data[i]);
        if (abs(data[i]) > max)
            max = abs(data[i]);
    }
    emit micLevel(sum/size, max);
    //qDebug("mic --- %i", int(sum/size));
}

void MediaStream::processDspData(short* data, int size)
{
    int max =0;
    long sum = 0;
    if (!size) return;
    for (int i=0;i<size;i++) {
        sum += abs(data[i]);
        if (abs(data[i]) > max)
            max = abs(data[i]);
    }
    emit dspLevel(sum/size, max);
    //qDebug("dsp --- %i", int(sum/size));
}


