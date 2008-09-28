#ifndef XMPP_JID_H
#define XMPP_JID_H

namespace XMPP {
	class Jid {
	public:
		Jid(QString) {}
		Jid() {}
		bool isValid() { return true; }
	};
};

#endif
