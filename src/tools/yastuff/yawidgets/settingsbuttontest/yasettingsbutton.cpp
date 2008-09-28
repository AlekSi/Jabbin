/*
 * yasettingsbutton.cpp - settings gear button
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

#include "yasettingsbutton.h"

#include <QPainter>
#include <QPushButton>
#include <QAction>
#include <QMenu>
#include <QPlastiqueStyle>
#include <QToolButton>
#include <QVBoxLayout>

#include "yavisualutil.h"
#include "pixmaputil.h"
#include "psitooltip.h"

//----------------------------------------------------------------------------
// YaSettingsButtonExtraButton
//----------------------------------------------------------------------------

class YaSettingsButtonExtraButton : public QToolButton
{
	Q_OBJECT
public:
	YaSettingsButtonExtraButton(QWidget* parent)
		: QToolButton(parent)
		, extra_(0)
	{
		Q_ASSERT(dynamic_cast<QBoxLayout*>(parent->layout()));

		setCursor(Qt::PointingHandCursor);

		QVBoxLayout* vbox = new QVBoxLayout(0);
		vbox->addSpacing(3);
		vbox->addWidget(this);

		QBoxLayout* parentLayout = dynamic_cast<QBoxLayout*>(parent->layout());
		Q_ASSERT(parentLayout);
		parentLayout->setSpacing(4);
		parentLayout->insertLayout(0, vbox);

		// copied from YaWindowExtraButton
		normalPixmap_   = QPixmap(QString(":/images/window/gear_normal.png"));
		hoveredPixmap_  = QPixmap(QString(":/images/window/gear_hovered.png"));
		selectedPixmap_ = QPixmap(QString(":/images/window/gear_selected.png"));
		selectedBackground_ = QPixmap(QString(":/images/window/gear_selected_background.png"));
		QSize size(14, 17);
		setFocusPolicy(Qt::NoFocus);
		setFixedSize(size);
		setIconSize(size);

		setToolTip(tr("Settings"));
		PsiToolTip::install(this);

		setAttribute(Qt::WA_Hover, true);
	}

	bool isTopRight()
	{
		return geometry().right() == parentWidget()->rect().right();
	}

	// reimplemented
	void paintEvent(QPaintEvent*)
	{
		QPainter p(this);
		if (extra_ && (extra_->isAnimating() || extra_->isExpanded())) {
			// p.fillRect(rect(), Ya::VisualUtil::blueBackgroundColor());
			p.drawPixmap(0, yOffset(), selectedBackground_);
			drawGear(&p, selectedPixmap_);
			return;
		}

		drawGear(&p, testAttribute(Qt::WA_UnderMouse) || isDown() ? hoveredPixmap_ : normalPixmap_);
	}

	void setExtra(YaSettingsButtonExtra* extra)
	{
		Q_ASSERT(extra);
		extra_ = extra;
		connect(extra_, SIGNAL(stateChanged()), SLOT(update()));
	}

private:
	YaSettingsButtonExtra* extra_;
	QPixmap normalPixmap_;
	QPixmap hoveredPixmap_;
	QPixmap selectedPixmap_;
	QPixmap selectedBackground_;

	int yOffset() const
	{
		return 0;
	}

	void drawGear(QPainter* p, const QPixmap& pix)
	{
		p->drawPixmap((width() - pix.width()) / 2, yOffset(), pix);
	}
};

//----------------------------------------------------------------------------
// YaSettingsButtonExtra
//----------------------------------------------------------------------------

YaSettingsButtonExtra::YaSettingsButtonExtra(QWidget* parent, YaSettingsButton* placeholderParent)
	: ExpandingExtraWidget(parent, placeholderParent)
	, button_(0)
	, state_(Normal)
{
	background_ = QPixmap(":images/rostertab_background_inactive.png");
}

YaSettingsButtonExtra::~YaSettingsButtonExtra()
{
}

QAbstractButton* YaSettingsButtonExtra::button() const
{
	return button_;
}

void YaSettingsButtonExtra::setButton(YaSettingsButtonExtraButton* button)
{
	Q_ASSERT(!button_);
	Q_ASSERT(dynamic_cast<YaSettingsButtonExtraButton*>(button));
	button_ = button;
	dynamic_cast<YaSettingsButtonExtraButton*>(button)->setExtra(this);
	connect(button_, SIGNAL(clicked()), SLOT(buttonClicked()));
	button_->installEventFilter(this);
	updateWidgets();
}

bool YaSettingsButtonExtra::isExpanded() const
{
	return state_ != Normal;
}

QSize YaSettingsButtonExtra::sizeHint() const
{
	switch (state_) {
	case Expanding:
	case Collapsing:
		return currentAnimationFrame();
	case Expanded:
		return QSize(expandedWidth(), expandedHeight());
	default:
		return QSize(normalWidth(), normalHeight());
	}
}

int YaSettingsButtonExtra::normalHeight() const
{
	return 0;
}

int YaSettingsButtonExtra::verticalMargin() const
{
	return background_.height() / 2;
}

int YaSettingsButtonExtra::expandedHeight() const
{
	int h = buttonsBottom();
	if (h != -1)
		return h + verticalMargin();

	return normalHeight();
}

int YaSettingsButtonExtra::normalWidth() const
{
	if (!button_)
		return 0;
	return button_->width() * 3;
}

int YaSettingsButtonExtra::expandedWidth() const
{
	return parentWidget()->width() - (5 * 2);
	// QRect r(placeholderParent()->mapToGlobal(placeholderParent()->rect().topLeft()),
	//         placeholderParent()->mapToGlobal(placeholderParent()->rect().bottomRight()));
	// QWidget* window = placeholderParent()->window();
	// r.setLeft(window->mapToGlobal(window->rect().topLeft()).x() + 5);
	// return r.width();
}

void YaSettingsButtonExtra::animate()
{
	switch (state_) {
	case Expanding:
		if (currentAnimationFrame().height() >= expandedHeight()) {
			setState(Expanded);
		}
		break;
	case Collapsing:
		if (currentAnimationFrame().height() <= normalHeight()) {
			setState(Normal);
		}
		break;
	default:
		;
	}

	emit invalidateExtraPosition();
}

void YaSettingsButtonExtra::setState(State state)
{
	if (state_ == state)
		return;

	state_ = state;
	emit stateChanged();

	updateButtonsVisibility(state_ == Expanded);

	switch (state_) {
	case Expanding:
		setExpandedState();
		if (button_)
			button_->parentWidget()->raise();
		startAnimation(ExpandingExtraWidget::ExpandingAnimation, QSize(expandedWidth(), expandedHeight()));
		break;
	case Collapsing:
		startAnimation(ExpandingExtraWidget::CollapsingAnimation, QSize(normalWidth(), normalHeight()));
		break;
	default:
		;
	}
}

int YaSettingsButtonExtra::maskBorders() const
{
	if (!button_)
		return 0;

	return button_->isTopRight() ? Ya::VisualUtil::LeftBorders |
	                               Ya::VisualUtil::BottomBorders :
	                               Ya::VisualUtil::AllBorders;
}

void YaSettingsButtonExtra::paintEvent(QPaintEvent* e)
{
	QWidget::paintEvent(e);
	QPainter p(this);
	p.fillRect(rect(), Qt::white);

	QRegion buttonRegion = Ya::VisualUtil::roundedMask(QSize(width() - 2, height() - 2),
	                       Ya::VisualUtil::rosterGroupCornerRadius(),
	                       static_cast<Ya::VisualUtil::Borders>(maskBorders()));
	buttonRegion.translate(1, 1);
	p.setClipRegion(buttonRegion);

	p.drawTiledPixmap(QRect(0, height() - background_.height() - 1, width(), background_.height()),
	                  PixmapUtil::rotatedPixmap(180, background_));
	p.drawTiledPixmap(QRect(0, 1, width(), background_.height()), background_);
}


void YaSettingsButtonExtra::resizeEvent(QResizeEvent* e)
{
	if (!button_)
		return;

	QWidget::resizeEvent(e);
	setMask(Ya::VisualUtil::roundedMask(size(), Ya::VisualUtil::rosterGroupCornerRadius(),
	                                    static_cast<Ya::VisualUtil::Borders>(maskBorders())));
	updateWidgets();
}

void YaSettingsButtonExtra::updateWidgets()
{
	if (state_ == Normal || state_ == Expanded) {
		invalidateButtonPositions();
	}
}

void YaSettingsButtonExtra::invalidateButtonPositions()
{
	if (!button_)
		return;
	QRect menuRect(rect());
	menuRect.setLeft(margin());
	menuRect.setTop(verticalMargin());
	menuRect.setRight(menuRect.right() - margin());
	menuRect.setBottom(menuRect.top() + margin());
	updateButtonsPositions(menuRect);
}

void YaSettingsButtonExtra::buttonClicked()
{
	return;

	if (isExpanded())
		setState(Collapsing);
	else
		setState(Expanding);
}

void YaSettingsButtonExtra::setMenu(QMenu* menu)
{
	if (!menu_.isNull()) {
		foreach(QAction* action, menu_->actions()) {
			disconnect(action, SIGNAL(changed()), this, SLOT(invalidateButtonPositions()));
			disconnect(action, SIGNAL(activated()), this, SLOT(actionActivated()));
		}
	}

	menu_ = menu;
	button_->setMenu(menu_);
	button_->setPopupMode(QToolButton::InstantPopup);

	if (!menu_.isNull()) {
		foreach(QAction* action, menu_->actions()) {
			connect(action, SIGNAL(changed()), this, SLOT(invalidateButtonPositions()));
			connect(action, SIGNAL(activated()), this, SLOT(actionActivated()));
		}
	}

	invalidateButtons();
}

void YaSettingsButtonExtra::actionActivated()
{
	if (isExpanded())
		setState(Collapsing);
}

void YaSettingsButtonExtra::setNormalState()
{
	ExpandingExtraWidget::setNormalState();

	if (state_ == Normal || state_ == Collapsing)
		return;

	setState(Collapsing);
}

bool YaSettingsButtonExtra::eventFilter(QObject* obj, QEvent* event)
{
	if ((obj == this) && event->type() == QEvent::MouseButtonRelease) {
		event->accept();

		QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
		if (rect().contains(mouseEvent->pos()) && !isAnimating()) {
			buttonClicked();
			return true;
		}
	}

	return ExpandingExtraWidget::eventFilter(obj, event);
}

void YaSettingsButtonExtra::invalidateButtons()
{
	ExpandingExtraWidget::invalidateButtons();

	if (!menu_.isNull()) {
		foreach(QAction* action, menu_->actions()) {
			createButtonFor(action);
		}
	}

	updateWidgets();
}

//----------------------------------------------------------------------------
// YaSettingsButton
//----------------------------------------------------------------------------

YaSettingsButton::YaSettingsButton(QWidget* parent)
	: OverlayWidget<QFrame, YaSettingsButtonExtra>(parent,
		new YaSettingsButtonExtra(parent->window(), this))
{
	setFocusPolicy(Qt::NoFocus);
}

YaSettingsButton::~YaSettingsButton()
{
}

QRect YaSettingsButton::extraGeometry() const
{
	if (!extra()->button())
		return QRect();

	QRect buttonRect = globalRect(extra()->button(), extra()->button()->geometry());
	QRect windowRect = globalRect(window(), window()->rect());

	int maxW = extra()->isExpanded() ? extra()->expandedWidth() : width();
	int w = qMin(extra()->sizeHint().width(), maxW);
	return QRect(windowRect.right() - extra()->margin() - w + 2,
	             buttonRect.bottom() + 0,
	             w,
	             extra()->sizeHint().height());
}

void YaSettingsButton::setMenu(QMenu* menu)
{
	extra()->setMenu(menu);
}

void YaSettingsButton::setWidgetForTopButton(QWidget* widget)
{
	YaSettingsButtonExtraButton* button = new YaSettingsButtonExtraButton(widget);
	extra()->setButton(button);
}

#include "yasettingsbutton.moc"
