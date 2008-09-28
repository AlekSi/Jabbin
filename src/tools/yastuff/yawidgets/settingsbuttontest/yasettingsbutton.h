/*
 * yasettingsbutton.h - settings gear button
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

#ifndef YASETTINGSBUTTON_H
#define YASETTINGSBUTTON_H

#include <QFrame>
#include <QPointer>

#include "expandingextrawidget.h"
#include "overlaywidget.h"
#include "customtimeline.h"

class YaSelfMoodButton;
class YaSettingsButton;
class YaSettingsButtonExtraButton;
class QPushButton;
class QMenu;
class QAbstractButton;

class YaSettingsButtonExtra : public ExpandingExtraWidget
{
	Q_OBJECT
public:
	YaSettingsButtonExtra(QWidget* parent, YaSettingsButton* placeholderParent);
	~YaSettingsButtonExtra();

	// reimplemented
	bool isExpanded() const;

	// reimplemented
	int normalHeight() const;
	int expandedHeight() const;
	int normalWidth() const;
	int expandedWidth() const;

	// reimplemented
	QSize sizeHint() const;

	void setMenu(QMenu* menu);

	QAbstractButton* button() const;
	void setButton(YaSettingsButtonExtraButton* button);

signals:
	void stateChanged();

private slots:
	// reimplemented
	void animate();

	void buttonClicked();
	void actionActivated();

	void invalidateButtonPositions();

private:
	YaSettingsButtonExtraButton* button_;
	QPointer<QMenu> menu_;

	enum State {
		Normal = 0,
		Expanding,
		Collapsing,
		Expanded
	};
	State state_;

	QList<YaSelfMoodButton*> buttons_;
	QPixmap background_;

	int maskBorders() const;
	int verticalMargin() const;

	// reimplemented
	void setState(State state);
	void setNormalState();

	void updateWidgets();

	// reimplemented
	void invalidateButtons();

	// reimplemented
	void paintEvent(QPaintEvent*);
	void resizeEvent(QResizeEvent* e);
	bool eventFilter(QObject* obj, QEvent* event);
};

class YaSettingsButton : public OverlayWidget<QFrame, YaSettingsButtonExtra>
{
	Q_OBJECT
public:
	YaSettingsButton(QWidget* parent);
	~YaSettingsButton();

	void setMenu(QMenu* menu);
	void setWidgetForTopButton(QWidget* widget);

private:
	// reimplemented
	QRect extraGeometry() const;
};

#endif
