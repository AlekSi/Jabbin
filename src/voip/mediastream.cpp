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

/*
	Copyright (C) 2010 Dmitry Kostjuchenko <dmitrykos@iauxsoft.com>
	Web: http://www.iauxsoft.com

	Comments:
		Major rewrite of media module:
		 - optimized setting volume to streams
		 - optimized DSP peak and average processing
		 - implemented PortAudio V19 support
		 - optimized memory operations
		 - cleaned up and structurized source-code
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
#include <portaudio.h>

#include "voicecodec.h"
#include "sdp.h"
#include "ringbuffer.h"

// Qt
#include <qthread.h>

// Network
#ifndef WIN32
	#include <netinet/in.h>
	#include <arpa/inet.h>
#else
	#include <winsock2.h>
#endif // WIN32

#include <stdio.h>
#include <assert.h>

//! Missing NULL?
#ifndef NULL
	#define NULL 0
#endif

//! Local types
typedef char					md_int8;
typedef unsigned char			md_uint8;
typedef short					md_int16;
typedef unsigned short			md_uint16;
typedef int						md_int32;
typedef unsigned int			md_uint32;
#ifdef __BORLANDC__ //<< older BCC compiler workaround
	typedef __int64				md_int64;
	typedef unsigned __int64	md_uint64;
#else
	typedef long long			md_int64;
	typedef unsigned long long  md_uint64;
#endif

//! Restrict (to-do: add corresponding GCC define)
#if defined(_MSC_VER) || (defined(_INTEL_COMPILER) || defined(_ICC))
	#define md_restrict __restrict
#endif

//! Local defines
#define _MEDIA_SAMPLE_TYPE				md_int16
#define _MEDIA_FORMAT_TYPE				paInt16
#define _MEDIA_FORMAT_CHANNELS			1
#define _MEDIA_FORMAT_SAMPLERATE		8000
#define _MEDIA_FORMAT_SAMPLE_SIZE		sizeof(_MEDIA_SAMPLE_TYPE)
#define _MEDIA_FORMAT_FRAME_SIZE		(_MEDIA_FORMAT_SAMPLE_SIZE*_MEDIA_FORMAT_CHANNELS)
#define _MEDIA_FRAMES_TO_SAMPLES(x)		((x)*_MEDIA_FORMAT_CHANNELS)
#define _MEDIA_FRAMES_TO_BYTES(x)		((x)*_MEDIA_FORMAT_FRAME_SIZE)
#define _MEDIA_SAMPLES_TO_BYTES(x)		((x)*_MEDIA_FORMAT_SAMPLE_SIZE)
#define _MEDIA_BYTES_TO_SAMPLES(x)		((x)/_MEDIA_FORMAT_SAMPLE_SIZE)
#define _MEDIA_BYTES_TO_FRAMES(x)		((x)/_MEDIA_FORMAT_FRAME_SIZE)
#define _MEDIA_RINGBUFFER_SIZE			(4*_MEDIA_SAMPLES_TO_BYTES(8000)) // 4x of SR
#define _MEDIA_FORMAT_FRAMES_PER_BUFFER 240

//! Aligns value backwards
template <class _Ty> static inline _Ty MEDIA_ALIGN_BWD(_Ty v, _Ty align)
{
	return ((v - (align ? v % align : 0)));
}

//! QT Sleep wrappper.
struct QtSleep : public QThread { static void Ms(unsigned long msec) { QThread::msleep(msec); } };
#define _MEDIA_SLEEP_MSEC(x) QtSleep::Ms((x))

//! Local statics
static PaError g_PaLibInit = paNotInitialized;

//! Externals
//! mic/dsp mixer levels, 0..100 (from: calldialog.cpp)
extern volatile int call_dlg_mic_level;
extern volatile int call_dlg_dsp_level;

// ---------------------------------------------------------------------------------------------
static void _GetClipMinMax(md_uint32 bits, md_int32 *__min_max)
{
	assert(bits <= 32 && "_GetClipMinMax: not implemented bitsize of sample (>32)");

	struct SClip { md_int32 _min, _max; };
	static SClip clip[] =
	{
		{ 0								, 0						       },
		{ -md_int32(md_uint32(0xff)      /2)-1, (md_uint32(0xff)      /2) }, // 8bit integer
		{ -md_int32(md_uint32(0xffff)    /2)-1, (md_uint32(0xffff)    /2) }, // 16bit integer
		{ -md_int32(md_uint32(0xffffff)  /2)-1, (md_uint32(0xffffff)  /2) }, // 24bit integer
		{ -md_int32(md_uint32(0xffffffff)/2)-1, (md_uint32(0xffffffff)/2) }, // 32bit integer
	};

	__min_max[0] = clip[ bits/8 ]._min;
	__min_max[1] = clip[ bits/8 ]._max;
}

// ---------------------------------------------------------------------------------------------
static void _ApplyVolume(_MEDIA_SAMPLE_TYPE * md_restrict buffer, md_uint32 count, md_int32 gain)
{
	md_int32 clip[2];
	_GetClipMinMax(sizeof(_MEDIA_SAMPLE_TYPE)*8, clip);

	md_int32 sample;
	_MEDIA_SAMPLE_TYPE *end = buffer + count;

	while (buffer != end)
	{
		sample  = (*buffer);

		// apply gain (0...middle(50)...100(max))
		sample *= gain;
		sample /= 50;

		// apply hard clipping
		if (sample < clip[0])
			sample = clip[0];
		else
		if (sample > clip[1])
			sample = clip[1];

		// set modified
		*buffer ++ = sample;
	}
}

// ---------------------------------------------------------------------------------------------
static void _ApplyVolumeWithDSP(_MEDIA_SAMPLE_TYPE * md_restrict buffer, md_uint32 count, 
	md_int32 gain, md_int32 *peak, md_int32 *avrg)
{
	md_int32 peak_v = 0;
	md_int64 sum_v  = 0;

	md_int32 clip[2];
	_GetClipMinMax(sizeof(_MEDIA_SAMPLE_TYPE)*8, clip);

	md_int32 sample;
	_MEDIA_SAMPLE_TYPE *end = buffer + count;

	while (buffer != end)
	{
		sample  = (*buffer);

		// apply gain (0...middle(50)...100(max))
		sample *= gain;
		sample /= 50;

		// apply hard clipping
		if (sample < clip[0])
			sample = clip[0];
		else
		if (sample > clip[1])
			sample = clip[1];

		// set modified
		*buffer ++ = sample;

		// get DSP values
		sample = abs(sample);
		sum_v += sample;
        if (sample > peak_v)
            peak_v = sample;
	}

	*peak = peak_v;
	*avrg = (count != 0 ? (md_int32)(sum_v/count) : 0);
}

// ---------------------------------------------------------------------------------------------
static void _GetDSPValuePeakAvrg(_MEDIA_SAMPLE_TYPE * md_restrict buffer, md_uint32 count, md_int32 *peak, 
	md_int32 *avrg)
{
	md_int32 peak_v = 0;
	md_int64 sum_v  = 0;
	_MEDIA_SAMPLE_TYPE sample;
	_MEDIA_SAMPLE_TYPE *end = buffer + count;

	while (buffer != end)
	{
		sample = abs(*buffer ++);
		sum_v += sample;
        if (sample > peak_v)
            peak_v = sample;
	}

	*peak = peak_v;
	*avrg = (count != 0 ? (md_int32)(sum_v/count) : 0);
}

// ---------------------------------------------------------------------------------------------
static void _ProcessMicData(MediaStream *stream, _MEDIA_SAMPLE_TYPE * md_restrict data, 
	md_uint32 count, md_int32 peak, md_int32 avrg, bool do_dsp)
{
	if (do_dsp)
		_GetDSPValuePeakAvrg(data, count, &peak, &avrg);

    stream->_MicLevel(avrg, peak);
    // qDebug("mic --- %i", avrg);
}

// ---------------------------------------------------------------------------------------------
static void _ProcessDspData(MediaStream *stream, _MEDIA_SAMPLE_TYPE * md_restrict data, 
	md_uint32 count, md_int32 peak, md_int32 avrg, bool do_dsp)
{
	if (do_dsp)
		_GetDSPValuePeakAvrg(data, count, &peak, &avrg);

    stream->_DspLevel(avrg, peak);
    // qDebug("mic --- %i", avrg);
}

// ---------------------------------------------------------------------------------------------
static void _ZeroAudioBuffer(_MEDIA_SAMPLE_TYPE * md_restrict buffer, md_uint32 count)
{
	_MEDIA_SAMPLE_TYPE *end = buffer + count;
	while (buffer != end)
		*buffer ++ = 0;
}

// ---------------------------------------------------------------------------------------------
class MediaThread : public QThread 
{
public:
    MediaThread(MediaStream *mediaStream) : QThread() 
    {
        this->mediaStream = mediaStream;
    }

    void run()
	{
        while (mediaStream->isRunning()) 
		{
            mediaStream->timerClick();
            msleep(1);
        }
    }

    MediaStream *mediaStream;
};

//#define TEST_AUDIO

// ---------------------------------------------------------------------------------------------
class MediaStream::Private
{
public:
    RTPSession *session;

    PaStream      *audioStream;

    RingBuffer    *micBuffer;
	RingBuffer    *dspBuffer;

    VoiceEncoder  *encoder;
    VoiceDecoder  *decoder;

    md_int32      codecPayload;

	JabbinTransmissionParams transparams;

    bool          sendPacketsFlag;
    volatile bool isRunning;

    MediaThread   *processThread;

	_MEDIA_SAMPLE_TYPE *tempMicBuffer;

	bool          paLibInitialized;
};

// ---------------------------------------------------------------------------------------------
MediaStream::MediaStream(QObject *parent, const char *name) : QObject(parent, name)
{
    d                = new Private();
    d->micBuffer     = new RingBuffer(_MEDIA_RINGBUFFER_SIZE);
    d->dspBuffer     = new RingBuffer(_MEDIA_RINGBUFFER_SIZE);
    d->audioStream   = 0;
    d->codecPayload  = -1;
    d->isRunning     = false;
    d->processThread = new MediaThread(this);
    d->session       = new RTPSession(RTPTransmitter::JabbinProto);
	d->tempMicBuffer = NULL;
}

// ---------------------------------------------------------------------------------------------
MediaStream::~MediaStream()
{
    if (isRunning())
        stop();

	// make sure thread is terminated
	d->processThread->terminate();
	bool thread_terminated = d->processThread->wait(20*1000/*20 sec timeout*/);
	assert(thread_terminated);
	Q_UNUSED(thread_terminated);

	// buffers
    delete d->micBuffer;
    delete d->dspBuffer;

	// thread handle
    delete d->processThread;

	// session handle
	delete d->session;

	// temp mic buffer
	free(d->tempMicBuffer);

	// private context
    delete d;
    d = 0;
}

