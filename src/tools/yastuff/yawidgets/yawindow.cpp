/*
 * yawindow.cpp - custom borderless window class
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

#include "yawindow.h"

#include <QHBoxLayout>
#include <QToolButton>
#include <QPainter>
#include <QSizeGrip>
#include <QApplication>
#include <QDesktopWidget>
#include <QDir>

#define USE_PSIOPTIONS
#define USE_YAVISUALUTIL

#ifdef USE_YAVISUALUTIL
#include "yavisualutil.h"
#endif

static const int borderSize = 3;
static const int cornerSize = 10;

#ifdef USE_PSIOPTIONS
#include "psioptions.h"
static const QString customFrameOptionPath = "options.ya.custom-frame";
static const QString officeBackgroundOptionPath = "options.ya.office-background";
static const QString chatBackgroundOptionPath = "options.ya.chat-background";
#endif

#ifdef Q_WS_WIN
#include <windows.h>
#endif

#include "psitooltip.h"

//----------------------------------------------------------------------------
// YaWindowExtraButton
//----------------------------------------------------------------------------

class YaWindowExtraButton : public QToolButton
{
	Q_OBJECT
public:
	YaWindowExtraButton(QString type, QWidget* parent)
		: QToolButton(parent)
	{
		setCursor(Qt::PointingHandCursor);

		pixmap_ = QPixmap(QString(":/images/window/%1.png").arg(type));
		QSize size(12, 12);
		setFocusPolicy(Qt::NoFocus);
		setFixedSize(size);
		setIconSize(size);

		setAttribute(Qt::WA_Hover, true);
	}

	// reimplemented
	void paintEvent(QPaintEvent*)
	{
		QPainter p(this);
		p.setOpacity(testAttribute(Qt::WA_UnderMouse) ? 1.0 : 0.5);
		p.drawPixmap(0, 0, pixmap_);
	}

private:
	QPixmap pixmap_;
};

//----------------------------------------------------------------------------
// YaWindowExtra
//----------------------------------------------------------------------------

YaWindowExtra::YaWindowExtra(QWidget* parent)
	: QWidget(parent)
	, minimizeEnabled_(true)
	, maximizeEnabled_(true)
{
	hbox_ = new QHBoxLayout(this);
	hbox_->setMargin(0);
	hbox_->setSpacing(3);

	int leftMargin, topMargin, rightMargin, bottomMargin;
	getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);
	setContentsMargins(leftMargin, topMargin + 5, rightMargin, bottomMargin);

	minimizeButton_ = new YaWindowExtraButton("minimize", this);
	connect(minimizeButton_, SIGNAL(clicked()), SIGNAL(minimizeClicked()));
	addButton(minimizeButton_);
	minimizeButton_->setToolTip(tr("Minimize"));
	PsiToolTip::install(minimizeButton_);

	maximizeButton_ = new YaWindowExtraButton("maximize", this);
	connect(maximizeButton_, SIGNAL(clicked()), SIGNAL(maximizeClicked()));
	addButton(maximizeButton_);
	maximizeButton_->setToolTip(tr("Maximize"));
	PsiToolTip::install(maximizeButton_);

	closeButton_ = new YaWindowExtraButton("close", this);
	connect(closeButton_, SIGNAL(clicked()), SIGNAL(closeClicked()));
	addButton(closeButton_);
	closeButton_->setToolTip(tr("Close"));
	PsiToolTip::install(closeButton_);
}

void YaWindowExtra::paintEvent(QPaintEvent*)
{
	// QPainter p(this);
	// p.fillRect(rect(), Qt::red);
}

void YaWindowExtra::addButton(QToolButton* button)
{
	QVBoxLayout* vbox = new QVBoxLayout(0);
	vbox->addSpacing(5);
	vbox->addWidget(button);
	vbox->addStretch();
	hbox_->addLayout(vbox);
}

bool YaWindowExtra::minimizeEnabled() const
{
	return minimizeEnabled_;
}

void YaWindowExtra::setMinimizeEnabled(bool enabled)
{
	minimizeEnabled_ = enabled;
	minimizeButton_->setVisible(enabled);
}

bool YaWindowExtra::maximizeEnabled() const
{
	return maximizeEnabled_;
}

void YaWindowExtra::setMaximizeEnabled(bool enabled)
{
	maximizeEnabled_ = enabled;
	maximizeButton_->setVisible(enabled);
}

void YaWindowExtra::setButtonsVisible(bool visible)
{
	minimizeButton_->setVisible(visible && minimizeEnabled_);
	maximizeButton_->setVisible(visible && maximizeEnabled_);
	closeButton_->setVisible(visible);
}

//----------------------------------------------------------------------------
// YaWindowBase
//----------------------------------------------------------------------------

YaWindowBase::YaWindowBase(QWidget* parent)
	: OverlayWidget<QFrame, YaWindowExtra>(0, new YaWindowExtra(0))
	, mode_(SystemWindowBorder)
	, currentOperation_(None)
	, isInInteractiveMode_(false)
	, staysOnTop_(false)
{
	Q_ASSERT(parent == 0);
	Q_UNUSED(parent);
	setFrameShape(QFrame::NoFrame);

	operationMap_.insert(Move,         OperationInfo(HMove | VMove, Qt::ArrowCursor, false));
	operationMap_.insert(LeftResize,   OperationInfo(HMove | HResize | HResizeReverse, Qt::SizeHorCursor));
	operationMap_.insert(RightResize,  OperationInfo(HResize, Qt::SizeHorCursor));
	operationMap_.insert(TopLeftResize, OperationInfo(HMove | VMove | HResize | VResize | VResizeReverse
	                     | HResizeReverse, Qt::SizeFDiagCursor));
	operationMap_.insert(TopRightResize, OperationInfo(VMove | HResize | VResize
	                     | VResizeReverse, Qt::SizeBDiagCursor));
	operationMap_.insert(TopResize,    OperationInfo(VMove | VResize | VResizeReverse, Qt::SizeVerCursor));
	operationMap_.insert(BottomResize, OperationInfo(VResize, Qt::SizeVerCursor));
	operationMap_.insert(BottomLeftResize,  OperationInfo(HMove | HResize | VResize | HResizeReverse, Qt::SizeBDiagCursor));
	operationMap_.insert(BottomRightResize, OperationInfo(HResize | VResize,
#ifdef Q_WS_MAC
	                     Qt::ArrowCursor
#else
	                     Qt::SizeFDiagCursor
#endif
	                                                     ));

	sizeGrip_ = new QSizeGrip(this);

	extra()->setParent(this);

#ifdef USE_PSIOPTIONS
	optionChanged(customFrameOptionPath);
	connect(PsiOptions::instance(), SIGNAL(optionChanged(const QString&)), SLOT(optionChanged(const QString&)));
#else
	invalidateMode();
#endif

	// In Qt 4.3.2 mouse move events are sent to widgets who track them
	// only if these widgets do not contain child widgets in front of them
	// that don't have mouse tracking enabled. In order to circumvent this
	// case we also install a hook on QApplication to get all MouseMove
	// events.
	setMouseTracking(true);
	qApp->installEventFilter(this);

	connect(extra(), SIGNAL(closeClicked()), SLOT(closeClicked()));
	connect(extra(), SIGNAL(minimizeClicked()), SLOT(minimizeClicked()));
	connect(extra(), SIGNAL(maximizeClicked()), SLOT(maximizeClicked()));
}

YaWindowBase::~YaWindowBase()
{
}

void YaWindowBase::optionChanged(const QString& option)
{
#ifndef USE_PSIOPTIONS
	Q_UNUSED(option);
#else
	if (option == customFrameOptionPath) {
		setMode(PsiOptions::instance()->getOption(customFrameOptionPath).toBool() ?
		        CustomWindowBorder :
		        SystemWindowBorder);
	}
	else if (option == opacityOptionPath_ ||
	         option == officeBackgroundOptionPath ||
	         option == chatBackgroundOptionPath)
	{
		QEvent e(QEvent::ActivationChange);
		changeEvent(&e);
	}
#endif
}


YaWindowBase::Mode YaWindowBase::mode() const
{
	return mode_;
}

void YaWindowBase::setMode(Mode mode)
{
	mode_ = mode;
	invalidateMode();
}

void YaWindowBase::resizeEvent(QResizeEvent* e)
{
	OverlayWidget<QFrame, YaWindowExtra>::resizeEvent(e);
	invalidateMask();
}

void YaWindowBase::invalidateMask()
{
	QRect rect = qApp->desktop()->availableGeometry(this);
	bool square = (rect.top() == geometry().top() &&
	               rect.left() == geometry().left() &&
	               rect.right() == geometry().right());

	int cornerRadius = 6;
#ifdef Q_WS_MAC
	// http://trolltech.com/developer/task-tracker/index_html?method=entry&id=201941
	// currently use of pre-rendered round corners on Mac OS X is quite buggy
	// as it often results a semi-transparent corner image
	// cornerRadius = 10;
#endif
#ifdef USE_YAVISUALUTIL
	if (mode_ == CustomWindowBorder) {
		setMask(Ya::VisualUtil::roundedMask(size(), cornerRadius, !square ? Ya::VisualUtil::TopBorders : Ya::VisualUtil::NoBorders));
	}
	else {
		clearMask();
	}
#endif
}

void YaWindowBase::closeClicked()
{
	if (!extraButtonsShouldBeVisible())
		return;

	close();
}

void YaWindowBase::minimizeClicked()
{
	if (!extraButtonsShouldBeVisible())
		return;

	showMinimized();
}

void YaWindowBase::maximizeClicked()
{
	if (!extraButtonsShouldBeVisible())
		return;

	setYaMaximized(!isYaMaximized());
}

QRect YaWindowBase::yaMaximizedRect() const
{
	Q_ASSERT(extraButtonsShouldBeVisible());
	QRect result = qApp->desktop()->availableGeometry(this);
	if (!expandWidthWhenMaximized()) {
		result.setWidth(geometry().width());
		result.moveLeft(geometry().left());
	}
	return result;
}

bool YaWindowBase::isYaMaximized() const
{
	if (!extraButtonsShouldBeVisible())
		return isMaximized();

	return yaMaximizedRect() == geometry();
}

void YaWindowBase::setYaMaximized(bool maximized)
{
	// if (maximized) {
	// 	Q_ASSERT(maximizeEnabled());
	// }

	sizeGrip_->setVisible(!maximized);

	if (!extraButtonsShouldBeVisible()) {
		if (maximized)
			showMaximized();
		else
			showNormal();
		return;
	}

	if (maximized) {
		Q_ASSERT(!isYaMaximized());
		normalRect_ = geometry();
		setGeometry(yaMaximizedRect());
	}
	else {
		Q_ASSERT(isYaMaximized());
		if (!expandWidthWhenMaximized()) {
			normalRect_.moveLeft(geometry().left());
		}

		if (normalRect_.height() == yaMaximizedRect().height()) {
			normalRect_.setHeight(normalRect_.height() / 2);
		}

		if (normalRect_.width() == yaMaximizedRect().width() && expandWidthWhenMaximized()) {
			normalRect_.setWidth(normalRect_.width() / 2);
		}

		setGeometry(normalRect_);
	}
}

void YaWindowBase::setNormalMode()
{
	currentOperation_ = None;
	updateCursor();
}

void YaWindowBase::mousePressEvent(QMouseEvent* e)
{
	// if (!extraButtonsShouldBeVisible() || isYaMaximized()) {
	// 	setNormalMode();
	// 	return;
	// }

	if (e->button() == Qt::LeftButton) {
		e->accept();
		currentOperation_ = getOperation(e->pos());
		isInInteractiveMode_ = currentOperation_ != None;
		updateCursor();

		mousePressPosition_ = mapToParent(e->pos());
		oldGeometry_ = geometry();
	}
}

void YaWindowBase::mouseReleaseEvent(QMouseEvent* e)
{
	// if (!extraButtonsShouldBeVisible() || isYaMaximized()) {
	// 	setNormalMode();
	// 	return;
	// }

	if (e->button() == Qt::LeftButton) {
		e->accept();
		isInInteractiveMode_ = false;
		currentOperation_ = getOperation(e->pos());
		updateCursor();
	}
}

bool YaWindowBase::eventFilter(QObject* obj, QEvent* event)
{
	if (event->type() == QEvent::MouseMove) {
		QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
		if (mouseEvent->buttons() == Qt::NoButton) {
			QPoint pos = mapFromGlobal(mouseEvent->globalPos());
			if (rect().contains(pos)) {
				event->accept();
				mouseMoveEvent(mouseEvent, pos);
			}
		}
	}

	return OverlayWidget<QFrame, YaWindowExtra>::eventFilter(obj, event);
}

void YaWindowBase::mouseMoveEvent(QMouseEvent* e, QPoint pos)
{
	// if (!extraButtonsShouldBeVisible() || isYaMaximized()) {
	// 	setNormalMode();
	// 	return;
	// }

	if (e->buttons() & Qt::LeftButton && isInInteractiveMode_) {
		e->accept();
		setNewGeometry(mapToParent(pos));
		return;
	}

	currentOperation_ = getOperation(pos);
	updateCursor();
}

void YaWindowBase::mouseMoveEvent(QMouseEvent* e)
{
	mouseMoveEvent(e, e->pos());
}

void YaWindowBase::mouseDoubleClickEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton && extraButtonsShouldBeVisible()) {
		if (getRegion(Move).contains(e->pos())) {
#ifdef Q_WS_MAC
			showMinimized();
#else
			setYaMaximized(!isYaMaximized());
#endif
			e->accept();
			return;
		}
	}

	OverlayWidget<QFrame, YaWindowExtra>::mouseDoubleClickEvent(e);
}

int getMoveDeltaComponent(uint cflags, uint moveFlag, uint resizeFlag, int delta, int maxDelta, int minDelta)
{
	if (cflags & moveFlag) {
		if (delta > 0)
			return (cflags & resizeFlag) ? qMin(delta, maxDelta) : delta;
		return (cflags & resizeFlag) ? qMax(delta, minDelta) : delta;
	}
	return 0;
}

int getResizeDeltaComponent(uint cflags, uint resizeFlag, uint resizeReverseFlag, int delta)
{
	if (cflags & resizeFlag) {
		if (cflags & resizeReverseFlag)
			return -delta;
		return delta;
	}
	return 0;
}

QPoint constraintToAvailableGeometry(const QRect& availableGeometry, const QPoint& _pos)
{
	QPoint pos = _pos;
	if (pos.x() < availableGeometry.left())
		pos.setX(availableGeometry.left());
	if (pos.y() < availableGeometry.top())
		pos.setY(availableGeometry.top());
	if (pos.x() > availableGeometry.right())
		pos.setX(availableGeometry.right());
	if (pos.y() > availableGeometry.bottom())
		pos.setY(availableGeometry.bottom());
	return pos;
}

void YaWindowBase::setNewGeometry(const QPoint& _pos)
{
	QRegion allScreens;
	for (int i = 0; i < qApp->desktop()->numScreens(); ++i)
		allScreens += qApp->desktop()->availableGeometry(i);

	QRect availableGeometry = allScreens.boundingRect();
	// QRect availableGeometry = qApp->desktop()->availableGeometry(this);

	QPoint pos = constraintToAvailableGeometry(availableGeometry, _pos);
	uint cflags = operationMap_.find(currentOperation_).value().changeFlags;

	QRect g;
	if (cflags & (HMove | VMove)) {
		int dx = getMoveDeltaComponent(cflags, HMove, HResize, pos.x() - mousePressPosition_.x(),
		                               oldGeometry_.width() - minimumSize().width(),
		                               oldGeometry_.width() - maximumSize().width());
		int dy = getMoveDeltaComponent(cflags, VMove, VResize, pos.y() - mousePressPosition_.y(),
		                               oldGeometry_.height() - minimumSize().height(),
		                               oldGeometry_.height() - maximumSize().height());
		g.setTopLeft(oldGeometry_.topLeft() + QPoint(dx, dy));
	}
	else {
		g.setTopLeft(geometry().topLeft());
	}

	if (cflags & (HResize | VResize)) {
		int dx = getResizeDeltaComponent(cflags, HResize, HResizeReverse,
		                                 pos.x() - mousePressPosition_.x());
		int dy = getResizeDeltaComponent(cflags, VResize, VResizeReverse,
		                                 pos.y() - mousePressPosition_.y());
		g.setSize(oldGeometry_.size() + QSize(dx, dy));
	}
	else {
		g.setSize(geometry().size());
	}

	// limit the maximum window size to be the size of the current desktop
	if (g.width() > availableGeometry.width())
		g.setWidth(availableGeometry.width());
	if (g.height() > availableGeometry.height())
		g.setHeight(availableGeometry.height());

	// check that window is not moved outside of screen, expecially on
	// Mac OS X. On OSX title should never be higher than the menu bar.
	if (g.top() < availableGeometry.top())
		g.moveTop(availableGeometry.top());

	// if moved completely off-screen, make sure it's visible again
	if (g.right() < availableGeometry.left())
		g.moveLeft(availableGeometry.left());
	if (g.left() > availableGeometry.right())
		g.moveRight(availableGeometry.right());
	if (g.top() > availableGeometry.bottom())
		g.moveBottom(availableGeometry.bottom());

	// TODO: in two-monitor Mac setups when one is placed on top of another and window
	// is dragged between two monitors vertically, this leads to very significant flicker
	// I suspect there is either some Mac or Qt stuff going underneath
	setGeometry(g);
}

void YaWindowBase::invalidateMode()
{
	bool doShow = isVisible();

	updateWindowFlags();
	invalidateMask();

	extra()->setButtonsVisible(extraButtonsShouldBeVisible());

	QPoint p = pos();
	if (p.x() < 0)
		p.setX(0);
	if (p.y() < 0)
		p.setY(0);
	move(p);

	if (doShow)
		show();
}

QRect YaWindowBase::extraGeometry() const
{
	QRect g(rect());
	g.setTopLeft(QPoint(rect().right()  - sizeGrip_->sizeHint().width()  + 1,
	                    rect().bottom() - sizeGrip_->sizeHint().height() + 1));
	sizeGrip_->setGeometry(g);
	sizeGrip_->raise();

	const QSize sh = extra()->sizeHint();
	return QRect(globalRect().x() + width() - sh.width() - 8,
	             globalRect().y() + 0,
	             sh.width(), sh.height());
}

bool YaWindowBase::extraButtonsShouldBeVisible() const
{
	return mode_ == CustomWindowBorder;
}

bool YaWindowBase::isMoveOperation() const
{
	return currentOperation_ == Move;
}

bool YaWindowBase::isResizeOperation() const
{
	return currentOperation_ != None && currentOperation_ != Move;
}


YaWindowBase::Operation YaWindowBase::getOperation(const QPoint& pos) const
{
	OperationInfoMap::const_iterator it;
	for (it = operationMap_.constBegin(); it != operationMap_.constEnd(); ++it) {
		// FIXME: an opportunity to cache getRegion() results
		if (getRegion(it.key()).contains(pos)) {
			if (isYaMaximized()) {
				if (it.key() == Move && !expandWidthWhenMaximized())
					return Move;

				return None;
			}

			return it.key();
		}
	}

	return None;
}

QRegion YaWindowBase::getRegion(Operation operation) const
{
	QRegion leftBorder(0, 0, borderSize, height());
	QRegion topBorder(0, 0, width(), borderSize);
	QRegion bottomBorder(0, height() - borderSize, width(), height());
	QRegion rightBorder(width() - borderSize, 0, width(), height());
	QRegion topLeftBorder(0, 0, cornerSize, cornerSize);
	QRegion topRightBorder(width() - cornerSize, 0, width(), cornerSize);
	QRegion bottomLeftCorner(0, height() - cornerSize, cornerSize, height());
	QRegion bottomRightCorner(width() - cornerSize, height() - cornerSize, width(), height());

	bottomLeftCorner = leftBorder.united(bottomBorder).intersected(bottomLeftCorner);
	bottomRightCorner = rightBorder.united(bottomBorder).intersected(bottomRightCorner);

	int topRegionHeight = height();
#ifdef YAPSI
	topRegionHeight = 70;
#endif
	QRegion fullRegion(0, 0, width(), topRegionHeight);
	QRegion borders = leftBorder.united(bottomBorder).united(rightBorder);
#ifndef Q_WS_MAC
	borders = borders.united(topBorder).united(topLeftBorder).united(topRightBorder);
#endif
	fullRegion = fullRegion.subtracted(borders);

	leftBorder = leftBorder.subtracted(bottomLeftCorner);
	bottomBorder = bottomBorder.subtracted(bottomLeftCorner.united(bottomRightCorner));
	rightBorder = rightBorder.subtracted(bottomRightCorner);

#ifndef Q_WS_MAC
	if (operation == LeftResize)
		return leftBorder;
	if (operation == TopResize)
		return topBorder;
	if (operation == BottomResize)
		return bottomBorder;
	if (operation == RightResize)
		return rightBorder;
	if (operation == TopLeftResize)
		return topLeftBorder;
	if (operation == TopRightResize)
		return topRightBorder;
	if (operation == BottomLeftResize)
		return bottomLeftCorner;
#endif
	if (operation == BottomRightResize)
		return bottomRightCorner;
	if (operation == Move)
		return fullRegion;

	return QRegion();
}

void YaWindowBase::updateCursor()
{
	if (currentOperation_ == None) {
		unsetCursor();
		return;
	}

	if (currentOperation_ == Move || operationMap_.find(currentOperation_).value().hover) {
		setCursor(operationMap_.find(currentOperation_).value().cursorShape);
		return;
	}
}

void YaWindowBase::paintEvent(QPaintEvent*)
{
}

void YaWindowBase::changeEvent(QEvent* e)
{
	OverlayWidget<QFrame, YaWindowExtra>::changeEvent(e);
	if (e->type() == QEvent::ActivationChange) {
		updateOpacity();
		repaintBackground();
	}
}

void YaWindowBase::repaintBackground()
{
	// update() doesn't repaint all the background on Windows in all cases
	repaint();

	// make YaRosterTabButton repaint correctly
	foreach(QWidget* w, findChildren<QWidget*>()) {
		w->repaint();
	}
}

bool YaWindowBase::minimizeEnabled() const
{
	return extra()->minimizeEnabled();
}

void YaWindowBase::setMinimizeEnabled(bool enabled)
{
	extra()->setMinimizeEnabled(enabled);
	updateWindowFlags();
}

bool YaWindowBase::maximizeEnabled() const
{
	return extra()->maximizeEnabled();
}

void YaWindowBase::updateWindowFlags()
{
	setWindowFlags(0);
}

Qt::WindowFlags YaWindowBase::desiredWindowFlags()
{
	Qt::WindowFlags wflags = Qt::Window;
	if (mode_ == CustomWindowBorder)
		wflags |= Qt::FramelessWindowHint;

	// on windows adds context menu to the window taskbar button
	wflags |= Qt::WindowSystemMenuHint;

#ifndef Q_WS_WIN
	if (mode_ != CustomWindowBorder) {
#endif
		// without Qt::WindowMinimizeButtonHint window
		// won't get minimized when its taskbar button
		// is clicked
		// if (minimizeEnabled())
			wflags |= Qt::WindowMinimizeButtonHint;
		// if (maximizeEnabled())
			wflags |= Qt::WindowMaximizeButtonHint;
#ifndef Q_WS_WIN
	}
#endif

	if (staysOnTop_)
		wflags |= Qt::WindowStaysOnTopHint;
	return wflags;
}

void YaWindowBase::setMaximizeEnabled(bool enabled)
{
	extra()->setMaximizeEnabled(enabled);
	updateWindowFlags();
}

bool YaWindowBase::staysOnTop() const
{
	return staysOnTop_;
}

void YaWindowBase::setStaysOnTop(bool staysOnTop)
{
	staysOnTop_ = staysOnTop;
	updateWindowFlags();
}

void YaWindowBase::setOpacityOptionPath(const QString& optionPath)
{
	opacityOptionPath_ = optionPath;
	updateOpacity();
}

void YaWindowBase::updateOpacity()
{
	if (opacityOptionPath_.isEmpty())
		return;

#ifdef USE_PSIOPTIONS
	int maximum_opacity = 100;
	int opacity = PsiOptions::instance()->getOption(opacityOptionPath_).toInt();
#ifdef Q_WS_WIN
	// work-around for severe flickering that happens on windows when borderless
	// windows become completely opaque / non-opaque
	if (opacity < 100) {
		maximum_opacity = 99;
	}
#endif

	if (!isActiveWindow())
		setWindowOpacity(double(qMax(MINIMUM_OPACITY, qMin(opacity, maximum_opacity))) / 100);
	else
		setWindowOpacity(double(maximum_opacity) / 100);
#endif
}

/**
 * Sadly setWindowFlags isn't virtual, so we cannot fully overload it.
 * But nevertheless we try to override it sorta in order to not
 * accidentally reset borderlessness.
 */
