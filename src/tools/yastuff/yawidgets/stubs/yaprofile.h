#ifndef YAPROFILE_H
#define YAPROFILE_H

#include <QObject>
#include <QPixmap>

#include "yacommon.h"
#include "xmpp_jid.h"

class PsiAccount;

class YaProfile : public QObject
{
	Q_OBJECT
public:
	YaProfile(QObject* parent)
		: QObject(parent)
	{}

	static YaProfile* create(PsiAccount*, QString) { return new YaProfile(0); }

	XMPP::Jid jid() const { return XMPP::Jid(); }
	QString username() const { return QString(); }
	QPixmap avatar() const { return Ya::noAvatarPixmap(); }
	void setAvatar(QPixmap) { qWarning("YaProfile::setAvatar(): not implemented in stub"); }
	QString name() { return "foobar"; }
	QString asYaLink(int len = 0) { 
		QString S_ELLIPSIS = QString::fromUtf8("…");
		QString S_NICK_TEMPLATE = "<a href='%1'><span style='color: red;'>%2</span><span style='color: black;'>%3</span>";
		QString S_URL_TEMPLATE = "http://%1.foo.com/index.html";
		QString username = "nikolay";
		QString name = QString::fromUtf8("Александр Пушкин");
		return S_NICK_TEMPLATE.arg(
		           S_URL_TEMPLATE.arg(username),
		           name.left(1),
		           len > 0 && len < name.length() ?
		           name.mid(1, len - 1 - S_ELLIPSIS.length()) + S_ELLIPSIS :
		           name.right(name.length() - 1));
	}
	void openProfile() {}

signals:
	void nameChanged();
};

#endif
