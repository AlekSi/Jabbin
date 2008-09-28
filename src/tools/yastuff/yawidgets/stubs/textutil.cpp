#include "textutil.h"

#include <QRegExp>

QString TextUtil::rich2plain(const QString &in)
{
	QString out;

	for(int i = 0; i < (int)in.length(); ++i) {
		// tag?
		if(in[i] == '<') {
			// find end of tag
			++i;
			int n = in.indexOf('>', i);
			if(n == -1)
				break;
			QString str = in.mid(i, (n-i));
			i = n;

			QString tagName;
			n = str.indexOf(' ');
			if(n != -1)
				tagName = str.mid(0, n);
			else
				tagName = str;

			if(tagName == "br")
				out += '\n';

			// handle output of Qt::convertFromPlainText() correctly
			if((tagName == "p" || tagName == "/p") && out.length() > 0)
				out += '\n';
		}
		// entity?
		else if(in[i] == '&') {
			// find a semicolon
			++i;
			int n = in.indexOf(';', i);
			if(n == -1)
				break;
			QString type = in.mid(i, (n-i));
			i = n; // should be n+1, but we'll let the loop increment do it

			if(type == "amp")
				out += '&';
			else if(type == "lt")
				out += '<';
			else if(type == "gt")
				out += '>';
			else if(type == "quot")
				out += '\"';
			else if(type == "apos")
				out += '\'';
		}
		else if(in[i].isSpace()) {
			if(in[i] == QChar::Nbsp)
				out += ' ';
			else if(in[i] != '\n') {
				if(i == 0)
					out += ' ';
				else {
					QChar last = out.at(out.length()-1);
					bool ok = TRUE;
					if(last.isSpace() && last != '\n')
						ok = FALSE;
					if(ok)
						out += ' ';
				}
			}
		}
		else {
			out += in[i];
		}
	}

	return out;
}

QString TextUtil::linkifyClever(QString in)
{
	QString txt = in;
	//                   1        23                             4
	QRegExp r = QRegExp("(^|\\s|>)((http|https|ftp)://[^\\s!<>]*)(<|>|\\s|$|!)");
	txt.replace(r, "\\1<a href=\"\\2\">\\2</a>\\4");
	return txt;
}