void YaWindowBase::setWindowFlags(Qt::WindowFlags type)
{
	QWidget::setWindowFlags(desiredWindowFlags() | type);

#ifdef Q_WS_WIN
	HMENU sysMenu = GetSystemMenu(winId(), false);
	if (sysMenu) {
		// DeleteMenu(sysMenu, SC_MAXIMIZE, MF_BYCOMMAND);
		DeleteMenu(sysMenu, SC_SIZE, MF_BYCOMMAND);
		DeleteMenu(sysMenu, SC_MOVE, MF_BYCOMMAND);
	}
#endif
}

YaWindowExtra* YaWindowBase::windowExtra() const
{
	return extra();
}

/**
 * Overriding this in a roster window in order to return false could be useful.
 */
bool YaWindowBase::expandWidthWhenMaximized() const
{
	return true;
}

bool YaWindowBase::isMoveArea(const QPoint& pos)
{
	return getOperation(pos) == Move;
}

#ifdef Q_OS_WIN
bool YaWindowBase::winEvent(MSG* msg, long* result)
{
	if (msg->message == WM_SYSCOMMAND) {
		if (msg->wParam == SC_MAXIMIZE && mode_ == CustomWindowBorder) {
			maximizeClicked();

			*result = 0;
			return true;
		}
	}

	return OverlayWidget<QFrame, YaWindowExtra>::winEvent(msg, result);
}
#endif

//----------------------------------------------------------------------------
// YaWindow
//----------------------------------------------------------------------------

YaWindow::YaWindow(QWidget* parent, Qt::WindowFlags f)
	: AdvancedWidget<YaWindowBase>(parent, f)
{
}

YaWindow::~YaWindow()
{
}

void YaWindow::setVisible(bool visible)
{
	if (visible && !isVisible() && property("show-offscreen").toBool()) {
		showWidgetOffscreen();
		return;
	}

	AdvancedWidget<YaWindowBase>::setVisible(visible);
}

#include "yawindow.moc"
