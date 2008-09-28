/*
 * yatabwidget.cpp
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

#include "yatabwidget.h"

#include <QSignalMapper>
#include <QPainter>
#include <QEvent>
#include <QPlastiqueStyle>
#include <QApplication>
#include <QVariant>
#include <QAction>

#include "yatabbar.h"
#include "tabbablewidget.h"
#include "yavisualutil.h"

//----------------------------------------------------------------------------
// YaTabWidgetStyle
//----------------------------------------------------------------------------

class YaTabWidgetStyle : public QPlastiqueStyle
{
public:
	YaTabWidgetStyle(QStyle* defaultStyle)
		: defaultStyle_(defaultStyle)
	{}

	// reimplemented
	void polish(QWidget* widget)
	{
		defaultStyle()->polish(widget);
	}

	void unpolish(QWidget* widget)
	{
		defaultStyle()->unpolish(widget);
	}

	void drawControl(ControlElement ce, const QStyleOption* opt, QPainter* p, const QWidget* w) const
	{
		defaultStyle()->drawControl(ce, opt, p, w);
	}

	void drawPrimitive(PrimitiveElement pe, const QStyleOption* opt, QPainter* p, const QWidget* w) const
	{
		defaultStyle()->drawPrimitive(pe, opt, p, w);
	}

	void drawComplexControl(ComplexControl cc, const QStyleOptionComplex* opt, QPainter* p, const QWidget* widget) const
	{
		defaultStyle()->drawComplexControl(cc, opt, p, widget);
	}

	QSize sizeFromContents(ContentsType ct, const QStyleOption* opt, const QSize& contentsSize, const QWidget* w) const
	{
		return defaultStyle()->sizeFromContents(ct, opt, contentsSize, w);
	}

	SubControl hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex* opt, const QPoint& pos, const QWidget* widget) const
	{
		return defaultStyle()->hitTestComplexControl(cc, opt, pos, widget);
	}

	QRect subControlRect(ComplexControl cc, const QStyleOptionComplex* opt, SubControl sc, const QWidget* widget) const
	{
		return defaultStyle()->subControlRect(cc, opt, sc, widget);
	}

	int styleHint(StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData) const
	{
		switch (hint) {
		case SH_TabBar_Alignment:
			return Qt::AlignLeft;
		case SH_TabBar_PreferNoArrows:
			return true;
		default:
			break;
		}
		return defaultStyle()->styleHint(hint, option, widget, returnData);
	}

	int pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
	{
		switch (metric) {
		case PM_TabBarScrollButtonWidth:
			return 0;
		default:
			break;
		}
		return QPlastiqueStyle::pixelMetric(metric, option, widget);
		// this one gets us into forever recursion on Qt 4.3.3.
		// return defaultStyle()->pixelMetric(metric, option, widget);
	}

	QRect subElementRect(SubElement element, const QStyleOption* option, const QWidget* widget) const
	{
		QRect result = defaultStyle()->subElementRect(element, option, widget);
		switch (element) {
		case SE_TabWidgetTabBar: {
			const YaTabWidget* tw = dynamic_cast<const YaTabWidget*>(widget);
			if (tw) {
				QSize sh = tw->tabSizeHint();
				QRect r  = tw->tabRect();
				result.setLeft(r.left());
				result.setWidth(qMin(r.right(), sh.width()));
				result.setHeight(sh.height() - (widget->height() - r.height()));
			}
			return result;
		}
		case SE_TabWidgetTabContents:
			return result.adjusted(-2, -2, 2, 0);
		default:
			return result;
		}
	}

private:
	QStyle* defaultStyle_;
	QStyle* defaultStyle() const { return defaultStyle_; }
};

//----------------------------------------------------------------------------
// YaTabWidget
//----------------------------------------------------------------------------

YaTabWidget::YaTabWidget(QWidget* parent)
	: QTabWidget(parent)
{
	YaTabWidgetStyle* style = new YaTabWidgetStyle(qApp->style());
	style->setParent(this);
	setStyle(style);

	YaTabBar* tabBar = new YaTabBar(this);
	connect(tabBar, SIGNAL(closeButtonClicked()), SIGNAL(closeButtonClicked()));
	connect(tabBar, SIGNAL(aboutToShow(int)), SLOT(aboutToShow(int)));
	connect(tabBar, SIGNAL(reorderTabs(int, int)), SIGNAL(reorderTabs(int, int)));

	setTabBar(tabBar);
	setTabPosition(South);

	QSignalMapper* activateTabMapper_ = new QSignalMapper(this);
	connect(activateTabMapper_, SIGNAL(mapped(int)), tabBar, SLOT(setCurrentIndex(int)));
	for (int i = 0; i < 10; ++i) {
		QAction* action = new QAction(this);
		connect(action, SIGNAL(activated()), activateTabMapper_, SLOT(map()));
		action->setShortcut(QKeySequence(QString("Ctrl+%1").arg(i)));
		activateTabMapper_->setMapping(action, (i > 0 ? i : 10) - 1);
		addAction(action);
	}
}

YaTabWidget::~YaTabWidget()
{
}

void YaTabWidget::paintEvent(QPaintEvent*)
{
	QPainter p(this);

	TabbableWidget* dlg = dynamic_cast<TabbableWidget*>(currentWidget());
	if (dlg) {
		dlg->background().paint(&p, rect(), isActiveWindow());
	}

	if (tabBar()->isVisible()) {
		QRect r = tabRect();
		int y = tabBar()->geometry().top();
		p.fillRect(QRect(QPoint(r.left(), y), r.bottomRight()), static_cast<YaTabBar*>(tabBar())->tabBackgroundColor());

		p.setPen(Ya::VisualUtil::rosterTabBorderColor());
		p.drawLine(tabBar()->geometry().topLeft(), QPoint(r.right(), y));
	}
}

QSize YaTabWidget::tabSizeHint() const
{
	return tabBar()->sizeHint();
}

QRect YaTabWidget::tabRect() const
{
	int margin = 2;
	return rect().adjusted(margin, 0, -margin, -margin);
}

void YaTabWidget::resizeEvent(QResizeEvent* e)
{
	QTabWidget::resizeEvent(e);
	static_cast<YaTabBar*>(tabBar())->updateLayout();
}

void YaTabWidget::tabInserted(int)
{
	updateLayout();
}

void YaTabWidget::tabRemoved(int)
{
	updateLayout();
}

void YaTabWidget::updateLayout()
{
	// force re-layout
	QEvent e(QEvent::LayoutRequest);
	event(&e);
}

bool YaTabWidget::tabHighlighted(int index) const
{
	return tabBar()->tabData(index).toBool();
}

void YaTabWidget::setTabHighlighted(int index, bool highlighted)
{
	YaTabBar* tb = (YaTabBar*)tabBar();
	bool previouslyHighlighted = tb->tabData(index).toBool();
	tb->setTabData(index, QVariant(highlighted));
	if (previouslyHighlighted != highlighted)
		tb->updateFading();
}

void YaTabWidget::updateHiddenTabs()
{
	static_cast<YaTabBar*>(tabBar())->updateHiddenTabs();
}

void YaTabWidget::aboutToShow(int index)
{
	TabbableWidget* tab = dynamic_cast<TabbableWidget*>(widget(index));
	if (tab)
		tab->aboutToShow();
}
