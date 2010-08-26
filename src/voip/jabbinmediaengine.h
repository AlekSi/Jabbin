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
 
#ifndef JABBIN_MEDIAENGINE_H_
#define JABBIN_MEDIAENGINE_H_

#include "talk/session/phone/mediaengine.h"
#include "talk/session/phone/codec.h"

#include <QQueue>

class MediaStream;

namespace cricket {

class JabbinMediaChannel : public MediaChannel {
 public:
  JabbinMediaChannel();
  virtual ~JabbinMediaChannel();
  virtual void SetCodec(int payload);
  virtual void SetCodecs(const std::vector<Codec> &codecs);
  virtual void OnPacketReceived(const void *data, int len);

  virtual void SetPlayout(bool playout);
  virtual void SetSend(bool send);

  //virtual float GetCurrentQuality();
  virtual int GetOutputLevel();
  //int fd() {return fd_;}
  bool mute() {return mute_;}
  bool dying() {return dying_;}

  virtual void StartMediaMonitor(VoiceChannel * voice_channel, uint32 cms) {};
  virtual void StopMediaMonitor() {};

 private:
  MediaStream *mediaStream;
  QQueue<QByteArray> incomingPackets;

  //pthread_t thread_;
  //int fd_;
  int pt_;
  bool dying_;
  bool mute_;
  bool play_;
};

class JabbinMediaEngine : public MediaEngine {
 public:
  JabbinMediaEngine();
  virtual ~JabbinMediaEngine();
  virtual bool Init();
  virtual void Terminate();
  
  virtual MediaChannel *CreateChannel();

  virtual int SetAudioOptions(int options);
  virtual int SetSoundDevices(int wave_in_device, int wave_out_device);

  virtual float GetCurrentQuality();
  virtual int GetInputLevel();

  virtual std::vector<Codec> codecs();
  virtual bool FindCodec(const Codec &codec);
 
};

}  // namespace cricket

#endif //JABBIN_MEDIAENGINE_H_