// ---------------------------------------------------------------------------------------------
static int audioCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
						const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags,
						void *userData)
{
	Q_UNUSED(timeInfo);
	Q_UNUSED(statusFlags);

    // qDebug("audio callback");

    // to test audio, uncomment next 2 lines
    // memcpy( outputBuffer, inputBuffer, framesPerBuffer*2 );
    // return 0;
	
    MediaStream::Private *d = (MediaStream::Private *)userData;
	const md_uint32 bytesPerBuffer = _MEDIA_FRAMES_TO_BYTES(framesPerBuffer);

	// check if MediaStream is running, or stop processing
	if (!d->isRunning)
		return paAbort;

	// put input into buffer
    d->micBuffer->lock();
    d->micBuffer->put((char *)inputBuffer, bytesPerBuffer);
    d->micBuffer->unlock();

	// lock output buffer
    d->dspBuffer->lock();

	// get current available buffer size
    const md_uint32 dataSize = d->dspBuffer->size();
    
	// qDebug("send %d samples to dsp", dataSize/2 );
    
	// consume full buffer length
    if (dataSize >= bytesPerBuffer)
	{
        memcpy(outputBuffer, d->dspBuffer->data(), bytesPerBuffer);
        d->dspBuffer->fetch(bytesPerBuffer);
    }
	else
	// consume what buffer has, zero tail
	if (dataSize != 0)
	{
		// make sure data is aligned to frame size
		const md_uint32 writeSize = MEDIA_ALIGN_BWD(dataSize, _MEDIA_FORMAT_FRAME_SIZE);
		
		// copy to output
		memcpy(outputBuffer, d->dspBuffer->data(), writeSize);

		// zero tail
		memset(((char *)outputBuffer) + writeSize, 0, (bytesPerBuffer - writeSize));

		// consume only written size, leave left unaligned data for next turn to avoid clicks
		d->dspBuffer->fetch(writeSize);
    }
	// nothing in buffer, fill with silence
	else
	{
		_ZeroAudioBuffer((_MEDIA_SAMPLE_TYPE *)outputBuffer, _MEDIA_FRAMES_TO_SAMPLES(framesPerBuffer));
	}
    
	// unlock output buffer
    d->dspBuffer->unlock();
        
    return paContinue;
}

