/*
 * yatabbar.cpp
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

#include "yatabbar.h"

#include <QApplication>
#include <QEvent>
#include <QStyleOptionTabV2>
#include <QPainter>
#include <QHelpEvent>
#include <QAction>
#include <QMenu>
#include <QSignalMapper>
#include <QSysInfo>

#include "yatabwidget.h"
#include "yaclosebutton.h"
#include "yachevronbutton.h"
#include "yavisualutil.h"
#include "psitooltip.h"
#include "tabbablewidget.h"
#include "yachatsendbutton.h"

static int margin = 3;

YaTabBar::YaTabBar(QWidget* parent)
	: OverlayWidget<QTabBar, YaChevronButton>(parent,
		new YaChevronButton(parent->window()))
	, previousCurrentIndex_(-1)
	, draggedTabIndex_(-1)
	, timeLine_(new QTimeLine(1500, this))
{
	timeLine_->setFrameRange(0, 100);
	timeLine_->setUpdateInterval(100);
	timeLine_->setLoopCount(0);
	timeLine_->setCurveShape(QTimeLine::SineCurve);
	connect(timeLine_, SIGNAL(frameChanged(int)), this, SLOT(update()));

	connect(this, SIGNAL(currentChanged(int)), SLOT(updateLayout()));

	closeButton_ = new YaCloseButton(this);
	closeButton_->setChatButton(true);
	connect(closeButton_, SIGNAL(clicked()), SIGNAL(closeButtonClicked()));

	activateTabMapper_ = new QSignalMapper(this);
	connect(activateTabMapper_, SIGNAL(mapped(int)), SLOT(setCurrentIndex(int)));

	setAcceptDrops(true);
}

YaTabBar::~YaTabBar()
{
}

int YaTabBar::maximumWidth() const
{
	return static_cast<YaTabWidget*>(parentWidget())->tabRect().width() - 10 - extra()->sizeHint().width();
}

QSize YaTabBar::preferredTabSize(const QString& text, bool current) const
{
	QSize sh;
	sh.setHeight(qMax(fontMetrics().height(), iconSize().height()));
	sh += QSize(0, margin * 2);

	sh.setWidth(0);
	sh += QSize(margin, 0);
	sh += QSize(iconSize().width(), 0);
	sh += QSize(margin, 0);
	int textWidth = fontMetrics().width(text);
	textWidth = qMax(textWidth, fontMetrics().width("nnn"));
	sh += QSize(textWidth, 0);
	sh += QSize(margin, 0);

	if (current) {
		sh += QSize(1, 0);
		sh += QSize(closeButton_->sizeHint().width(), 0);
		sh += QSize(margin, 0);
	}

	return sh;
}

QSize YaTabBar::preferredTabSize(int index) const
{
	return preferredTabSize(tabText(index), index == currentIndex());
}

int YaTabBar::minimumTabWidth() const
{
	return preferredTabSize(QString(), false).width();
}

int YaTabBar::maximumTabWidth() const
{
	return preferredTabSize(QString(20, QChar('n')), true).width();
}

QSize YaTabBar::tabSizeHint(int index) const
{
	if (index >= 0 && index < tabSizeHint_.count())
		return tabSizeHint_[index];
	return QSize(0, 0);
}

bool YaTabBar::needToRelayoutTabs()
{
	QStringList tabNames;
	for (int i = 0; i < count(); ++i)
		tabNames += tabText(i);

	if (previousTabNames_ == tabNames &&
	    previousCurrentIndex_ == currentIndex() &&
	    previousSize_.width() == maximumWidth() &&
	    previousFont_ == font().toString())
	{
		return false;
	}

	previousTabNames_ = tabNames;
	previousCurrentIndex_ = currentIndex();
	previousSize_.setWidth(maximumWidth());
	previousFont_ = font().toString();
	return true;
}

QList<int> YaTabBar::visibleTabs()
{
	QList<int> result;
	for (int i = 0; i < count(); ++i) {
		if (!hiddenTabs_.contains(i))
			result << i;
	}
	return result;
}

QSize YaTabBar::relayoutTab(int index)
{
	QSize sh = preferredTabSize(index);

	int totalTabWidth = 0;
	foreach(int i, visibleTabs()) {
		totalTabWidth += preferredTabSize(i).width();
	}

	if (index != currentIndex()) {
		if (totalTabWidth > maximumWidth()) {
			int w = 0;
			int n = visibleTabs().count() - 1;
			if (n > 0)
				w = ((maximumWidth() - preferredTabSize(currentIndex()).width()) / n);
			sh.setWidth(qMax(minimumTabWidth(), qMin(w, sh.width())));
		}
	}

	bool rightmostTab = index == visibleTabs().last();
	if (totalTabWidth >= maximumWidth() && rightmostTab) {
		int restWidth = 0;
		foreach(int i, visibleTabs()) {
			if (i != index)
				restWidth += relayoutTab(i).width();
		}

		sh.setWidth(qMin(sh.width(), maximumWidth() - restWidth));
	}

	sh.setWidth(qMax(minimumTabWidth(), qMin(sh.width(), maximumTabWidth())));

	return sh;
}

void YaTabBar::relayoutTabsHelper()
{
	tabSizeHint_.clear();

	foreach(int index, visibleTabs()) {
		tabSizeHint_ << relayoutTab(index);
	}
}

void YaTabBar::relayoutTabs()
{
	hiddenTabs_.clear();

	while (1) {
		relayoutTabsHelper();

		if (visibleTabs().isEmpty())
			break;

		int totalTabWidth = 0;
		foreach(QSize s, tabSizeHint_)
			totalTabWidth += s.width();
		if (totalTabWidth <= maximumWidth())
			break;

		Q_ASSERT(!visibleTabs().isEmpty());
		hiddenTabs_ << visibleTabs().last();
	}

	updateHiddenTabActions();
	updateLayout();
}

void YaTabBar::updateHiddenTabActions()
{
	qDeleteAll(hiddenTabsActions_);
	hiddenTabsActions_.clear();

	qSort(hiddenTabs_);

	bool useCheckBoxes = true;
#ifdef Q_WS_WIN
	// on XP when icons in menus are enabled, checkbox is not shown,
	// the black square gets drawn outside of icon
	if (QSysInfo::WindowsVersion == QSysInfo::WV_XP) {
		useCheckBoxes = false;
	}
#endif

	QMenu* menu = new QMenu(extra());
	foreach(int index, hiddenTabs_) {
		QString text = tabText(index);
		if (tabData(index).toBool())
			text = "* " + text;
		QAction* action = new QAction(text, this);
		action->setIcon(tabIcon(index));
		if (useCheckBoxes) {
			action->setCheckable(true);
			action->setChecked(index == currentIndex());
		}
		menu->addAction(action);

		connect(action, SIGNAL(activated()), activateTabMapper_, SLOT(map()));
		activateTabMapper_->setMapping(action, index);

		hiddenTabsActions_ << action;
	}

	if (extra()->menu())
		extra()->menu()->deleteLater();
	extra()->setMenu(menu);
}

void YaTabBar::updateHiddenTabs()
{
	updateHiddenTabActions();
}

QRect YaTabBar::closeButtonRect(const QRect& currentTabRect) const
{
	if (currentTabRect.width() < 10)
		return QRect();
	QRect result(currentTabRect);
	// static const int sizeGripWidth = 12;
	// int sizeGripMargin = qMax(sizeGripWidth - 6 - (maximumWidth() - currentTabRect.right()), 0);
	result.setLeft(result.right() /*- sizeGripMargin*/ - margin - 1 - closeButton_->sizeHint().width());
	result.setSize(closeButton_->sizeHint());
	result.moveCenter(QPoint(result.center().x(), currentTabRect.center().y()));
	return result;
}

