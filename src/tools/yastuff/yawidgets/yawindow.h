/*
 * yawindow.h - custom borderless window class
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

#ifndef YAWINDOW_H
#define YAWINDOW_H

#include <QFrame>

#include "overlaywidget.h"
#include "advwidget.h"

class QSizeGrip;
class QToolButton;
class QHBoxLayout;

class YaWindowExtra : public QWidget
{
	Q_OBJECT
public:
	YaWindowExtra(QWidget* parent);

	bool minimizeEnabled() const;
	void setMinimizeEnabled(bool enabled);
	bool maximizeEnabled() const;
	void setMaximizeEnabled(bool enabled);

	void setButtonsVisible(bool visible);

signals:
	void closeClicked();
	void minimizeClicked();
	void maximizeClicked();

protected:
	// reimplemented
	void paintEvent(QPaintEvent*);

private:
	QHBoxLayout* hbox_;
	QToolButton* settingsButton_;
	QToolButton* minimizeButton_;
	QToolButton* maximizeButton_;
	QToolButton* closeButton_;
	bool minimizeEnabled_;
	bool maximizeEnabled_;

	void addButton(QToolButton* button);
};

class YaWindowBase : public OverlayWidget<QFrame, YaWindowExtra>
{
	Q_OBJECT
public:
	YaWindowBase(QWidget* parent = 0);
	~YaWindowBase();

	enum Mode {
		SystemWindowBorder = 0,
		CustomWindowBorder
	};

	Mode mode() const;
	void setMode(Mode mode);

	void setOpacityOptionPath(const QString& optionPath);

	bool minimizeEnabled() const;
	void setMinimizeEnabled(bool enabled);
	bool maximizeEnabled() const;
	void setMaximizeEnabled(bool enabled);

	bool staysOnTop() const;
	void setStaysOnTop(bool staysOnTop);

	bool isMoveArea(const QPoint& pos);

	// reimplemented (sorta)
	void setWindowFlags(Qt::WindowFlags type);

	// reimplemented
	bool eventFilter(QObject* obj, QEvent* event);

	YaWindowExtra* windowExtra() const;

protected:
	// reimplemented
	void resizeEvent(QResizeEvent*);
	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);
	void mouseDoubleClickEvent(QMouseEvent*);
	void paintEvent(QPaintEvent*);
	void changeEvent(QEvent* event);

	void mouseMoveEvent(QMouseEvent*, QPoint pos);

#ifdef Q_OS_WIN
	// reimplemented
	bool winEvent(MSG* msg, long* result);
#endif

protected slots:
	virtual void optionChanged(const QString& option);

private slots:
	void closeClicked();
	void minimizeClicked();
	void maximizeClicked();

private:
	// adapted from QMdiSubWindow class code
	enum Operation {
		None,
		Move,
		LeftResize,
		RightResize,
		TopLeftResize,
		TopRightResize,
		TopResize,
		BottomResize,
		BottomLeftResize,
		BottomRightResize
	};

	enum ChangeFlag {
		HMove          = 1 << 0,
		VMove          = 1 << 1,
		HResize        = 1 << 2,
		VResize        = 1 << 3,
		HResizeReverse = 1 << 4,
		VResizeReverse = 1 << 5
	};

	struct OperationInfo {
		uint changeFlags;
		Qt::CursorShape cursorShape;
		QRegion region;
		bool hover;
		OperationInfo(uint changeFlags, Qt::CursorShape cursorShape, bool hover = true)
			: changeFlags(changeFlags)
			, cursorShape(cursorShape)
			, hover(hover)
		{}
	};

	typedef QMap<Operation, OperationInfo> OperationInfoMap;

	Mode mode_;
	Operation currentOperation_;
	bool isInInteractiveMode_;
	QPoint mousePressPosition_;
	QRect oldGeometry_;
	OperationInfoMap operationMap_;
	QSizeGrip* sizeGrip_;
	QString opacityOptionPath_;
	QRect normalRect_;
	bool staysOnTop_;

	void invalidateMask();
	void invalidateMode();
	void updateWindowFlags();
	Qt::WindowFlags desiredWindowFlags();

	Operation getOperation(const QPoint& pos) const;
	QRegion getRegion(Operation operation) const;
	void updateCursor();
	void setNormalMode();

	void setNewGeometry(const QPoint& pos);
	void updateOpacity();

	bool isMoveOperation() const;
	bool isResizeOperation() const;

	// reimplemented
	virtual QRect extraGeometry() const;
	virtual bool extraButtonsShouldBeVisible() const;

protected:
	virtual void repaintBackground();
	virtual bool expandWidthWhenMaximized() const;

	QRect yaMaximizedRect() const;
	bool isYaMaximized() const;
	void setYaMaximized(bool maximized);
};

class YaWindow : public AdvancedWidget<YaWindowBase>
{
public:
	YaWindow(QWidget* parent = 0, Qt::WindowFlags f = 0);
	~YaWindow();

	// reimplemented
	void setVisible(bool visible);
};

#endif
