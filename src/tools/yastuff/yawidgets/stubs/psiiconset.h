#ifndef PSIICONSET_H
#define PSIICONSET_H

#include "xmpp_status.h"

class PsiIcon
{
public:
	PsiIcon() {}

	QIcon icon() const { return QPixmap(":/iconsets/roster/default/online.png"); }
};

class PsiIconset
{
public:
	static PsiIconset* instance() { return 0; }
	PsiIcon status(XMPP::Status::Type) { return PsiIcon(); }
};

#endif
