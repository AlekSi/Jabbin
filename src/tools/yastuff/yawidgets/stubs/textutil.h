#ifndef TEXTUTIL_H
#define TEXTUTIL_H

#include <QString>

namespace TextUtil
{
	// QString plain2rich(const QString &);
	QString rich2plain(const QString &in);
	QString linkifyClever(QString str);
};

#endif
