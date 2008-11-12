#ifndef JABBIN_TRANSMITTER_H

#define JABBIN_TRANSMITTER_H

#include "rtptransmitter.h"
#include <Qt3Support/Q3CString>
#include <Qt3Support/Q3PtrQueue>

namespace cricket {
class MediaChannel;
};

class JabbinTransmitter : public RTPTransmitter
{
public:
	JabbinTransmitter();
	virtual ~JabbinTransmitter();

	// The init function is there for initialization before any other threads
	// may access the object (e.g. initialization of mutexes)
	virtual int Init(bool threadsafe);
	virtual int Create(size_t maxpacksize,const RTPTransmissionParams *transparams);
	virtual void Destroy();

	// The user MUST delete the returned instance when it is no longer needed
	virtual RTPTransmissionInfo *GetTransmissionInfo();

	// If the buffersize ins't large enough, the transmitter must fill in the
	// required length in 'bufferlength'
	// If the size is ok, bufferlength is adjusted so that it indicates the
	// amount of bytes in the buffer that are part of the hostname.
	// The buffer is NOT null terminated!
	virtual int GetLocalHostName(u_int8_t *buffer,size_t *bufferlength);

	virtual bool ComesFromThisTransmitter(const RTPAddress *addr);
	virtual size_t GetHeaderOverhead();

	virtual int Poll();
	virtual int WaitForIncomingData(const RTPTime &delay);
	virtual int AbortWait();

	virtual int SendRTPData(const void *data,size_t len);
	virtual int SendRTCPData(const void *data,size_t len);

	virtual void ResetPacketCount();
	virtual u_int32_t GetNumRTPPacketsSent();
	virtual u_int32_t GetNumRTCPPacketsSent();

	virtual int AddDestination(const RTPAddress &addr);
	virtual int DeleteDestination(const RTPAddress &addr);
	virtual void ClearDestinations();

	virtual bool SupportsMulticasting();
	virtual int JoinMulticastGroup(const RTPAddress &addr);
	virtual int LeaveMulticastGroup(const RTPAddress &addr);
	virtual void LeaveAllMulticastGroups();

	// Note: the list of addresses must be cleared when the receive mode is changed!
	virtual int SetReceiveMode(RTPTransmitter::ReceiveMode m);
	virtual int AddToIgnoreList(const RTPAddress &addr);
	virtual int DeleteFromIgnoreList(const RTPAddress &addr);
	virtual void ClearIgnoreList();
	virtual int AddToAcceptList(const RTPAddress &addr);
	virtual int DeleteFromAcceptList(const RTPAddress &addr);
	virtual void ClearAcceptList();
	virtual int SetMaximumPacketSize(size_t s);

	virtual bool NewDataAvailable();
	virtual RTPRawPacket *GetNextPacket();
#ifdef RTPDEBUG
	virtual void Dump();
#endif // RTPDEBUG
private:
    class Private;
    Private *d;
};


class JabbinTransmissionParams: public RTPTransmissionParams
{
public:
	JabbinTransmissionParams()
        :RTPTransmissionParams(RTPTransmitter::JabbinProto) /*, sendPacketsFlag(false)*/ {}
	virtual ~JabbinTransmissionParams() { }


    //void *incomingPackets; //QPtrQueue<QByteArray> *
    Q3PtrQueue<QByteArray> * incomingPackets;
    //cricket::MediaChannel::NetworkInterface *iface;
    //void *iface;           //cricket::MediaChannel::NetworkInterface *
    cricket::MediaChannel *mediaChannel;

    //bool sendPacketsFlag;

};



#endif