QRect YaTabBar::closeButtonRect() const
{
	int x = 0;
	foreach(int i, tabDrawOrder_) {
		QStyleOptionTabV2 tab = getStyleOption(i);
		QRect tabRect(x, 0, tab.rect.width(), tab.rect.height());
		x += tabRect.width();
		if (i == currentIndex())
			return closeButtonRect(tabRect);
	}
	return QRect();
}

QRect YaTabBar::chevronButtonRect() const
{
	QRect r = tabRect(0).adjusted(0, 1, 0, 0);
	r.setWidth(extra()->sizeHint().width());
	r.setHeight(height() - 1);
	r.moveLeft(maximumWidth() + 1);
	return QRect(mapToGlobal(r.topLeft()),
	             mapToGlobal(r.bottomRight()));
}

QRect YaTabBar::extraGeometry() const
{
	return chevronButtonRect();
}

bool YaTabBar::extraShouldBeVisible() const
{
	return !hiddenTabs_.isEmpty();
}

bool YaTabBar::showTabs() const
{
	return count() > 1;
}

void YaTabBar::tabLayoutChange()
{
	if (needToRelayoutTabs()) {
		relayoutTabs();
		return;
	}

	updateTabDrawOrder();
	updateCloseButtonPosition();
	extra()->setVisible(extraShouldBeVisible());
	extra()->raise();
	setVisible(showTabs());

	updateSendButton();
}