// ---------------------------------------------------------------------------------------------
void MediaStream::start(Q3PtrQueue<QByteArray> *incomingPackets, cricket::MediaChannel *mediaChannel, 
						int codecPayload)
{
	// check PA lib initialized
	if (g_PaLibInit != paNoError)
	{
		qDebug("media: PortAudio library not initialized: init error[%d|%s]", g_PaLibInit, Pa_GetErrorText(g_PaLibInit));
        return;
	}

	// running? stop
    if (isRunning())
        stop();

	// get encoder/decoder factory
    VoiceCodecFactory *factory = CodecsManager::instance()->codecFactory(codecPayload);
    if (factory == NULL) 
	{
		qDebug("media: no VoiceCodecFactory is found for %d", codecPayload);
        return;
    }

    d->codecPayload = codecPayload;
    d->decoder      = factory->decoder();
    d->encoder      = factory->encoder();

    // Now, we'll create a RTP session and set the destination
    d->transparams.mediaChannel    = mediaChannel;
    d->transparams.incomingPackets = incomingPackets;
	
    RTPSessionParams sessparams;
    sessparams.SetOwnTimestampUnit(1.0/_MEDIA_FORMAT_SAMPLERATE); // 8KHz
	sessparams.SetAcceptOwnPackets(true);
	//transparams.SetBindIP(localIP);
	//transparams.SetPortbase(localPort); /*3000*/

	// destroy previous
    d->session->Destroy();
    
	// create session
    md_int32 status;
    if ((status = d->session->Create(sessparams, &d->transparams)) < 0)
	{
		qDebug("media: can't create RTP session, %s", RTPGetErrorString(status).c_str());
        d->session->Destroy();
        return;
    }

	// set destination
    RTPIPv4Address addr;
    if ((status = d->session->AddDestination(addr)) < 0) 
	{
        qDebug("media: can't add rtp destination, %s", RTPGetErrorString(status).c_str());
        d->session->Destroy();
        return;
    }

	// try open default audio device (for full-duplex we must have same number of channels)
    status = Pa_OpenDefaultStream(
        &d->audioStream,				/* passes back stream pointer */
        _MEDIA_FORMAT_CHANNELS,			/* input channels */
        _MEDIA_FORMAT_CHANNELS,			/* output channels */
        _MEDIA_FORMAT_TYPE,				/* fixed point output */
        _MEDIA_FORMAT_SAMPLERATE,		/* sample rate */
        _MEDIA_FORMAT_FRAMES_PER_BUFFER,/* frames per buffer */
        audioCallback,					/* specify our custom callback */
        d);								/* pass our data through to callback */

	// check if suceeded
    if (status != paNoError) 
	{
		qDebug("media: PortAudio failed on Pa_OpenDefaultStream: error: %s", Pa_GetErrorText(status));
        d->session->Destroy();
		Pa_CloseStream(d->audioStream);
		d->audioStream = NULL;
        return;   
	}

	// start stream
    if ((status = Pa_StartStream(d->audioStream)) != paNoError) 
	{
		qDebug("media: PortAudio failed on Pa_StartStream: error: %s", Pa_GetErrorText(status));
        d->session->Destroy();
		Pa_CloseStream(d->audioStream);
		d->audioStream = NULL;
        return;
    }

	// start processing thread
    d->isRunning = true;
    d->processThread->start();
    
    qDebug("media: stream started");
}

