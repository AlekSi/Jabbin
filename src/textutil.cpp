#include <QTextDocument> // for escape()
#include <Q3PtrList>
#include <QUrl>

#include "textutil.h"
#include "psiiconset.h"
#include "rtparse.h"
#include "common.h"
#ifdef YAPSI
#include "iconset.h"
#endif

// Qt::escape() doesn't escape " to &quot; -- it sucks
QString TextUtil::escape(const QString& plain)
{
	QString rich;
	rich.reserve(int(plain.length() * 1.1));
	for (int i = 0; i < plain.length(); ++i) {
		if (plain.at(i) == QLatin1Char('<'))
			rich += QLatin1String("&lt;");
		else if (plain.at(i) == QLatin1Char('>'))
			rich += QLatin1String("&gt;");
		else if (plain.at(i) == QLatin1Char('"'))
			rich += QLatin1String("&quot;");
		else if (plain.at(i) == QLatin1Char('&'))
			rich += QLatin1String("&amp;");
		else
			rich += plain.at(i);
	}
	return rich;
}

QString TextUtil::unescape(const QString& escaped)
{
	QString plain = escaped;
	plain.replace("&lt;", "<");
	plain.replace("&gt;", ">");
	plain.replace("&quot;", "\"");
	plain.replace("&amp;", "&");
	return plain;
}

QString TextUtil::quote(const QString &toquote, int width, bool quoteEmpty)
{
	int ql = 0, col = 0, atstart = 1, ls=0;

	QString quoted = "> "+toquote;  // quote first line
	QString rxs  = quoteEmpty ? "\n" : "\n(?!\\s*\n)";
	QRegExp rx(rxs);                // quote following lines
	quoted.replace(rx, "\n> ");
	rx.setPattern("> +>");          // compress > > > > quotes to >>>>
	quoted.replace(rx, ">>");
	quoted.replace(rx, ">>");
	quoted.replace(QRegExp(" +\n"), "\n");  // remove trailing spaces

	if (!quoteEmpty)
	{
		quoted.replace(QRegExp("^>+\n"), "\n\n");  // unquote empty lines
		quoted.replace(QRegExp("\n>+\n"), "\n\n");
	}


	for (int i=0;i<(int) quoted.length();i++)
	{
		col++;
		if (atstart && quoted[i] == '>') ql++; else atstart=0;

		switch(quoted[i].latin1())
		{
			case '\n': ql = col = 0; atstart = 1; break;
			case ' ':
			case '\t': ls = i; break;

		}
		if (quoted[i]=='\n') { ql=0; atstart = 1; }

		if (col > width)
		{
			if ((ls+width) < i)
			{
				ls = i; i = quoted.length();
				while ((ls<i) && !quoted[ls].isSpace()) ls++;
				i = ls;
			}
			if ((i<(int)quoted.length()) && (quoted[ls] != '\n'))
			{
				quoted.insert(ls, '\n');
				++ls;
				quoted.insert(ls, QString().fill('>', ql));
				i += ql+1;
				col = 0;
			}
		}
	}
	quoted += "\n\n";// add two empty lines to quoted text - the cursor
			// will be positioned at the end of those.
	return quoted;
}

QString TextUtil::plain2richSimple(const QString& plain)
{
	QString rich;
	int col = 0;

	for(int i = 0; i < (int)plain.length(); ++i) {
#ifdef Q_OS_WIN
		if(plain[i] == '\r' && i+1 < (int)plain.length() && plain[i+1] == '\n')
			++i;	// Qt/Win sees \r\n as two new line chars
#endif
		if(plain[i] == '\n') {
			rich += "<br>";
			col = 0;
		}
		else if(plain[i] == '<')
			rich += "&lt;";
		else if(plain[i] == '>')
			rich += "&gt;";
		else if(plain[i] == '\"')
			rich += "&quot;";
		else if(plain[i] == '\'')
			rich += "&apos;";
		else if(plain[i] == '&')
			rich += "&amp;";
		else
			rich += plain[i];
		++col;
	}

	return rich;
}

QString TextUtil::plain2rich(const QString &plain)
{
	return "<span style='white-space: pre-wrap'>" + plain2richSimple(plain) + "</span>";
}

