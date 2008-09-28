/*
 * tabbable.h
 * Copyright (C) 2007 Kevin Smith
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

#ifndef TABBABLE_H
#define TABBABLE_H

#include "advwidget.h"
#include "im.h" // ChatState

namespace XMPP {
	class Jid;
	class Message;
}
using namespace XMPP;

class PsiAccount;
class TabManager;
class TabDlg;

#ifdef YAPSI
#include "yawindowbackground.h"
#endif

class TabbableWidget : public AdvancedWidget<QWidget>
{
	Q_OBJECT
public:
	TabbableWidget(const Jid &, PsiAccount *, TabManager *tabManager);
	~TabbableWidget();

	PsiAccount* account() const;

	virtual Jid jid() const; 
	virtual const QString & getDisplayName();

	virtual bool readyToHide();
	TabDlg* getManagingTabDlg();

	bool isTabbed(); 
	bool isActiveTab();

	// reimplemented
	virtual void doFlash(bool on);

	virtual void invalidateTab();

	enum State {
		StateNone = 0,
		StateComposing
	};
	virtual State state() const = 0;
	virtual int unreadMessageCount() const = 0;
	virtual QString desiredCaption() const = 0;

	virtual void aboutToShow();

#ifdef YAPSI
	const YaWindowBackground& background() const;
#endif

signals:
	void invalidateTabInfo();
	void updateFlashState();
	void eventsRead(const Jid &);

public slots:
	virtual void closed();
	virtual void deactivated();
	virtual void activated();
	void bringToFront(bool raiseWindow = true);
	void ensureTabbedCorrectly();

protected:
	virtual void setJid(const Jid&);

	// reimplemented
	void paintEvent(QPaintEvent*);

private:
	Jid jid_;
	PsiAccount *pa_;
	TabManager *tabManager_;
#ifdef YAPSI
	YaWindowBackground background_;
#endif
};

#endif