// ---------------------------------------------------------------------------------------------
void MediaStream::stop()
{
	// make not running
    d->isRunning = false;

	// wait for thread to stop
    d->processThread->wait();

	// close stream
    if (d->audioStream != NULL) 
	{
		// abort stream
		Pa_AbortStream(d->audioStream);

		// busy wait
		md_uint32 timeout = 30*1000;
		while (timeout--)
		{
			if ((Pa_IsStreamStopped(d->audioStream) != 0) || 
				(Pa_IsStreamActive(d->audioStream)  != 1))
				break; // stream is guaranteed stopped

			_MEDIA_SLEEP_MSEC(1);
		}

		// close (failed closing stream, better leak than crash, although this situation shall not be possible)
		if (timeout == 0)
			Pa_CloseStream(d->audioStream);
        d->audioStream = NULL;
    }

	// close session
    d->session->Destroy();

	qDebug("media: stream stopped");
}

// ---------------------------------------------------------------------------------------------
bool MediaStream::isRunning()
{
    return d->isRunning;
}

// ---------------------------------------------------------------------------------------------
void MediaStream::setSend(bool send)
{
    d->sendPacketsFlag = send;
	qDebug("MediaStream::setSend : %d", (send ? 1 : 0));
}

// ---------------------------------------------------------------------------------------------
void MediaStream::timerClick()
{
#ifndef TEST_AUDIO

	// poll for data
    md_int32 status;
    if ((status = d->session->Poll()) < 0) 
	{
		qDebug("media: Poll[1] failed with error: %s", RTPGetErrorString(status).c_str());
    }

    //checkRtpError( status );

    // check incoming packets
    if (d->session->GotoFirstSourceWithData()) 
	{
        // qDebug("have rtp data");
        do 
		{
            //RTPSourceData *sourceData = d->session.GetCurrentSourceInfo();

            RTPPacket *packet;
            if ((packet = d->session->GetNextPacket()) != NULL)
			{
                //qDebug("Get packet N %ld", packet->GetExtendedSequenceNumber());

                //DKZM:to free debug log
                //debug("Received packet %ld with payload type %d, size %d",  
                // packet->GetExtendedSequenceNumber(), packet->GetPayloadType(), packet->GetPayloadLength() );

                // TODO initialise decoder here using packet payload type, maybe use QIntDict of decoders
                if (d->decoder != NULL) 
				{
                    _MEDIA_SAMPLE_TYPE *decodedData = NULL;
                    md_int32 size = d->decoder->decode((char *)packet->GetPayloadData(), packet->GetPayloadLength(), &decodedData);
                    if (size > 0) 
					{
                        //DKZM:to free debug log
						//qDebug("decoded data (%d byes) with payload type %d",  size*2, packet->GetPayloadType() );

                        // adjust the volume (optimized: 50 equals to no volume, DSP done in-place)
						if (call_dlg_dsp_level != 50)
						{
							md_int32 peak, avrg;
							_ApplyVolumeWithDSP(decodedData, size, call_dlg_dsp_level, &peak, &avrg);
							_ProcessDspData(this, decodedData, size, peak, avrg, false);
							// qDebug("dsp --- %i", avrg);
						}
						// update DSP values
						else
						{
	                        _ProcessDspData(this, decodedData, size, 0, 0, true);
						}

						// write to DSP buffer
                        d->dspBuffer->lock();
                        d->dspBuffer->put((char *)decodedData, (size * sizeof(_MEDIA_SAMPLE_TYPE)));
                        d->dspBuffer->unlock();
                        
						// to-do (OPT): optimize by reusing decodedData buffer
                        delete [] decodedData;
                    }
                }
				else 
				{
					qDebug("media: failed decoding data with payload type %d", packet->GetPayloadType());
                }

                // we don't longer need the packet, so we'll delete it
                delete packet;
            }
        } 
		while (d->session->GotoNextSourceWithData());
    }

	// next:
    //   - send the packet
    //   - check for in data

	// lock mic buffer, transfer data to intermediary buffer
    d->micBuffer->lock();
    md_uint32 micDataSamples = MEDIA_ALIGN_BWD(_MEDIA_BYTES_TO_SAMPLES(d->micBuffer->size()), _MEDIA_FORMAT_FRAME_SIZE);
	md_uint32 micDataSize    = _MEDIA_SAMPLES_TO_BYTES(micDataSamples);
	if (micDataSamples != 0)
	{
		// resize
        d->tempMicBuffer = (_MEDIA_SAMPLE_TYPE *)realloc(d->tempMicBuffer, micDataSize);
		if (d->tempMicBuffer != NULL)
		{
			// copy to temp buffer
			memcpy(d->tempMicBuffer, d->micBuffer->data(), micDataSize);
			// consume
			d->micBuffer->fetch(micDataSize);
		}
    }
    d->micBuffer->unlock();

	// adjust mic volume (optimized: 50 equals to no volume, DSP done in-place)
	if (call_dlg_mic_level != 50)
	{
		md_int32 peak, avrg;
		_ApplyVolumeWithDSP(d->tempMicBuffer, micDataSamples, call_dlg_mic_level, &peak, &avrg);
		_ProcessMicData(this, d->tempMicBuffer, micDataSamples, peak, avrg, false);
	}
	// update levels
	else
	{
		_ProcessMicData(this, d->tempMicBuffer, micDataSamples, 0, 0, true);
	}

	// encode & send
    if (micDataSamples != 0)
	{
        char *encodedData	= NULL;
        int size			= 0;
        int localPayload	= d->codecPayload; // to-do: get local payload here
		int samplesEncoded;
		int status;

        // qDebug("have mic data %d", micDataSamples);
        do 
		{
			// encode
            size = d->encoder->encode(d->tempMicBuffer, micDataSamples, &encodedData, &samplesEncoded);
			assert((md_uint32)samplesEncoded <= micDataSamples);

            // DKZM:to free debug log
            // qDebug("encoded %d  encoded %d", samplesEncoded, size);

            // to-do: for pcmu packet (payload==0) send packets of certain size
            if (size > 0)
			{
				// send packet
                if (d->sendPacketsFlag && d->session->IsActive())
				{
                    // DKZM:to free debug log
					// qDebug("send %d bytes packet", d->outBufferPos);

					// claculate time stamp increment in miliseconds
					const md_int32 timestampinc = samplesEncoded/(_MEDIA_FORMAT_SAMPLERATE/1000);

                    status = d->session->SendPacket(encodedData, (size_t)size, 
						(md_uint8)localPayload, false, timestampinc);
                    if (status < 0) 
					{
						qDebug("media: failed SendPacket with error: %s", RTPGetErrorString(status).c_str() );
                    }
				}
				else
				{
					if (!d->session->IsActive())
						qDebug("media: session is not active");
				}
                   
				//qDebug("sent packet");
             }

			// free encoder buffer (to-do: reuse buffer instead of deallocation)
            if (encodedData != NULL) 
			{
                delete [] encodedData;
                encodedData = NULL;
            }

			// deduct encoded samples
			if ((md_uint32)samplesEncoded > micDataSamples)
			{
				qDebug("media: bad encoder sequence");
				break;
			}
			micDataSamples -= (md_uint32)samplesEncoded;
        } 
		while ((micDataSamples != 0) && (size > 0));
    }

#else // TEST_AUDIO

	// lock mic buffer, transfer data to intermediary buffer
    d->micBuffer->lock();
    const md_uint32 micDataSamples = MEDIA_ALIGN_BWD(_MEDIA_BYTES_TO_SAMPLES(d->micBuffer->size()), _MEDIA_FORMAT_FRAME_SIZE);
	const md_uint32 micDataSize    = _MEDIA_SAMPLES_TO_BYTES(micDataSamples);
	if (micDataSamples != 0)
	{
		// resize
        d->tempMicBuffer = (_MEDIA_SAMPLE_TYPE *)realloc(d->tempMicBuffer, micDataSize);
		if (tempMicBuffer != NULL)
		{
			// copy to temp buffer
			memcpy(d->tempMicBuffer, d->micBuffer->data(), micDataSize);
			// consume
			d->micBuffer->fetch(micDataSize);
		}
    }
    d->micBuffer->unlock();

	// push to output buffer
    if (d->tempMicBuffer != NULL) 
	{
        // write to DSP buffer
        d->dspBuffer->lock();
        d->dspBuffer->put((char *)d->tempMicBuffer, micDataSize);
        d->dspBuffer->unlock();
    }

	// log debug info
    static volatile md_uint32 totalSamples = 0;
    totalSamples += micDataSamples;
    if (micDataSamples != 0)
        printf("audio samples: %d  %d   \r", micDataSamples, totalSamples);

#endif // TEST_AUDIO
}

