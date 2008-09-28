#ifndef VCARDFACTORY_H
#define VCARDFACTORY_H

#include "xmpp_jid.h"

namespace XMPP {
	class VCard;
}

class PsiAccount;

class VCardFactory
{
public:
	VCardFactory() {}

	static VCardFactory* instance() { return 0; }

	static XMPP::VCard* vcard(XMPP::Jid jid) { return 0; }
	static void setVCard(const PsiAccount*, const XMPP::VCard&) {}
};

#endif
