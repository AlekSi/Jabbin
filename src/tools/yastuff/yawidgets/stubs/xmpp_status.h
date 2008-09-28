#ifndef STATUS_H
#define STATUS_H

#include <QString>

namespace XMPP {

class Status
{
public:
	enum Type { Online, Away, FFC, XA, DND, Offline, Invisible };

	Status(Type type, const QString& message) : type_(type), message_(message) {};

	Type type() const { return type_; }
	const QString& status() const { return message_; }

private:
	Type type_;
	QString message_;
};

} // namespace XMPP

#endif
