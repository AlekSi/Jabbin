/*
 * xmlconsole.cpp - dialog for interacting manually with Jabber
 * Copyright (C) 2001, 2002  Justin Karneges, Remko Troncon
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <QLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QHBoxLayout>
#include <QMessageBox>

#include "xmpp_client.h"
#include "xmlconsole.h"
#include "psiaccount.h"
#include "psicon.h"
#include "psicontactlist.h"
#ifdef YAPSI
#include "yastyle.h"
#include "yaremoveconfirmationmessagebox.h"
#endif

//----------------------------------------------------------------------------
// XmlConsole
//----------------------------------------------------------------------------
XmlConsole::XmlConsole(PsiAccount *_pa)
:QWidget()
{
#ifdef YAPSI
	setStyle(YaStyle::defaultStyle());
#endif
	ui_.setupUi(this);

	pa = _pa;
	pa->dialogRegister(this);
	connect(pa, SIGNAL(updatedAccount()), SLOT(updateCaption()));
	connect(pa->client(), SIGNAL(xmlIncoming(const QString &)), SLOT(client_xmlIncoming(const QString &)));
	connect(pa->client(), SIGNAL(xmlOutgoing(const QString &)), SLOT(client_xmlOutgoing(const QString &)));
	connect(pa->psi(), SIGNAL(accountCountChanged()), this, SLOT(updateCaption()));
	updateCaption();

	prompt = 0;

	ui_.te->setUndoRedoEnabled(false);
	ui_.te->setReadOnly(true);
	ui_.te->setTextFormat(Qt::PlainText);

	QTextFrameFormat f = ui_.te->document()->rootFrame()->frameFormat();
	f.setBackground(QBrush(Qt::black));
	ui_.te->document()->rootFrame()->setFrameFormat(f);

	connect(ui_.pb_clear, SIGNAL(clicked()), SLOT(clear()));
	connect(ui_.pb_input, SIGNAL(clicked()), SLOT(insertXml()));
	connect(ui_.pb_close, SIGNAL(clicked()), SLOT(close()));
	connect(ui_.pb_dumpRingbuf, SIGNAL(clicked()), SLOT(dumpRingbuf()));
	connect(ui_.ck_enable, SIGNAL(clicked(bool)), ui_.gb_filter, SLOT(setEnabled(bool)));

#ifdef DEFAULT_XMLCONSOLE
	enable();
#endif

#ifdef YAPSI
	YaStyle::makeMeNativeLooking(this);
#endif

	resize(560,400);
}

XmlConsole::~XmlConsole()
{
	pa->dialogUnregister(this);
}

void XmlConsole::clear()
{
	ui_.te->clear();
	QTextFrameFormat f = ui_.te->document()->rootFrame()->frameFormat();
	f.setBackground(QBrush(Qt::black));
	ui_.te->document()->rootFrame()->setFrameFormat(f);
}

void XmlConsole::updateCaption()
{
	if (pa->psi()->contactList()->enabledAccounts().count() > 1)
		setWindowTitle(pa->name() + ": " + tr("XML Console"));
	else
		setWindowTitle(tr("XML Console"));
}

void XmlConsole::enable()
{
	ui_.ck_enable->setChecked(true);
}

bool XmlConsole::filtered(const QString& str) const
{
	if(ui_.ck_enable->isChecked()) {
		// Only do parsing if needed
		if (!ui_.le_jid->text().isEmpty() || !ui_.ck_iq->isChecked() || !ui_.ck_message->isChecked() || !ui_.ck_presence->isChecked()) {
			QDomDocument doc;
			if (!doc.setContent(str))
				return true;
			
			QDomElement e = doc.documentElement();
			if ((e.tagName() == "iq" && !ui_.ck_iq->isChecked()) || (e.tagName() == "message" && !ui_.ck_message->isChecked()) || ((e.tagName() == "presence" && !ui_.ck_presence->isChecked())))
				return true;

			if (!ui_.le_jid->text().isEmpty()) {
				Jid jid(ui_.le_jid->text());
				bool hasResource = !jid.resource().isEmpty();
				if (!jid.compare(e.attribute("to"),hasResource) && !jid.compare(e.attribute("from"),hasResource))
					return true;
			}
		}
		return false;
	}
	return true;
}

void XmlConsole::dumpRingbuf()
{
	QList<PsiAccount::xmlRingElem> buf = pa->dumpRingbuf();
	bool enablesave = ui_.ck_enable->isChecked();
	ui_.ck_enable->setChecked(true);
	QString stamp;
	foreach (PsiAccount::xmlRingElem el, buf) {
		stamp = "<!-- TS:" + el.time.toString(Qt::ISODate) + "-->";
		if (el.type == PsiAccount::RingXmlOut) {
			client_xmlOutgoing(stamp + el.xml);
		} else {
			client_xmlIncoming(stamp + el.xml);
		}
	}
	ui_.ck_enable->setChecked(enablesave);
}

void XmlConsole::client_xmlIncoming(const QString &str)
{
	if (!filtered(str)) {
		ui_.te->setColor(Qt::yellow);
		ui_.te->append(str + '\n');
	}
}

void XmlConsole::client_xmlOutgoing(const QString &str)
{
	if(!filtered(str)) {
		ui_.te->setColor(Qt::red);
		ui_.te->append(str + '\n');
	}
}

void XmlConsole::insertXml()
{
	if(prompt)
		bringToFront(prompt);
	else {
		prompt = new XmlPrompt(this);
		connect(prompt, SIGNAL(textReady(const QString &)), SLOT(xml_textReady(const QString &)));
		prompt->show();
	}
}

void XmlConsole::xml_textReady(const QString &str)
{
	pa->client()->send(str);
}


//----------------------------------------------------------------------------
// XmlPrompt
//----------------------------------------------------------------------------
XmlPrompt::XmlPrompt(QWidget *parent, const char *name)
	: QDialog(parent, name, false)
{
#ifdef YAPSI
	setStyle(YaStyle::defaultStyle());
#endif
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle(tr("XML Input"));

	QVBoxLayout *vb1 = new QVBoxLayout(this, 8);

	te = new QTextEdit(this);
	te->setAcceptRichText(false);
	vb1->addWidget(te);

	QHBoxLayout *hb1 = new QHBoxLayout(vb1);
	QPushButton *pb;

	pb = new QPushButton(tr("&Transmit"), this);
	pb->setDefault(TRUE);
	connect(pb, SIGNAL(clicked()), SLOT(doTransmit()));
	hb1->addWidget(pb);
	hb1->addStretch(1);

	pb = new QPushButton(tr("&Close"), this);
	connect(pb, SIGNAL(clicked()), SLOT(close()));
	hb1->addWidget(pb);

#ifdef YAPSI
	YaStyle::makeMeNativeLooking(this);
#endif

	resize(320,240);
}

XmlPrompt::~XmlPrompt()
{
}

void XmlPrompt::doTransmit()
{
	QString str = te->text();

	// Validate input
	QDomDocument doc;
	if (!doc.setContent(str)) {
		QString title = tr("Malformed XML");
		QString msg = tr("You have entered malformed XML input. Are you sure you want to send this ?");
		QString yes = tr("Yes");
		QString no = tr("No");
#ifndef YAPSI
		int i = QMessageBox::warning(this, title, msg, yes, no);
		if (i != 0)
			return;
#else
		YaRemoveConfirmationMessageBox msgBox(tr("Malformed XML"), msg, this);
		msgBox.setDestructiveActionName(yes);
		if (!msgBox.confirmRemoval())
			return;
#endif
	}

	textReady(str);
	close();
}

