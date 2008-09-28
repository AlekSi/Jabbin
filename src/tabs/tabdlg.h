/*
 * tabdlg.h - dialog for handling tabbed chats
 * Copyright (C) 2005 Kevin Smith
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

#ifndef TABDLG_H
#define TABDLG_H

#include <QWidget>
#include <QSize>
#include <QMap>
#include <QPointer>

#include "advwidget.h"

#include "tabbablewidget.h"

class PsiCon;
class ChatTabs;
class ChatDlg;
class QPushButton;
class QMenu;
class QString;
class Q3DragObject;
class QContextMenuEvent;
#ifdef YAPSI
class YaTabWidget;
#include "yawindow.h"
typedef YaWindow TabBaseClass;
#else
class PsiTabWidget;
typedef AdvancedWidget<QWidget> TabBaseClass;
#endif
class TabManager;

class TabDlg : public TabBaseClass
{
	Q_OBJECT
public:
	TabDlg(TabManager* tabManager);
	~TabDlg();
	bool managesTab(const TabbableWidget*) const;
	bool tabOnTop(const TabbableWidget*) const;
	TabbableWidget *getTab(int i) const;
	void removeTabWithNoChecks(TabbableWidget *tab);

	TabbableWidget* getTabPointer(PsiAccount* account, QString fullJid);

	uint tabCount() const;
	virtual QString desiredCaption() const;
	QString captionForTab(TabbableWidget* tab) const;

signals:
	void openedChatsChanged();

protected:
	void setShortcuts();
	int unreadMessageCount() const;

	// reimplemented
	void closeEvent(QCloseEvent*);
	void keyPressEvent(QKeyEvent *);
	void windowActivationChange(bool);
	void resizeEvent(QResizeEvent *);
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);

protected slots:
	void detachCurrentTab();
	void mouseDoubleClickTab(QWidget*);
	void detachTab(TabbableWidget*);
	void sendTabTo(TabbableWidget*, TabDlg *);
	void reorderTabs(int oldIndex, int newIndex);

public slots:
	void insertTab(int index, TabbableWidget *tab);
	void addTab(TabbableWidget *tab);
	void setLooks();
	void closeCurrentTab();
	void closeTab(TabbableWidget*, bool doclose = true);
	void selectTab(TabbableWidget*);
	void activated();
	void optionsUpdate();

private slots:
	void updateFlashState();
	void tabSelected(QWidget* selected);
	void checkHasChats();
	void updateTab();
	void updateTab(TabbableWidget*);
	void nextTab();
	void previousTab();
	void tab_aboutToShowMenu(QMenu *menu);
	void menu_sendTabTo(QAction *act);
	void queuedSendTabTo(TabbableWidget* chat, TabDlg *dest);
	void showTabMenu(int tab, QPoint pos, QContextMenuEvent * event);
	void restoringSavedChatsChanged();

private:
	QList<TabbableWidget*> tabs_;
#ifdef YAPSI
	YaTabWidget *tabWidget_;
#else
	PsiTabWidget *tabWidget_;
#endif
	QPushButton *detachButton_;
	QPushButton *closeButton_;
	QPushButton *closeCross_;
	QMenu *tabMenu_;
	QAction *act_close_;
	QAction *act_next_;
	QAction *act_prev_;
	TabManager *tabManager_;
	QPointer<TabbableWidget> selectedTab_;
	bool closing_;

	QSize chatSize_;

	void extinguishFlashingTabs();
	void updateCaption();
	void closeTabs();
	void openedChatsUpdate();
};

#endif