// ---------------------------------------------------------------------------------------------
bool MediaStream::_BackendInitialize()
{
	qDebug("media: ===========================================================");
	qDebug("media: =======================AUDIO BACKEND=======================");
	qDebug("media: ============================+==============================");

    // initialise PA lib
    if ((g_PaLibInit = Pa_Initialize()) != paNoError) 
	{
		qDebug("media: PortAudio failed on Pa_Initialize: error[%d|%s]", g_PaLibInit, Pa_GetErrorText(g_PaLibInit));
    }

	qDebug("media: audio backend initialized");

	qDebug("media: ==========================DONE=============================");
	qDebug("media: ===========================================================");

	return (g_PaLibInit == paNoError);
}

// ---------------------------------------------------------------------------------------------
void MediaStream::_BackendDestroy()
{
	qDebug("media: ===========================================================");
	qDebug("media: =======================AUDIO BACKEND=======================");
	qDebug("media: ============================-==============================");

	// Terminate PA lib inventory
	if (g_PaLibInit == paNoError)
	{
		PaError error_id;
		if ((error_id = Pa_Terminate()) != paNoError) 
		{
			qDebug("media: PortAudio failed on Pa_Terminate: error[%d|%s]", error_id, Pa_GetErrorText(error_id));
		}
		else
		{
			g_PaLibInit = paNotInitialized;
			qDebug("media: audio backend terminated");
		}
	}

	qDebug("media: ==========================DONE=============================");
	qDebug("media: ===========================================================");
}

// -- EOF --