QString TextUtil::rich2plain(const QString &in)
{
	QString out;

	for(int i = 0; i < (int)in.length(); ++i) {
		// tag?
		if(in[i] == '<') {
			// find end of tag
			++i;
			int n = in.find('>', i);
			if(n == -1)
				break;
			QString str = in.mid(i, (n-i));
			i = n;

			QString tagName;
			n = str.find(' ');
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
			int n = in.find(';', i);
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
			if(in[i] == QChar::nbsp)
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

QString TextUtil::resolveEntities(const QString &in)
{
	QString out;

	for(int i = 0; i < (int)in.length(); ++i) {
		if(in[i] == '&') {
			// find a semicolon
			++i;
			int n = in.find(';', i);
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
		else {
			out += in[i];
		}
	}

	return out;
}


static bool linkify_pmatch(const QString &str1, int at, const QString &str2)
{
	if(str2.length() > (str1.length()-at))
		return FALSE;

	for(int n = 0; n < (int)str2.length(); ++n) {
		if(str1.at(n+at).lower() != str2.at(n).lower())
			return FALSE;
	}

	return TRUE;
}

static bool linkify_isOneOf(const QChar &c, const QString &charlist)
{
	for(int i = 0; i < (int)charlist.length(); ++i) {
		if(c == charlist.at(i))
			return TRUE;
	}

	return FALSE;
}

// encodes a few dangerous html characters
static QString linkify_htmlsafe(const QString &in)
{
	QString out;

	for(int n = 0; n < in.length(); ++n) {
		if(linkify_isOneOf(in.at(n), "\"\'`<>")) {
			// hex encode
			QString hex;
			hex.sprintf("%%%02X", in.at(n).latin1());
			out.append(hex);
		}
		else {
			out.append(in.at(n));
		}
	}

	return out;
}

static bool linkify_okUrl(const QString &url)
{
	if(url.at(url.length()-1) == '.')
		return FALSE;

	return TRUE;
}

static bool linkify_okEmail(const QString &addy)
{
	// this makes sure that there is an '@' and a '.' after it, and that there is
	// at least one char for each of the three sections
	int n = addy.find('@');
	if(n == -1 || n == 0)
		return FALSE;
	int d = addy.find('.', n+1);
	if(d == -1 || d == 0)
		return FALSE;
	if((addy.length()-1) - d <= 0)
		return FALSE;
	if(addy.find("..") != -1)
		return false;

	return TRUE;
}

QString TextUtil::linkify(const QString &in)
{
	QString out = in;
	int x1, x2;
	bool isUrl, isEmail;
	QString linked, link, href;

	for(int n = 0; n < (int)out.length(); ++n) {
		isUrl = FALSE;
		isEmail = FALSE;
		x1 = n;

		if(linkify_pmatch(out, n, "http://")) {
			n += 7;
			isUrl = TRUE;
			href = "";
		}
		else if(linkify_pmatch(out, n, "https://")) {
			n += 8;
			isUrl = TRUE;
			href = "";
		}
		else if(linkify_pmatch(out, n, "ftp://")) {
			n += 6;
			isUrl = TRUE;
			href = "";
		}
		else if(linkify_pmatch(out, n, "news://")) {
			n += 7;
			isUrl = TRUE;
			href = "";
		}
		else if (linkify_pmatch(out, n, "ed2k://")) {
			n += 7;
			isUrl = TRUE;
			href = "";
		}
		else if(linkify_pmatch(out, n, "www.")) {
			isUrl = TRUE;
			href = "http://";
		}
		else if(linkify_pmatch(out, n, "ftp.")) {
			isUrl = TRUE;
			href = "ftp://";
		}
		else if(linkify_pmatch(out, n, "@")) {
			isEmail = TRUE;
			href = "mailto:";
		}

		if(isUrl) {
			// make sure the previous char is not alphanumeric
			if(x1 > 0 && out.at(x1-1).isLetterOrNumber())
				continue;

			// find whitespace (or end)
			for(x2 = n; x2 < (int)out.length(); ++x2) {
				if(out.at(x2).isSpace() || out.at(x2) == '<')
					break;
			}
			int len = x2-x1;
			QString pre = resolveEntities(out.mid(x1, x2-x1));

			// go backward hacking off unwanted punctuation
			int cutoff;
			for(cutoff = pre.length()-1; cutoff >= 0; --cutoff) {
				if(!linkify_isOneOf(pre.at(cutoff), "!?,.()[]{}<>\""))
					break;
			}
			++cutoff;
			//++x2;

			link = pre.mid(0, cutoff);
			if(!linkify_okUrl(link)) {
				n = x1 + link.length();
				continue;
			}
			href += link;
			href = linkify_htmlsafe(href);
			//printf("link: [%s], href=[%s]\n", link.latin1(), href.latin1());
			linked = QString("<a href=\"%1\">").arg(href) + Qt::escape(link) + "</a>" + Qt::escape(pre.mid(cutoff));
			out.replace(x1, len, linked);
			n = x1 + linked.length() - 1;
		}
		else if(isEmail) {
			// go backward till we find the beginning
			if(x1 == 0)
				continue;
			--x1;
			for(; x1 >= 0; --x1) {
				if(!linkify_isOneOf(out.at(x1), "_.-") && !out.at(x1).isLetterOrNumber())
					break;
			}
			++x1;

			// go forward till we find the end
			x2 = n + 1;
			for(; x2 < (int)out.length(); ++x2) {
				if(!linkify_isOneOf(out.at(x2), "_.-") && !out.at(x2).isLetterOrNumber())
					break;
			}

			int len = x2-x1;
			link = out.mid(x1, len);
			//link = resolveEntities(link);

			if(!linkify_okEmail(link)) {
				n = x1 + link.length();
				continue;
			}

			href += link;
			//printf("link: [%s], href=[%s]\n", link.latin1(), href.latin1());
			linked = QString("<a href=\"%1\">").arg(href) + link + "</a>";
			out.replace(x1, len, linked);
			n = x1 + linked.length() - 1;
		}
	}

	return out;
}


QString TextUtil::linkifyClever(const QString & in)
{
	QString txt = in;
	//                   1        23                             4
	QRegExp r = QRegExp("(^|\\s|>)((http|https|ftp)://[^\\s!<>]*)(<|>|\\s|$|!)");
	txt.replace(r, "\\1<a href=\"\\2\">\\2</a>\\4");
	return txt;
}

// sickening
QString TextUtil::emoticonify(const QString &in)
{
	if (!option.useEmoticons)
		return in;

	RTParse p(in);
	while ( !p.atEnd() ) {
		// returns us the first chunk as a plaintext string
		QString str = p.next();

		int i = 0;
		while ( i >= 0 ) {
			// find closest emoticon
			int ePos = -1;
			PsiIcon *closest = 0;

			int foundPos = -1, foundLen = -1;

			Q3PtrListIterator<Iconset> iconsets(PsiIconset::instance()->emoticons);
			Iconset *iconset;
    			while ( (iconset = iconsets.current()) != 0 ) {
				QListIterator<PsiIcon*> it = iconset->iterator();
				while ( it.hasNext()) {
					PsiIcon *icon = it.next();
					if ( icon->regExp().isEmpty() )
						continue;

					// some hackery
					int iii = i;
					bool searchAgain;

					do {
						searchAgain = false;

						// find the closest match
						const QRegExp &rx = icon->regExp();
						int n = rx.search(str, iii);
						if ( n == -1 )
							continue;

						if(ePos == -1 || n < ePos || (rx.matchedLength() > foundLen && n < ePos + foundLen)) {
							bool leftSpace  = n == 0 || (n > 0 && str[n-1].isSpace());
							bool rightSpace = (n+rx.matchedLength() == (int)str.length()) || (n+rx.matchedLength() < (int)str.length() && str[n+rx.matchedLength()].isSpace());
#ifdef YAPSI
							if (leftSpace && rightSpace) {
#else
							// there must be whitespace at least on one side of the emoticon
							if (leftSpace || rightSpace) {
#endif
								ePos = n;
								closest = icon;

								foundPos = n;
								foundLen = rx.matchedLength();
								break;
							}

							searchAgain = true;
						}

						iii = n + rx.matchedLength();
					} while ( searchAgain );
				}

				++iconsets;
			}

			QString s;
			if(ePos == -1)
				s = str.mid(i);
			else
				s = str.mid(i, ePos-i);
			p.putPlain(s);

			if ( !closest )
				break;

			p.putRich( QString("<icon name=\"%1\" text=\"%2\">").arg(TextUtil::escape(closest->name())).arg(TextUtil::escape(str.mid(foundPos, foundLen))) );
			i = foundPos + foundLen;
		}
	}

	QString out = p.output();
	return out;
}

QString TextUtil::legacyFormat(const QString& in)
{
	//enable *bold* stuff
	// //old code
	//out=out.replace(QRegExp("(^[^<>\\s]*|\\s[^<>\\s]*)\\*(\\S+)\\*([^<>\\s]*\\s|[^<>\\s]*$)"),"\\1<b>*\\2*</b>\\3");
	//out=out.replace(QRegExp("(^[^<>\\s\\/]*|\\s[^<>\\s\\/]*)\\/([^\\/\\s]+)\\/([^<>\\s\\/]*\\s|[^<>\\s\\/]*$)"),"\\1<i>/\\2/</i>\\3");
	//out=out.replace(QRegExp("(^[^<>\\s]*|\\s[^<>\\s]*)_(\\S+)_([^<>\\s]*\\s|[^<>\\s]*$)"),"\\1<u>_\\2_</u>\\3");

	QString out=in;
	out=out.replace(QRegExp("(^|\\s|>)_(\\S+)_(<|\\s|$)"),"\\1<u>_\\2_</u>\\3");
	out=out.replace(QRegExp("(^|\\s|>)\\*(\\S+)\\*(<|\\s|$)"),"\\1<b>*\\2*</b>\\3");
	out=out.replace(QRegExp("(^|\\s|>)\\/(\\S+)\\/(<|\\s|$)"),"\\1<i>/\\2/</i>\\3");

	return out;
}

#ifdef YAPSI
// based on psirichtext.cpp: convertIconsToObjectReplacementCharacters()
template<typename EscapeFunc, typename IconFunc>
QString customEmoticonify(const QString &in, EscapeFunc escapeFunc, IconFunc iconFunc)
{
	static QRegExp rxName("name=\"([^\"]+)\"");
	static QRegExp rxText("text=\"([^\"]+)\"");

	QString result;
	QString work = TextUtil::emoticonify(in);

	forever {
		int start = work.indexOf("<icon");
		if (start == -1)
			break;

		result += escapeFunc(work.left(start));

		int end = work.indexOf(">", start);
		Q_ASSERT(end != -1);

		QString fragment = work.mid(start, end - start);
		if (rxName.indexIn(fragment) != -1) {
			QString iconName = TextUtil::unescape(rxName.capturedTexts()[1]);
			QString iconText;
			if (rxText.indexIn(fragment) != -1)
				iconText = TextUtil::unescape(rxText.capturedTexts()[1]);

			const PsiIcon* icon = IconsetFactory::iconPtr(iconName);
			if (icon)
				result += iconFunc(icon, iconText);
			else
				result += escapeFunc(iconText);
		}

		work = work.mid(end + 1);
	}

	return result + escapeFunc(work);
}

static QString escapeHtmlHelper(const QString& in)
{
	return TextUtil::escape(in);
}

static QString escapeNothingHelper(const QString& in)
{
	return in;
}

static QString emoticonifyForYaOnlineHelper(const PsiIcon* icon, const QString& iconText)
{
	Q_UNUSED(iconText);
	return QString("<smile href=\"file://%1\"/>")
	       .arg(QUrl::toPercentEncoding(icon->fileName(), "/:", "+\"").constData());
}

QString TextUtil::emoticonifyForYaOnline(const QString &in)
{
	return customEmoticonify(in, escapeNothingHelper, emoticonifyForYaOnlineHelper);
}

static QString emoticonifyForHTMLHelper(const PsiIcon* icon, const QString& iconText)
{
	return QString("<img src=\"file://%1\" alt=\"%2\"/>")
	       .arg(TextUtil::escape(icon->fileName()))
	       .arg(TextUtil::escape(iconText));
}

QString TextUtil::emoticonifyForHtml(const QString &in)
{
	QString result = customEmoticonify(in, escapeNothingHelper, emoticonifyForHTMLHelper);
	result.replace("\n", "<br/>");
	return result;
}
#endif