void YaTabBar::updateSendButton()
{
	// hack in order to get YaChatSendButtonExtra into correct position
	if (currentTab()) {
		YaChatSendButton* sendButton = currentTab()->findChild<YaChatSendButton*>();
		if (sendButton) {
			sendButton->updatePosition();
		}
	}
}

void YaTabBar::updateCloseButtonPosition(const QRect& closeButtonRect)
{
	if (showTabs()) {
		closeButton_->setGeometry(closeButtonRect);
	}
	closeButton_->setVisible(showTabs());
}


void YaTabBar::updateCloseButtonPosition()
{
	updateCloseButtonPosition(closeButtonRect());
}

QColor YaTabBar::getCurrentColorGrade(const QColor& c1, const QColor& c2, const int sliceCount, const int currentStep) const
{
	double colorStep = sliceCount > 1 ? 1.0 * (currentStep % sliceCount) / (sliceCount - 1) : 0;
	QColor result(
		(int)(c1.red() < c2.red() ? c1.red() + (c2.red() - c1.red()) * colorStep : c1.red() - (c1.red() - c2.red()) * colorStep),
		(int)(c1.green() < c2.green() ? c1.green() + (c2.green() - c1.green()) * colorStep : c1.green() - (c1.green() - c2.green()) * colorStep),
		(int)(c1.blue() < c2.blue() ? c1.blue() + (c2.blue() - c1.blue()) * colorStep : c1.blue() - (c1.blue() - c2.blue()) * colorStep)
	);
	return result;
}

void YaTabBar::startFading()
{
	switch (timeLine_->state()) {
	case QTimeLine::NotRunning:
		timeLine_->start();
		break;
	case QTimeLine::Paused:
		timeLine_->resume();
		break;
	case QTimeLine::Running:
	default:
		break;
	}
}

void YaTabBar::stopFading()
{
	timeLine_->stop();
}

void YaTabBar::updateFading()
{
	updateHiddenTabActions();

	for (int i = 0; i < count(); i++) {
		if (tabData(i).toBool()) {
			startFading();
			return;
		}
	}
	stopFading();
}

bool YaTabBar::drawHighlightChevronBackground() const
{
	foreach(int index, hiddenTabs_) {
		if (tabData(index).toBool())
			return true;
	}
	return false;
}

