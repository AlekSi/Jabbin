/*
 * yaloginpage.cpp
 * Copyright (C) 2008  Yandex LLC (Michail Pishchagin)
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

#include "yaloginpage.h"

#include <QPainter>
#include <QMovie>

#include "psicontactlist.h"
#include "psiaccount.h"
#include "psicon.h"
#include "psioptions.h"
#include "psilogger.h"
#include "desktoputil.h"
#include "yacommon.h"

YaLoginPage::YaLoginPage()
	: QWidget(0)
	, contactList_(0)
	, shouldBeVisible_(true)
{
	ui_.setupUi(this);
	connect(ui_.signIn, SIGNAL(clicked()), SLOT(signIn()));
	connect(ui_.signingIn, SIGNAL(clicked()), SLOT(stop()));
	connect(ui_.registerButton, SIGNAL(clicked()), SLOT(registerAccount()));

	connect(ui_.login, SIGNAL(returnPressed()), SLOT(updateFocus()));
	connect(ui_.password, SIGNAL(returnPressed()), SLOT(updateFocus()));

	connect(ui_.login, SIGNAL(textChanged(const QString&)), SLOT(textChanged()));
	connect(ui_.password, SIGNAL(textChanged(const QString&)), SLOT(textChanged()));
	textChanged();

	QMovie * movie = new QMovie(":/images/busy/animation.gif");
	ui_.labelBusy->setMovie(movie);
	movie->start();

	YaPushButton::initAllButtons(this);
}

YaLoginPage::~YaLoginPage()
{
}

void YaLoginPage::setContactList(PsiContactList* contactList)
{
	contactList_ = contactList;
	Q_ASSERT(contactList_);
	connect(contactList_, SIGNAL(accountCountChanged()), SLOT(accountCountChanged()));
	connect(contactList_, SIGNAL(loadedAccounts()), SLOT(updatedActivity()));
	if (contactList_->accountsLoaded())
		accountCountChanged();
}

bool YaLoginPage::shouldBeVisible()
{
#ifdef YAPSI_ACTIVEX_SERVER
	return false;
#endif
	// if (!contactList_ || !getLoginAccount())
	// 	return shouldBeVisible_;
	//if (!ui_.errorLabel->text().isEmpty() ||
        //        !getLoginAccount()->isConnected())
        //        return true;
	return shouldBeVisible_;
}

void YaLoginPage::accountCountChanged()
{
	QList<PsiAccount*> accounts;

	Q_ASSERT(contactList_);
	foreach(PsiAccount* account, knownAccounts_) {
		disconnect(account, SIGNAL(connectionError(QString)), this, SLOT(connectionError(QString)));
		disconnect(account, SIGNAL(updatedActivity()), this, SLOT(updatedActivity()));
	}

	foreach(PsiAccount* account, contactList_->accounts()) {
		connect(account, SIGNAL(connectionError(QString)), this, SLOT(connectionError(QString)));
		connect(account, SIGNAL(updatedActivity()), this, SLOT(updatedActivity()));
		accounts << account;
	}

	knownAccounts_ = accounts;
	updatedActivity();
}

void YaLoginPage::connectionError(const QString &error)
{
	ui_.errorLabel->setText(error);
}

void YaLoginPage::updatedActivity()
{
	Q_ASSERT(contactList_);
	if (!contactList_->accountsLoaded())
		return;

	bool shouldBeVisible = true;

	if (contactList_->enabledAccounts().count() > 1) {
		shouldBeVisible = false;
	}
	else {
		foreach(PsiAccount* account, contactList_->enabledAccounts()) {
			if (account->isAvailable())
				shouldBeVisible = false;
		}
	}

	setShouldBeVisible(shouldBeVisible);
}

void YaLoginPage::setShouldBeVisible(bool shouldBeVisible)
{
	if (shouldBeVisible != shouldBeVisible_) {
		shouldBeVisible_ = shouldBeVisible;
		emit updateVisibility();
	}

	if (shouldBeVisible_) {
		bool enableControls = true;
		PsiAccount* account = 0;
		if (login().isEmpty())
			account = getLoginAccount();
		else
			account = getAccount();
		if (account) {
			UserAccount acc = account->userAccount();
			QString jid = Ya::stripYaRuHostname(acc.jid);

			ui_.login->setText(jid);
			ui_.password->setText(acc.pass);

			if (account->isActive())
				enableControls = false;
		}

		if (enableControls) {
			ui_.buttonStack->setCurrentWidget(ui_.signInPage);
			ui_.statusStack->setCurrentWidget(ui_.signInPage2);
                } else {
			ui_.buttonStack->setCurrentWidget(ui_.signingInPage);
			ui_.statusStack->setCurrentWidget(ui_.signingInPage2);
                }

		ui_.login->setEnabled(enableControls);
		ui_.password->setEnabled(enableControls);
		ui_.signIn->setEnabled(signInEnabled());
		// ui_.busy->setActive(!enableControls);
		ui_.labelBusy->movie()->setPaused(enableControls);

		if (login().isEmpty()) {
			ui_.login->setFocus();
		}
		else {
			ui_.password->setFocus();
			ui_.password->selectAll();
		}
	}
}

void YaLoginPage::signIn()
{
	ui_.errorLabel->setText(QString());

	// on new account creation accountCountChanged() is triggered, so we back up our fields
	QString jid  = login();
	QString pass = password();

	Q_ASSERT(contactList_);
	PsiAccount* account = getAccount();
	if (!account) {
		account = contactList_->psi()->createAccount();
	}
	Q_ASSERT(account);

	UserAccount acc = account->userAccount();
	acc.jid  = jid;
	acc.pass = pass;
	account->setUserAccount(acc);
	account->setEnabled(true);
	account->autoLogin();
}

void YaLoginPage::stop()
{
	PsiLogger::instance()->log(QString("%1 YaLoginPage()::stop()").arg(LOG_THIS));

	Q_ASSERT(contactList_);
	PsiAccount* account = getAccount();
	if (account)
		account->setStatus(XMPP::Status::Offline);
}

bool YaLoginPage::signInEnabled() const
{
	return ui_.login->isEnabled() &&
	       !ui_.login->text().isEmpty() &&
	       !ui_.password->text().isEmpty();
}

void YaLoginPage::textChanged()
{
	ui_.signIn->setEnabled(signInEnabled());
}

void YaLoginPage::updateFocus()
{
	if (ui_.login->hasFocus()) {
		ui_.password->setFocus();
		ui_.password->selectAll();
	}
	else if (ui_.password->hasFocus()) {
		ui_.signIn->setFocus();
		ui_.signIn->click();
	}
	else {
		ui_.login->setFocus();
		ui_.login->selectAll();
	}
}

QString YaLoginPage::login() const
{
	QString result = ui_.login->text().trimmed();
	if (!result.isEmpty() && !result.contains("@"))
		result += "@jabbin.com";
	return result;
}

QString YaLoginPage::password() const
{
	return ui_.password->text();
}

PsiAccount* YaLoginPage::getLoginAccount() const
{
	Q_ASSERT(login().isEmpty());

	if (!contactList_->enabledAccounts().isEmpty())
		return contactList_->enabledAccounts().first();

	if (!contactList_->accounts().isEmpty())
		return contactList_->accounts().first();

	return 0;
}

PsiAccount* YaLoginPage::getAccount() const
{
	Q_ASSERT(contactList_);

	foreach(PsiAccount* account, contactList_->accounts()) {
		if (account->jid().bare() == login())
			return account;
	}

	if ((contactList_->accounts().count() == 1) ||
	    (PsiOptions::instance()->getOption("options.ui.account.single").toBool() && !contactList_->accounts().isEmpty()))
		return contactList_->accounts().first();

	return 0;
}

void YaLoginPage::registerAccount()
{
	DesktopUtil::openYaUrl("http://www.jabbin.com/register");
}

void YaLoginPage::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	p.fillRect(rect(), Qt::white);
}
