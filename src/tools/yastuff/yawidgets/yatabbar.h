/*
 * yatabbar.h
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

#ifndef YATABBAR_H
#define YATABBAR_H

#include <QColor>
#include <QTabBar>
#include <QTimeLine>

#include "overlaywidget.h"
#include "yachevronbutton.h"

class QStyleOptionTabV2;
class QSignalMapper;
class YaCloseButton;
class TabbableWidget;

class YaTabBar : public OverlayWidget<QTabBar, YaChevronButton>
{
	Q_OBJECT
public:
	YaTabBar(QWidget* parent);
	~YaTabBar();

	TabbableWidget* currentTab() const;
	QString tabBackgroundName() const;
	QColor tabBackgroundColor() const;

	// reimplemented
	QSize minimumSizeHint() const;

	void updateHiddenTabs();

signals:
	void closeButtonClicked();
	void reorderTabs(int oldIndex, int newIndex);

public slots:
	void updateLayout();

	virtual void startFading();
	virtual void stopFading();
	virtual void updateFading();

protected:
	virtual QColor getCurrentColorGrade(const QColor&, const QColor&, const int, const int) const;
	void updateSendButton();

	// reimplemented
	void paintEvent(QPaintEvent*);
	QSize tabSizeHint(int index) const;
	void tabLayoutChange();
	bool event(QEvent* event);
	void setCurrentIndex(int index);
	void mousePressEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);

signals:
	void aboutToShow(int index);

private:
	YaCloseButton* closeButton_;
	QList<QAction*> hiddenTabsActions_;

	QVector<int> hiddenTabs_;
	QVector<QSize> tabSizeHint_;
	QStringList previousTabNames_;
	int previousCurrentIndex_;
	QSize previousSize_;
	QString previousFont_;

	QList<int> tabDrawOrder_;
	QPoint pressedPosition_;
	QPoint dragPosition_;
	QPoint dragOffset_;
	int draggedTabIndex_;

	QColor highlightColor() const;
	void drawTab(QPainter* painter, int index, const QRect& tabRect);
	void updateTabDrawOrder();
	QRect draggedTabRect() const;
	void startDrag();

	QSize preferredTabSize(const QString& text, bool current) const;
	QSize preferredTabSize(int index) const;
	int minimumTabWidth() const;
	int maximumTabWidth() const;
	int maximumWidth() const;

	bool needToRelayoutTabs();
	void relayoutTabs();
	void relayoutTabsHelper();
	QSize relayoutTab(int index);
	QList<int> visibleTabs();
	void updateHiddenTabActions();
	bool drawHighlightChevronBackground() const;

	bool showTabs() const;
	QStyleOptionTabV2 getStyleOption(int tab) const;
	QRect closeButtonRect(const QRect& currentTabRect) const;
	QRect closeButtonRect() const;
	void updateCloseButtonPosition();
	void updateCloseButtonPosition(const QRect& closeButtonRect);
	QRect chevronButtonRect() const;

	// reimplemented
	QRect extraGeometry() const;
	bool extraShouldBeVisible() const;

	QTimeLine* timeLine_;
	QSignalMapper* activateTabMapper_;
};

#endif