void YaTabBar::drawTab(QPainter* painter, int index, const QRect& tabRect)
{
	QStyleOptionTabV2 tab = getStyleOption(index);
	if (!(tab.state & QStyle::State_Enabled)) {
		tab.palette.setCurrentColorGroup(QPalette::Disabled);
	}

	// Don't bother drawing a tab if the entire tab is outside of the visible tab bar.
	if (tabRect.right() < 0 || tabRect.left() > width() || tabRect.width() < 3)
		return;

	bool isCurrent = index == currentIndex();
	bool isHovered = tabRect.contains(mapFromGlobal(QCursor::pos())) && draggedTabIndex_ == -1;
	bool isHighlighted = tabData(index).toBool();

	QColor backgroundColor = this->tabBackgroundColor();
	if (isCurrent) {
		backgroundColor = Ya::VisualUtil::editAreaColor();
	}
	else if (isHovered) {
		backgroundColor = Ya::VisualUtil::tabHighlightColor();
	}
	else if (isHighlighted) {
		backgroundColor = highlightColor();
	}

	if (backgroundColor.isValid()) {
		painter->fillRect(tabRect, backgroundColor);
	}

	painter->save();
	painter->setPen(Ya::VisualUtil::rosterTabBorderColor());

	bool drawLeftLine  = tabRect.left() != rect().left();
	bool drawRightLine = true; // tabRect.right() != rect().right() || rect().width() < maximumWidth();

	switch (shape()) {
	case YaTabBar::RoundedSouth:
	case YaTabBar::TriangularSouth:
		if (isCurrent) {
			if (drawLeftLine)
				painter->drawLine(tabRect.topLeft(), tabRect.bottomLeft());
			if (drawRightLine)
				painter->drawLine(tabRect.topRight(), tabRect.bottomRight());
		}
		else {
			painter->drawLine(tabRect.topLeft(), tabRect.topRight());

			if (isHovered) {
				if (currentIndex() != (index - 1))
					if (drawLeftLine)
						painter->drawLine(tabRect.topLeft(), tabRect.bottomLeft());
				if (currentIndex() != (index + 1))
					if (drawRightLine)
						painter->drawLine(tabRect.topRight(), tabRect.bottomRight());
			}
		}
		break;
	default:
		Q_ASSERT(false);
		break;
	}

	painter->restore();

	QRect iconRect(tabRect);
	iconRect.setLeft(iconRect.left() + margin);
	iconRect.setWidth(iconSize().width());

	tabIcon(index).paint(painter, iconRect);

	QRect closeRect;
	if (index == currentIndex()) {
		closeRect = closeButtonRect(tabRect);
	}

	QRect textRect(tabRect);
	textRect.setHeight(fontMetrics().height());
	textRect.moveCenter(tabRect.center());

	textRect.setLeft(iconRect.right() + margin);
	if (closeRect.isNull())
		textRect.setRight(textRect.right() - margin);
	else
		textRect.setRight(closeRect.left() - margin);

	QString text = tabText(index);
	painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, text);
	if (fontMetrics().width(text) > textRect.width() && backgroundColor.isValid()) {
		Ya::VisualUtil::drawTextFadeOut(painter, textRect.adjusted(1, 0, 1, 0), backgroundColor, 15);
	}
}

QColor YaTabBar::highlightColor() const
{
	QColor highlightColor = Ya::VisualUtil::yellowBackgroundColor();
	if (tabBackgroundName() == "orange.png") {
		highlightColor = QColor(0xFF, 0xFF, 0xFF);
	}

	QColor result;
	if (timeLine_->state() == QTimeLine::Running) {
		result = getCurrentColorGrade(
		             tabBackgroundColor(),
		             highlightColor,
		             timeLine_->endFrame() - timeLine_->startFrame(),
		             timeLine_->currentFrame()
		         );
	}
	return result;
}

QRect YaTabBar::draggedTabRect() const
{
	if (draggedTabIndex_ != -1) {
		QStyleOptionTabV2 tab = getStyleOption(draggedTabIndex_);
		QRect tabRect(0, 0, tab.rect.width(), tab.rect.height());
		tabRect.moveLeft(qMin(width() - tabRect.width(),
		                      qMax(0, dragPosition_.x() - dragOffset_.x())));
		return tabRect;
	}
	return QRect();
}

