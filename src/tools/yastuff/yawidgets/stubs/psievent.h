#ifndef PSIEVENT_H
#define PSIEVENT_H

class PsiAccount;

#include "xmpp_jid.h"

class PsiEvent
{
public:
	PsiEvent() {}
	PsiAccount* account() const { return 0; }
	QString from() const { return QString(); }
	QString description() const { return QString(); }
	XMPP::Jid jid() const { return XMPP::Jid(); }

	enum Type {
		Auth = 0
	};

	Type type() const { return Auth; }
};

#endif
