/*
    Jabbin - Jabber and VoIP client
    Copyright (C) 2006 Dmitry Poplavsky <pdima@users.sourceforge.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include "jabbintransmitter.h"

#include <string>
#include <vector>

#include "talk/session/phone/mediachannel.h"

#include "rtpipv4address.h"
#include "rtptimeutilities.h"
#include "rtprawpacket.h"




class JabbinTransmitter::Private
{
public:
    Private() {
        mediaChannel = 0;
        rtpPacketsCount = 0;
    }

    QQueue<QByteArray> incomingPackets;
    cricket::MediaChannel *mediaChannel;

    int rtpPacketsCount;

};


JabbinTransmitter::JabbinTransmitter()
{
    d = new Private;
}

JabbinTransmitter::~JabbinTransmitter()
{
    delete d;
}

// The init function is there for initialization before any other threads
// may access the object (e.g. initialization of mutexes)
int JabbinTransmitter::Init(bool threadsafe)
{
    return 0;
}

int JabbinTransmitter::Create(size_t maxpacksize,const RTPTransmissionParams *transparams)
{
    const JabbinTransmissionParams *jParams =  dynamic_cast<const JabbinTransmissionParams*>(transparams);

    d->incomingPackets = jParams->incomingPackets; //(QPtrQueue<QByteArray> *)(jParams->incomingPackets);
    d->mediaChannel = jParams->mediaChannel;
    return 0;
}

void JabbinTransmitter::Destroy()
{
}

// The user MUST delete the returned instance when it is no longer needed
RTPTransmissionInfo * JabbinTransmitter::GetTransmissionInfo()
{
    return 0;
}

// If the buffersize ins't large enough, the transmitter must fill in the
// required length in 'bufferlength'
// If the size is ok, bufferlength is adjusted so that it indicates the
// amount of bytes in the buffer that are part of the hostname.
// The buffer is NOT null terminated!
int JabbinTransmitter::GetLocalHostName(u_int8_t *buffer,size_t *bufferlength)
{
    strncpy( (char*)buffer, "localhost", *bufferlength );
    if ( *bufferlength < strlen("localhost") )
        *bufferlength = strlen("localhost");
    return 0;
}

bool JabbinTransmitter::ComesFromThisTransmitter(const RTPAddress *addr)
{
    return false;
}

size_t JabbinTransmitter::GetHeaderOverhead()
{
    return 10;
}

int JabbinTransmitter::Poll()
{
    return 0;
}

int JabbinTransmitter::WaitForIncomingData(const RTPTime &delay)
{
    return 0;
}

int JabbinTransmitter::AbortWait()
{
    return 0;
}

int JabbinTransmitter::SendRTPData(const void *data,size_t len)
{

    if ( d->mediaChannel && d->mediaChannel->network_interface() ) {
	//DKZM:to free debug log
        //qDebug( "JabbinTransmitter: send packet, len %d", len );
        //qDebug( "mediaChannel->GetOutputLevel(): %d", d->mediaChannel->GetOutputLevel() );
        //cricket::MediaChannel::NetworkInterface *networkInterface = d->mediaChannel->network_interface();

        //qDebug( "packet of length %d was intended to be sent via %d", len, int( networkInterface ) );
        //networkInterface->SendPacket(data,len);
        //
        d->mediaChannel->network_interface()->SendPacket(data,len);
    }
    //d->iface->SendPacket(data, len);
    d->rtpPacketsCount++;
    return 0;
}

int JabbinTransmitter::SendRTCPData(const void *data,size_t len)
{
    return 0;
}

void JabbinTransmitter::ResetPacketCount()
{
    d->rtpPacketsCount = 0;
}

u_int32_t JabbinTransmitter::GetNumRTPPacketsSent()
{
    return d->rtpPacketsCount;
}

u_int32_t JabbinTransmitter::GetNumRTCPPacketsSent()
{
    return 0;
}

int JabbinTransmitter::AddDestination(const RTPAddress &addr)
{
    return 0;
}

int JabbinTransmitter::DeleteDestination(const RTPAddress &addr)
{
    return 0;
}

void JabbinTransmitter::ClearDestinations() {}

bool JabbinTransmitter::SupportsMulticasting() { return false; }

int JabbinTransmitter::JoinMulticastGroup(const RTPAddress &addr) { return 0; }
int JabbinTransmitter::LeaveMulticastGroup(const RTPAddress &addr) { return 0; }
void JabbinTransmitter::LeaveAllMulticastGroups() {}

// Note: the list of addresses must be cleared when the receive mode is changed!
int JabbinTransmitter::SetReceiveMode(RTPTransmitter::ReceiveMode m) { return 0; }
int JabbinTransmitter::AddToIgnoreList(const RTPAddress &addr) { return 0; }
int JabbinTransmitter::DeleteFromIgnoreList(const RTPAddress &addr){ return 0; }
void JabbinTransmitter::ClearIgnoreList() {}
int JabbinTransmitter::AddToAcceptList(const RTPAddress &addr) { return 0; }
int JabbinTransmitter::DeleteFromAcceptList(const RTPAddress &addr) { return 0; }
void JabbinTransmitter::ClearAcceptList() {}
int JabbinTransmitter::SetMaximumPacketSize(size_t s) { return 0; }

bool JabbinTransmitter::NewDataAvailable()
{
    return !d->incomingPackets.isEmpty();
}

RTPRawPacket*  JabbinTransmitter::GetNextPacket()
{
    if ( !NewDataAvailable() )
        return 0;

    QByteArray data = d->incomingPackets.dequeue();
    int recvlen = data.size();
    u_int8_t *datacopy =  new u_int8_t[recvlen+1];
    memcpy(datacopy, data.data() ,recvlen);

    //RTPIPv4Address *addr = new RTPIPv4Address();
    RTPTime curtime = RTPTime::CurrentTime();

    RTPRawPacket *packet = new  RTPRawPacket(datacopy,recvlen,0,curtime,true);

    return packet;
}

