#ifndef JINGLEVOICECALLER_H
#define JINGLEVOICECALLER_H

#include <qmap.h>

#include "im.h"
#include "voicecaller.h"

//#include "talk/session/phone/call.h"

using namespace XMPP;

class PsiAccount;

namespace talk_base {
	class SocketServer;
	class Thread;
	class NetworkManager;
	class SessionManager;
	class PhoneSessionClient;
	class Call;
	class SocketAddress;
}

namespace cricket {
	class SessionManager;
	class PhoneSessionClient;
	class BasicPortAllocator;
	class HttpPortAllocator;
    class Call;
}

class JingleClientSlots;
class JingleCallSlots;


class JingleVoiceCaller : public VoiceCaller
{
	Q_OBJECT

	friend class JingleClientSlots;

public:
	JingleVoiceCaller(PsiAccount* account);
	~JingleVoiceCaller();

	virtual bool calling(const Jid&);

	virtual void initialize();
	virtual void deinitialize();

	virtual void call(const Jid&);
	virtual void accept(const Jid&);
	virtual void reject(const Jid&);
	virtual void terminate(const Jid&);

        virtual void sendDTMF(const Jid& j, const QString & dtmfCode );
        virtual void sendDTMF(cricket::Call * call);

protected:
	void sendStanza(const char*);
	void registerCall(const Jid&, cricket::Call*);
	void removeCall(const Jid&);

protected slots:
	void receiveStanza(const QString&);

private:
	bool initialized_;
	static talk_base::SocketServer *socket_server_;
	static talk_base::Thread *thread_;
	static talk_base::NetworkManager *network_manager_;
	static cricket::HttpPortAllocator *port_allocator_;
	static talk_base::SocketAddress *stun_addr_;
	cricket::SessionManager *session_manager_;
	cricket::PhoneSessionClient *phone_client_;
	JingleClientSlots *slots_;
	QMap<QString,cricket::Call*> calls_;
        QMap<cricket::Call*, QString> phoneCalls_;
};

#endif
