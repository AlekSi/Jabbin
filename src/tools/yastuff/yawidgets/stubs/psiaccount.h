#ifndef PSIACCOUNT_H
#define PSIACCOUNT_H

#include "xmpp_jid.h"

class PsiAccount
{
public:
	PsiAccount() {}

	XMPP::Jid jid() const { return XMPP::Jid(); }

	void dj_addAuth(const XMPP::Jid&) {}
	void dj_deny(const XMPP::Jid&) {}

	PsiAccount* eventQueue() const { return (PsiAccount*)this; }
	void dequeue(PsiEvent*) const {}

	QString nick() const { return QString(); }
};

#endif