void YaTabBar::paintEvent(QPaintEvent*)
{
	setFont(Ya::VisualUtil::normalFont());

	QStyleOptionTab tabOverlap;
	tabOverlap.shape = shape();

	QPainter p(this);

	// extra()->setHighlightColor(drawHighlightChevronBackground() ? highlightColor() : QColor());
	extra()->setEnableBlinking(drawHighlightChevronBackground());

	updateTabDrawOrder();

	int x = 0;
	foreach(int i, tabDrawOrder_) {
		QStyleOptionTabV2 tab = getStyleOption(i);
		QRect tabRect(x, 0, tab.rect.width(), tab.rect.height());
		x += tabRect.width();

		if (draggedTabIndex_ == i)
			continue;

		drawTab(&p, i, tabRect);
	}

	if (draggedTabIndex_ != -1) {
		QRect tabRect = draggedTabRect();
		drawTab(&p, draggedTabIndex_, tabRect);
		updateCloseButtonPosition(closeButtonRect(tabRect));
	}
}

QStyleOptionTabV2 YaTabBar::getStyleOption(int tab) const
{
	QStyleOptionTabV2 opt;
	opt.init(this);
	opt.state &= ~(QStyle::State_HasFocus | QStyle::State_MouseOver);
	opt.rect = tabRect(tab);
	bool isCurrent = tab == currentIndex();
	opt.row = 0;
//    if (tab == pressedIndex)
//        opt.state |= QStyle::State_Sunken;
	if (isCurrent)
		opt.state |= QStyle::State_Selected;
	if (isCurrent && hasFocus())
		opt.state |= QStyle::State_HasFocus;
	if (isTabEnabled(tab))
		opt.state &= ~QStyle::State_Enabled;
	if (isActiveWindow())
		opt.state |= QStyle::State_Active;
//    if (opt.rect == hoverRect)
//        opt.state |= QStyle::State_MouseOver;
	opt.shape = shape();
	opt.text = tabText(tab);

	if (tabTextColor(tab).isValid())
		opt.palette.setColor(foregroundRole(), tabTextColor(tab));

	opt.icon = tabIcon(tab);
	opt.iconSize = opt.icon.actualSize(QSize(32, 32));  // Will get the default value then.

	int totalTabs = count();

	if (tab > 0 && tab - 1 == currentIndex())
		opt.selectedPosition = QStyleOptionTab::PreviousIsSelected;
	else if (tab < totalTabs - 1 && tab + 1 == currentIndex())
		opt.selectedPosition = QStyleOptionTab::NextIsSelected;
	else
		opt.selectedPosition = QStyleOptionTab::NotAdjacent;

	if (tab == 0) {
		if (totalTabs > 1)
			opt.position = QStyleOptionTab::Beginning;
		else
			opt.position = QStyleOptionTab::OnlyOneTab;
	}
	else if (tab == totalTabs - 1) {
		opt.position = QStyleOptionTab::End;
	}
	else {
		opt.position = QStyleOptionTab::Middle;
	}
	if (const QTabWidget *tw = qobject_cast<const QTabWidget *>(parentWidget())) {
		if (tw->cornerWidget(Qt::TopLeftCorner) || tw->cornerWidget(Qt::BottomLeftCorner))
			opt.cornerWidgets |= QStyleOptionTab::LeftCornerWidget;
		if (tw->cornerWidget(Qt::TopRightCorner) || tw->cornerWidget(Qt::BottomRightCorner))
			opt.cornerWidgets |= QStyleOptionTab::RightCornerWidget;
	}
	return opt;
}

void YaTabBar::updateLayout()
{
	// force re-layout
	QEvent e(QEvent::ActivationChange);
	changeEvent(&e);

	updateSendButton();
}

QSize YaTabBar::minimumSizeHint() const
{
	QSize sh = sizeHint();
	sh.setWidth(10);
	return sh;
}

bool YaTabBar::event(QEvent* event)
{
	if (event->type() == QEvent::ToolTip) {
		QPoint pos = dynamic_cast<QHelpEvent *>(event)->globalPos();
		QPoint localPos = dynamic_cast<QHelpEvent *>(event)->pos();
		QString toolTip;

		int tab = tabAt(localPos);
		if (tabSizeHint(tab).width() < preferredTabSize(tab).width()) {
			toolTip = tabText(tab);
		}

		PsiToolTip::showText(pos, toolTip, this);
		return true;
	}

	return QTabBar::event(event);
}

