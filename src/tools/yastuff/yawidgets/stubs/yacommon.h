#ifndef YACOMMON_H
#define YACOMMON_H

#include <QFontMetrics>
#include <QPixmap>

class QColor;

namespace XMPP {
	class Jid;
}

#include "xmpp_status.h"
#include "visibletext.h"
#include "psievent.h"

class UserListItem;
class QChar;

class Ya {
public:
	enum DecorationState {
		Normal = 0,
		Hover,
		Pressed
	};

	static QPixmap groupPixmap(QSize size, bool open, DecorationState state)
	{
		return QPixmap();
	}

	static QString ellipsis()
	{
		return "...";
	}

	static bool isYaInformer(PsiEvent* e)
	{
		return e != 0;
	}

	static bool isSubscriptionRequest(PsiEvent* e)
	{
		return e != 0;
	}

	static QString visibleText(const QString& fullText, const QFontMetrics& fontMetrics, int width, int height, int numLines)
	{
		return ::visibleText(fullText, fontMetrics, width, height, numLines);
	}

	static QPixmap noAvatarPixmap()
	{
		return QPixmap(":/images/avatars/it.png");
	}

	static bool isYaJid(const XMPP::Jid&)
	{
		return true;
	}

	static const QChar& friendsChar()
	{
		static QChar c;
		return c;
	}

	static QString defaultFriendsGroup()
	{
		return "Friends";
	}

	static bool isFriendsGroup(const QString&)
	{
		return false;
	}

	static bool isInFriends(const UserListItem*)
	{
		return false;
	}

	static QString statusName(XMPP::Status::Type)
	{
		return "Online";
	}

	static QString statusFullName(XMPP::Status::Type)
	{
		return "Online";
	}
};

#endif
