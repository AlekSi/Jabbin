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

#include "jabbinmediaengine.h"

#include <sys/types.h>
#ifndef WIN32
    #include <unistd.h>
#endif
#include <fcntl.h>

#ifndef WIN32
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
#else
	#include <winsock2.h>
#endif // WIN32

#include "mediastream.h"
#include "voicecodec.h"

#include "codecs/pcmucodec.h"

using namespace cricket;

JabbinMediaChannel::JabbinMediaChannel()
: MediaChannel()
{
  pt_ = -1;
  dying_ = false;
  SetInterface( NULL );

  qDebug( "Network interface: %d", (void *)network_interface() );

  mediaStream = new MediaStream(0);
}

JabbinMediaChannel::~JabbinMediaChannel() {
  dying_ = true;
  SetInterface( NULL );

  mediaStream->stop();
  delete mediaStream;
}

void JabbinMediaChannel::SetCodec(int pt) {

   /*
  if (!strcmp(codec, "iLBC"))
    pt_ = 102;
  else if (!strcmp(codec, "speex"))
    pt_ = 98;
  else
    pt_ = 0;

*/
  pt_ = pt;
  qDebug("set codec: , payload type: %d", pt_);
  qDebug( "Network interface: %d", (void *)network_interface() );

  mediaStream->stop();
  mediaStream->start( &incomingPackets, this, pt_ );
}

void JabbinMediaChannel::SetCodecs(const std::vector<Codec> &codecs)
{
    if ( codecs.size() ) {
        SetCodec( codecs[0].id );
    }
}

void JabbinMediaChannel::OnPacketReceived(const void *data, int len)
{

  char buf[4096];
  memcpy(buf, data, len);

  if (buf[1] == pt_) {
  } else {
      SetCodec(buf[1]);
      /*
      if (buf[1] == 13) {
      } else if (buf[1] == 102) {
        SetCodec("iLBC");
      } else if (buf[1] == 98 ) {
        SetCodec("speex");
      } else if (buf[1] == 0) {
        SetCodec("PCMU");
      }*/
  }

  if (play_ && buf[1] != 13) { // 13 == comfort noise
      QByteArray *ba = new QByteArray(len);
      memcpy( ba->data(), data, len );
      incomingPackets.enqueue( ba );
      //DKZM:to free debug log
      //qDebug("enqueue incoming packet, size %d", len);
  }
}

void JabbinMediaChannel::SetPlayout(bool playout) {
  play_ = playout;
}

void JabbinMediaChannel::SetSend(bool send) {
  mute_ = !send;
  mediaStream->setSend(send);
}

//float JabbinMediaChannel::GetCurrentQuality() { return 0.0; }
int JabbinMediaChannel::GetOutputLevel() { return 12;  } // just a testing value

JabbinMediaEngine::JabbinMediaEngine() 
{
	// Initialize media backend (to-do: use bool result to invalidate JabbinMediaEngine on failure)
	MediaStream::_BackendInitialize();
}
JabbinMediaEngine::~JabbinMediaEngine() {}

bool JabbinMediaEngine::Init() {
  // codecs_.push_back(Codec(102, "iLBC", 4));
  // codecs_.push_back(Codec(98, "speex", 4));
  // codecs_.push_back(Codec(0, "PCMU", 2));

return true;
}

void JabbinMediaEngine::Terminate() 
{
	// Destroy media backend and cleanup
	MediaStream::_BackendDestroy();
}

MediaChannel *JabbinMediaEngine::CreateChannel() {
  return new JabbinMediaChannel();
}

int JabbinMediaEngine::SetAudioOptions(int options) {return 0;}
int JabbinMediaEngine::SetSoundDevices(int wave_in_device, int wave_out_device) {return 0;}

float JabbinMediaEngine::GetCurrentQuality() {return 0.0;}
int JabbinMediaEngine::GetInputLevel() {return 0;}


std::vector<Codec> JabbinMediaEngine::codecs()
{
    std::vector<Codec> res;

    CodecsManager *manager = CodecsManager::instance();
    for ( int i=0; i<manager->payloads().count(); i++ ) {
        VoiceCodecFactory *f = manager->codecFactory( manager->payloads()[i] );

        Codec codec( f->payload(),
                     std::string(f->name().ascii()),
                     8000, //clockrate
                     int( f->bandwidth()*1000 ), // bitrate
                     1, // channels
                     1 // preference
                     );
        res.push_back(codec);
    }

    return res;
}

bool JabbinMediaEngine::FindCodec(const Codec &codec)
{
    return CodecsManager::instance()->codecFactory( codec.id ) != 0;
}