/**
 * Qt needs to be patched in order to contain virtual method:
 * virtual void QTabBar::setCurrentIndex(int), otherwise
 * all our magic is powerless.
 */
void YaTabBar::setCurrentIndex(int index)
{
	bool updatesEnabled = window()->updatesEnabled();
	window()->setUpdatesEnabled(false);

	OverlayWidget<QTabBar, YaChevronButton>::setCurrentIndex(index);

	// we need to do it after the tab was made current in order for it
	// to get sensible size, so it's layout could be updated immediately
	emit aboutToShow(index);

	window()->setUpdatesEnabled(updatesEnabled);
}

void YaTabBar::mousePressEvent(QMouseEvent* event)
{
	pressedPosition_ = event->pos();
	OverlayWidget<QTabBar, YaChevronButton>::mousePressEvent(event);
}

void YaTabBar::mouseReleaseEvent(QMouseEvent* event)
{
	bool updatesEnabled = this->updatesEnabled();
	setUpdatesEnabled(false);

	if (draggedTabIndex_ != -1) {
		emit reorderTabs(draggedTabIndex_, tabDrawOrder_.indexOf(draggedTabIndex_));
	}

	draggedTabIndex_ = -1;
	pressedPosition_ = QPoint();
	update();
	updateTabDrawOrder();
	updateCloseButtonPosition();

	OverlayWidget<QTabBar, YaChevronButton>::mouseReleaseEvent(event);

	setUpdatesEnabled(updatesEnabled);
}

void YaTabBar::mouseMoveEvent(QMouseEvent* event)
{
	if (draggedTabIndex_ != -1) {
		dragPosition_ = event->pos();
		update();
		return;
	}

	if (!pressedPosition_.isNull() &&
	    (pressedPosition_ - event->pos()).manhattanLength() > QApplication::startDragDistance())
	{
		dragPosition_ = event->pos();
		startDrag();
		return;
	}
	OverlayWidget<QTabBar, YaChevronButton>::mouseMoveEvent(event);
}

void YaTabBar::updateTabDrawOrder()
{
	tabDrawOrder_.clear();
	for (int i = 0; i < count(); ++i) {
		if (i >= tabSizeHint_.count())
			break;

		if (i != draggedTabIndex_)
			tabDrawOrder_ << i;
	}

	if (draggedTabIndex_ != -1) {
		int x = 0;
		int index = 0;
		foreach(int i, tabDrawOrder_) {
			QStyleOptionTabV2 tab = getStyleOption(i);
			QRect tabRect(x, 0, tab.rect.width(), tab.rect.height());
			x += tabRect.width();

			if (draggedTabRect().x() <= tabRect.center().x()) {
				tabDrawOrder_.insert(index, draggedTabIndex_);
				return;
			}

			index++;
		}

		tabDrawOrder_.append(draggedTabIndex_);
	}
}

void YaTabBar::startDrag()
{
	draggedTabIndex_ = tabAt(pressedPosition_);
	if (draggedTabIndex_ != -1 && draggedTabIndex_ == currentIndex()) {
		dragOffset_ = pressedPosition_ - tabRect(draggedTabIndex_).topLeft();
		update();
	}
	else {
		draggedTabIndex_ = -1;
		pressedPosition_ = QPoint();
	}
}

TabbableWidget* YaTabBar::currentTab() const
{
	QTabWidget* tabWidget = dynamic_cast<QTabWidget*>(parentWidget());
	Q_ASSERT(tabWidget);
	TabbableWidget* dlg = tabWidget ? dynamic_cast<TabbableWidget*>(tabWidget->currentWidget()) : 0;
	return dlg;
}

QColor YaTabBar::tabBackgroundColor() const
{
	TabbableWidget* dlg = currentTab();
	if (dlg) {
		return dlg->background().tabBackgroundColor();
	}

	return Ya::VisualUtil::blueBackgroundColor();
}

QString YaTabBar::tabBackgroundName() const
{
	TabbableWidget* dlg = currentTab();
	if (dlg) {
		return dlg->background().name();
	}

	return QString();
}
