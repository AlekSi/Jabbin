#ifndef PSICONTACTLIST_H
#define PSICONTACTLIST_H

class PsiAccount;

class PsiContactList : public QObject
{
	Q_OBJECT
public:
	PsiContactList() {}

	static PsiAccount* defaultAccount() { return 0; }
	QList<PsiAccount*> enabledAccounts() { return QList<PsiAccount*>(); }
};

#endif