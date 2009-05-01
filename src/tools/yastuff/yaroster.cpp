/*
 * yaroster.cpp - widget that handles contact-list
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

#include "yaroster.h"

#include <QPainter>
#include <QEvent>
#include <QKeyEvent>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QPointer>
#include <QTextDocument> // for Qt::escape
#include <QScrollArea>
#include <QSizeGrip>

#include "yatoolbox.h"
#include "yacontactlistview.h"
#include "yacontactlistviewdelegate.h"
#include "yatoolboxpage.h"
#include "contactlistproxymodel.h"
#include "yacontactlistcontactsmodel.h"
#include "yacontactlistfriendsmodel.h"
#include "yaexpandingoverlaylineedit.h"
#include "contactlistmodel.h"
#include "yavisualutil.h"
#include "psioptions.h"
#ifdef MODELTEST
#include "modeltest.h"
#endif
#include "shortcutmanager.h"
#include "yaloginpage.h"
#include "psicontactlist.h"
#include "yafilteredcontactlistview.h"
#include "yarostertooltip.h"
#include "yachattooltip.h"
#ifdef YAPSI_FANCY_DELETE_CONFIRMATIONS
#include "yamulticontactconfirmationtooltip.h"
#endif
#include "psiaccount.h"
#include "psicontact.h"
#include "yaprivacymanager.h"
#include "vcardfactory.h"
#include "xmpp_tasks.h"
#include "yacontactlistmodelselection.h"
#include "animatedstackedwidget.h"
#include "yaremoveconfirmationmessagebox.h"
#include "desktoputil.h"
#include "yawindowbackground.h"
#include "yaokbutton.h"
#include "yaclosebutton.h"
#include "textutil.h"
#include "fakegroupcontact.h"
#include "yacommon.h"
#include "tools/services/servicespopupbutton.h"

static const QString tabIndexOptionPath = "options.ya.main-window.tab-index";
static const QString showContactListGroupsOptionPath = "options.ya.main-window.contact-list.show-groups";
static const QString avatarStyleOptionPath = "options.ya.main-window.contact-list.avatar-style";

#define CONTACTLIST_UNSELECT_ON_CLICK_OUTSIDE

//----------------------------------------------------------------------------
// YaRosterToolButton
//----------------------------------------------------------------------------

YaRosterToolButton::YaRosterToolButton(QWidget* parent)
	: QToolButton(parent)
	, underMouse_(false)
{
}

void YaRosterToolButton::setUnderMouse(bool underMouse)
{
	underMouse_ = underMouse;
	if (!underMouse) {
		setDown(false);
	}
	update();
}

void YaRosterToolButton::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	// p.fillRect(rect(), Qt::red);

	QPixmap pixmap = icon().pixmap(size(), iconMode());

	if (!isDown() && !underMouse() && !isChecked() && !underMouse_ || (!isEnabled() && !isChecked()))
		p.setOpacity(0.5);

	int yOffset = 0 + (isDown() ? 1 : 0);
	if (!textVisible()) {
		p.drawPixmap((width() - pixmap.width()) / 2,
		             (height() - pixmap.height()) / 2 + yOffset,
		             pixmap);
	}
	else {
		QRect r = rect();
		r.setWidth(sizeHint().width());
		r.moveLeft((rect().width() - r.width()) / 2);

		p.drawPixmap(r.left(),
		             (r.height() - pixmap.height()) / 2 + yOffset,
		             pixmap);

		QFont f = p.font();
		f.setUnderline(true);
		p.setFont(f);
		p.setPen(QColor(0x03, 0x94, 0x00));
		p.drawText(r.adjusted(0, yOffset, 0, yOffset),
		           Qt::AlignRight | Qt::AlignVCenter, text());
	}
}

QIcon::Mode YaRosterToolButton::iconMode() const
{
	return /* isEnabled() ? */ QIcon::Normal /* : QIcon::Disabled */;
}

bool YaRosterToolButton::textVisible() const
{
	return !text().isEmpty() && isEnabled();
}

void YaRosterToolButton::setCompactSize(const QSize& size)
{
	compactSize_ = size;

	QEvent e(QEvent::EnabledChange);
	QCoreApplication::instance()->sendEvent(this, &e);
}

QSize YaRosterToolButton::minimumSizeHint() const
{
	QSize sh = compactSize_;
	if (textVisible()) {
		// sh.setWidth(sh.width() + 5);
		sh.setWidth(sh.width() + fontMetrics().width(text()));
	}
	return sh;
}

QSize YaRosterToolButton::sizeHint() const
{
	return minimumSizeHint();
}

void YaRosterToolButton::changeEvent(QEvent* e)
{
	QToolButton::changeEvent(e);

	if (e->type() == QEvent::EnabledChange) {
		if (toolTip_.isEmpty() && !toolTip().isEmpty()) {
			toolTip_ = toolTip();
		}

		setToolTip(isEnabled() ? toolTip_ : QString());

		if (textVisible()) {
			setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			setMinimumWidth(0);
			setMaximumWidth(1000);
			setFixedHeight(compactSize_.height());
		}
		else {
			// setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
			setFixedSize(compactSize_);
		}
		updateGeometry();
	}
}

//----------------------------------------------------------------------------
// YaRosterAddContactToolButton
//----------------------------------------------------------------------------

YaRosterAddContactToolButton::YaRosterAddContactToolButton(QWidget* parent, YaRoster* yaRoster)
	: YaRosterToolButton(parent)
	, yaRoster_(yaRoster)
{
	connect(yaRoster_, SIGNAL(availableAccountsChanged(bool)), SLOT(update()));
}

QIcon::Mode YaRosterAddContactToolButton::iconMode() const
{
	return yaRoster_ && yaRoster_->haveAvailableAccounts() ? QIcon::Normal : QIcon::Disabled;
}

//----------------------------------------------------------------------------
// YaCommonNoContactsPage
//----------------------------------------------------------------------------

class YaCommonNoContactsPage : public QScrollArea
{
	Q_OBJECT
public:
	YaCommonNoContactsPage(QWidget* parent)
		: QScrollArea(parent)
	{
		widget_ = new QWidget(this);
		widget_->installEventFilter(this);
		setFrameShape(QFrame::NoFrame);

		vbox_ = new QVBoxLayout(widget_);
	}

	virtual void init()
	{
		foreach(QLabel* label, widget_->findChildren<QLabel*>()) {
			connect(label, SIGNAL(linkActivated(const QString&)), SLOT(linkActivated(const QString&)));
			label->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
		}

#if QT_VERSION < 0x040401
		widget_->setMinimumHeight(400);
#endif
		setWidget(widget_);
		setWidgetResizable(true);
	}

	// reimplemented
	void paintEvent(QPaintEvent* e)
	{
		QScrollArea::paintEvent(e);
	}

	bool eventFilter(QObject* obj, QEvent* e)
	{
		if (e->type() == QEvent::Paint && obj == widget_) {
			QPainter p(widget_);
			p.fillRect(widget_->rect(), Qt::white);
			return true;
		}
		return QScrollArea::eventFilter(obj, e);
	}

signals:
	void addContact();

protected:
	QVBoxLayout* vbox() const
	{
		return vbox_;
	}

	QLabel* addTextLabel(const QStringList& paragraphs)
	{
		QHBoxLayout* hbox = new QHBoxLayout(0);
		QLabel* label = new QLabel(widget_);

		QString text = QString::fromUtf8(
			"<html><head><style type=\"text/css\">"
			"p, li { white-space: pre-wrap; font-size:12px; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; }"
			"</style></head>"
			"<body>"
		);
		foreach(QString p, paragraphs) {
			if (p.isEmpty()) {
				text += "<br>";
			}
			else {
				text += QString::fromUtf8("<p align=\"center\" style=\"color:#000000;\">%1</p>").arg(p);
			}
		}
		text += QString::fromUtf8("</body></html>");
		label->setText(text);
		label->setWordWrap(true);
		hbox->addWidget(label);
		vbox()->addLayout(hbox);

		return label;
	}

	void addPixmap(const QString& fileName)
	{
		QHBoxLayout* hbox = new QHBoxLayout(0);
		QLabel* label = new QLabel(widget_);
		label->setPixmap(QPixmap(fileName));
		hbox->addStretch();
		hbox->addWidget(label);
		hbox->addStretch();
		vbox()->addLayout(hbox);
	}

	void addYaRuLabel(const QString& text)
	{
		addTextLabel(QStringList() << text);
		addPixmap(":images/no-contacts/yaru.png");
	}

private slots:
	void linkActivated(const QString& link)
	{
		// we don't want nasty rectangles around clicked links
		foreach(QLabel* label, findChildren<QLabel*>()) {
			QMouseEvent me(QEvent::MouseButtonPress, QPoint(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
			qApp->sendEvent(label, &me);
		}

		if (link == "yachat://add-contact") {
			emit addContact();
		}
		else if (link == "yachat://find-friends") {
			DesktopUtil::openYaUrl("http://www.jabbin.com");
		}
		else {
			Q_ASSERT(false);
		}
	}

private:
	QWidget* widget_;
	QVBoxLayout* vbox_;
};

//----------------------------------------------------------------------------
// YaNoContactsPage
//----------------------------------------------------------------------------

class YaNoContactsPage : public YaCommonNoContactsPage
{
	Q_OBJECT
public:
	YaNoContactsPage(QWidget* parent)
		: YaCommonNoContactsPage(parent)
	{
	}

	// reimplemented
	virtual void init()
	{
		vbox()->addStretch();

		// addPixmap(":iconsets/system/default/logo_96.png");

		vbox()->addSpacing(0);

		addTextLabel(QStringList()
			<< QString::fromUtf8("Welcome")
			<< QString::fromUtf8("You can now start using Jabberout service!")
			<< QString("")
			<< QString::fromUtf8("Jabberout is more than just an IM. You can chat/speak with your friends and update them with your moods and your blog")
		);

		addPixmap(":images/addcontact.png");

		addYaRuLabel(QString::fromUtf8("After adding your Jabberout account you can start adding your friends to your contact list"));

		vbox()->addStretch();

		YaCommonNoContactsPage::init();
	}
};

//----------------------------------------------------------------------------
// YaAddContactPage
//----------------------------------------------------------------------------

class YaAddContactPage : public YaCommonNoContactsPage
{
	Q_OBJECT
public:
	YaAddContactPage(QWidget* parent)
		: YaCommonNoContactsPage(parent)
	{
	}

	// reimplemented
	virtual void init()
	{
		label_ = addTextLabel(QStringList());
		// vbox()->addStretch();

		// addPixmap(":iconsets/system/default/logo_96.png");

		vbox()->addSpacing(10);

		addTextLabel(QStringList()
			<< QString::fromUtf8("Please insert the nick (jabber id) of the friend you want to add in the above box. For example: user@jabbin.com")
		);

		vbox()->addSpacing(10);

		addYaRuLabel(QString::fromUtf8("You can find new friends on <a href=\"yachat://find-friends\"></a> — Jabberout service."));

		vbox()->addStretch();

		YaCommonNoContactsPage::init();
	}

	void setError(const QString& error)
	{
		errorText_ = error;
		updateLabel();
	}

	void setStatusText(const QString& statusText)
	{
		statusText_ = statusText;
		updateLabel();
	}

private:
	void updateLabel()
	{
		QStringList lines;

		if (!errorText_.isEmpty()) {
			QString error = tr("Error: %1").arg(errorText_);
			lines += QString("<p align=\"left\" style=\"color:#F00000;\">%1</p>").arg(TextUtil::plain2richSimple(error));
		}

		if (!statusText_.isEmpty()) {
			lines += QString("<p align=\"left\" style=\"color:#000000;\">%1</p>").arg(TextUtil::plain2richSimple(statusText_));
		}

		label_->setText(lines.join("<br>"));
		label_->setFixedHeight(lines.isEmpty() ? 0 : 70);
	}

private:
	QLabel* label_;
	QString errorText_;
	QString statusText_;
};

//----------------------------------------------------------------------------
// YaRosterFilterProxyModel
//----------------------------------------------------------------------------

class YaRosterFilterProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	YaRosterFilterProxyModel(QObject* parent)
		: QSortFilterProxyModel(parent)
	{
		sort(0, Qt::AscendingOrder);
		setDynamicSortFilter(true);
		setFilterCaseSensitivity(Qt::CaseInsensitive);
		setSortLocaleAware(true);
	}

protected:
	// reimplemented
	bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
	{
		QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

		QStringList data;
		if (index.isValid()) {
			// TODO: also check for vCard value
			data
				<< index.data(Qt::DisplayRole).toString()
				<< index.data(ContactListModel::JidRole).toString();
		}

		foreach(QString str, data) {
			if (str.contains(filterRegExp()))
				return true;
		}

		return false;
	}
};

//----------------------------------------------------------------------------
// YaRosterTabNameButton
//----------------------------------------------------------------------------

class YaRosterTabNameButton : public QPushButton
{
public:
	YaRosterTabNameButton(QString text, QWidget* parent)
		: QPushButton(text, parent)
	{}

protected:
	// reimplemented
	void paintEvent(QPaintEvent*)
	{
		QRect r(rect());
		r.translate(0, -1);
#ifdef YAPSI_YARU_FRIENDS
		if (isDown()) {
			r.translate(0, 1);
		}
#endif
		QPainter p(this);
		p.setPen(palette().color(QPalette::ButtonText));
		p.drawText(r, Qt::AlignHCenter | Qt::AlignVCenter, text());
	}
};

//----------------------------------------------------------------------------
// YaRosterTabButton
//----------------------------------------------------------------------------

class YaRosterTabButton : public YaToolBoxPageButton
{
	Q_OBJECT
public:
	YaRosterTabButton(YaRosterTab* roster, const QString& text)
		: roster_(roster)
		, hbox(0)
		, text_(text)
	{
		setFrameStyle(NoFrame);

		background_ = QPixmap(":images/rostertab_background2.png");
		backgroundInactive_ = QPixmap(":images/rostertab_background_inactive.png");
		setExpandedModeEnabled(false);
	}

	void setExpandedModeEnabled(bool enabled)
	{
		Q_UNUSED(enabled);
		int height = background_.height();
		int margin = 1;
		// if (enabled) {
		// 	height += margin;
		// }
		if (hbox) {
			hbox->setContentsMargins(0, margin, 0, 2);
		}
		setMinimumHeight(height);
		setMaximumHeight(height);
	}

	virtual void init()
	{
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setContentsMargins(3, 0, 3, 0);
		layout->setSpacing(0);

		hbox = new QHBoxLayout();
		// hbox->setMargin(0);
		hbox->setSpacing(0);

		layout->addStretch();
		layout->addLayout(hbox);

		leftFrame_ = new QFrame(this);
		leftFrame_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		leftFrame_->setFrameShape(QFrame::NoFrame);
		hbox->addWidget(leftFrame_);

		initButtons();
		foreach(QAbstractButton* btn, buttons()) {
			setupButton(btn);
			// hbox->addWidget(btn);
		}

		foreach(QAbstractButton* btn, leftButtons()) {
			hbox->addWidget(btn);
		}

		middleFrame_ = new QFrame(this);
		middleFrame_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		middleFrame_->setFrameShape(QFrame::NoFrame);
		hbox->addWidget(middleFrame_);

		foreach(QAbstractButton* btn, rightButtons()) {
			hbox->addWidget(btn);
		}

		rightFrame_ = new QFrame(this);
		rightFrame_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		rightFrame_->setFrameShape(QFrame::NoFrame);
		rightFrame_->hide();
		hbox->addWidget(rightFrame_);

		okButton_ = new YaOkButton(this);
		okButton_->hide();
		hbox->addWidget(okButton_);
		setupButton(okButton_);

		cancelButton_ = new YaCloseButton(this);
		cancelButton_->hide();
		hbox->addWidget(cancelButton_);
		setupButton(cancelButton_);
	}

	virtual void initButtons()
	{
		// nameButton_ = new YaRosterTabNameButton(text_, this);
		// connect(nameButton_, SIGNAL(clicked()), SIGNAL(toggleCurrentPage()));
	}

	void activate()
	{
		emit setCurrentPage();
	}

	// reimplemented
	virtual void setIsCurrentPage(bool isCurrentPage)
	{
		YaToolBoxPageButton::setIsCurrentPage(isCurrentPage);

		// QPalette palette = nameButton_->palette();
		// palette.setColor(QPalette::ButtonText, isCurrentPage ? QColor(0x1A,0x52,0xAE) : QColor(0x5D,0x91,0xD0));
		// nameButton_->setPalette(palette);
	}

	virtual QList<QAbstractButton*> buttons() const
	{
		QList<QAbstractButton*> result;
		// result << nameButton_;
		return result;
	}

	virtual QList<QAbstractButton*> leftButtons() const
	{
		QList<QAbstractButton*> result;
		return result;
	}

	virtual QList<QAbstractButton*> rightButtons() const
	{
		QList<QAbstractButton*> result;
		return result;
	}

	void setupButton(QAbstractButton* btn)
	{
		PsiToolTip::install(btn);
#ifdef YAPSI_YARU_FRIENDS
		btn->setCursor(Qt::PointingHandCursor);
#else
		// btn != nameButton_ ? btn->setCursor(Qt::PointingHandCursor) : btn->unsetCursor();
#endif
		btn->setFocusPolicy(Qt::NoFocus);

		QSize size = QSize(buttonHeight() - 6, buttonHeight() - 4);
		YaRosterToolButton* yaRosterToolButton = dynamic_cast<YaRosterToolButton*>(btn);
		if (yaRosterToolButton) {
			yaRosterToolButton->setCompactSize(size);
		}
		else {
			btn->setFixedSize(size);
		}
	}

protected:
	// reimplemented
	void paintEvent(QPaintEvent*)
	{
		QPainter p(this);

		// QRect g = ui_.stackedWidget->geometry();
		//Ya::VisualUtil::paintRosterBackground(this, &p);

		p.drawTiledPixmap(rect(), background_);
		p.fillRect(QRect(0, background_.height(), width(), height()), Qt::white);

		// QRegion buttonRegion = Ya::VisualUtil::roundedMask(QSize(width() - 2, height()),
		//                  Ya::VisualUtil::rosterGroupCornerRadius() - 2, Ya::VisualUtil::TopBorders);
		// buttonRegion.translate(1, 0);
		//
		// QPainter p(this);
		// p.fillRect(rect(), Qt::white);
		// p.setClipRegion(buttonRegion);
		//
		// QPixmap pix;
		// if (isCurrentPage())
		// 	pix = background_;
		// else
		// 	pix = backgroundInactive_;
		// p.drawTiledPixmap(rect(), pix);
	}

	YaRosterTab* roster() const { return roster_; }
	int buttonHeight() const { return background_.height() - 5; }

public:
	QFrame* leftFrame_;
	QFrame* middleFrame_;
	QFrame* rightFrame_;
	QToolButton* okButton_;
	QToolButton* cancelButton_;

private:
	YaRosterTab* roster_;
	QHBoxLayout* hbox;
	QString text_;
	// QPushButton* nameButton_;
	QPixmap background_;
	QPixmap backgroundInactive_;
};

//----------------------------------------------------------------------------
// YaRosterContactsTabButton
//----------------------------------------------------------------------------

class YaRosterContactsTabButton : public YaRosterTabButton
{
	Q_OBJECT
public:
	YaRosterContactsTabButton(YaRosterTab* roster, const QString& text)
		: YaRosterTabButton(roster, text)
		, yaRosterTab_(roster)
		, addButton_(0)
		, filterButton_(0)
		, addContactLineEdit_(0)
		, addContactLineEditAccount_(0)
		, filterEdit_(0)
	{}

	// reimplemented
	void initButtons()
	{
		YaRosterTabButton::initButtons();

		{
			filterButton_ = new YaRosterToolButton(this);
			filterButton_->setCheckable(true);
			filterButton_->setIcon(QPixmap(":images/filter.png"));
			connect(filterButton_, SIGNAL(toggled(bool)), roster(), SLOT(setFilterModeEnabled(bool)));
			connect(roster(), SIGNAL(filterModeChanged(bool)), filterButton_, SLOT(setChecked(bool)));

			QList<QKeySequence> keys = ShortcutManager::instance()->shortcuts("appwide.filter-contacts");
			if (keys.isEmpty())
				filterButton_->setToolTip(tr("Search all contacts"));
			else
				filterButton_->setToolTip(tr("Search all contacts (%1)")
				                          .arg(keys.first().toString(QKeySequence::NativeText)));
		}

		{
			addButton_ = new YaRosterAddContactToolButton(this, yaRosterTab_->yaRoster());
			addButton_->setText(tr("Add a contact"));
			addButton_->setToolTip(tr("Add a contact"));
			addButton_->setCheckable(true);
			// icon is now set in YaRosterContactsTab::setAddContactModeEnabled(bool enabled)
			connect(addButton_, SIGNAL(clicked()), roster(), SLOT(addButtonClicked()));
		}
	}

	// reimplemented
	void init()
	{
		YaRosterTabButton::init();

		{
			int margin = 3;

			addContactLineEdit_ = new YaExpandingOverlayLineEdit(this);
			addContactLineEdit_->setController(addButton_);
			// addContactLineEdit_->setEmptyText("user@ya.ru");
			addContactLineEdit_->setOkButtonVisible(false);
			addContactLineEdit_->setCancelButtonVisible(false);

			addContactLineEditAccount_ = new ServicesPopupButton(addContactLineEdit_);

			filterEdit_ = new YaExpandingOverlayLineEdit(this);
			filterEdit_->setController(filterButton_);
			filterEdit_->setOkButtonVisible(false);
			filterEdit_->setCancelButtonVisible(false);

			QList<YaExpandingOverlayLineEdit*> lineEdits;
			lineEdits << addContactLineEdit_ << filterEdit_;
			foreach(YaExpandingOverlayLineEdit* lineEdit, lineEdits) {
				lineEdit->setMaximumHeight(15 + 4);
				// lineEdit->setFrame(false);
				lineEdit->setProperty("margin-left-right", QVariant(margin));
				// foreach(QAbstractButton* btn, buttons()) {
				// 	if (lineEdit->controller() != btn)
				// 		lineEdit->addToGrounding(btn);
				// }
#ifndef YAPSI_NO_STYLESHEETS
				lineEdit->setStyleSheet(
					"QLineEdit {"
					"	color: black;"
					"	border-image: url(:/images/roster_lineedit.png) 2px 2px 2px 2px;"
					"	border-width: 2px 2px 2px 2px;"
					"}"
				);
#endif
			}

			addContactLineEdit_->addToGrounding(rightFrame_);

			filterEdit_->addToGrounding(rightFrame_);
		}
	}

	YaExpandingOverlayLineEdit* addContactLineEdit() const { return addContactLineEdit_; }
	ServicesPopupButton* addContactLineEditAccount() const { return addContactLineEditAccount_; }
	YaExpandingOverlayLineEdit* filterEdit() const { return filterEdit_; }
	QToolButton* addButton() const { return addButton_; }
	QToolButton* filterButton() const { return filterButton_; }

	// reimplemented
	QList<QAbstractButton*> buttons() const
	{
		QList<QAbstractButton*> result;
		result << addButton_;
		result << YaRosterTabButton::buttons();
		result << filterButton_;
		return result;
	}

	// reimplemented
	virtual QList<QAbstractButton*> leftButtons() const
	{
		QList<QAbstractButton*> result;
		result << addButton_;
		result << YaRosterTabButton::leftButtons();
		return result;
	}

	// reimplemented
	virtual QList<QAbstractButton*> rightButtons() const
	{
		QList<QAbstractButton*> result;
		result << filterButton_;
		result << YaRosterTabButton::rightButtons();
		return result;
	}


	// reimplemented
	virtual void setIsCurrentPage(bool isCurrentPage)
	{
		YaRosterTabButton::setIsCurrentPage(isCurrentPage);
		YaRosterContactsTab* r = dynamic_cast<YaRosterContactsTab*>(roster());
		Q_ASSERT(r);
		if (!isCurrentPage) {
			r->setFilterModeEnabled(false);
			r->setAddContactModeEnabled(false);
		}
	}

private:
	YaRosterTab* yaRosterTab_;
	QToolButton* addButton_;
	QToolButton* filterButton_;
	YaExpandingOverlayLineEdit* addContactLineEdit_;
	ServicesPopupButton* addContactLineEditAccount_;
	YaExpandingOverlayLineEdit* filterEdit_;
};

//----------------------------------------------------------------------------
// AddContactListView
//----------------------------------------------------------------------------

class AddContactListView : public YaContactListView
{
	Q_OBJECT
public:
	AddContactListView(QWidget* parent)
		: YaContactListView(parent)
	{}

	~AddContactListView()
	{}

	void setError(const QString& error)
	{
		errorText_ = error;
		viewport()->update();
	}

	void setStatusText(const QString& statusText)
	{
		statusText_ = statusText;
		viewport()->update();
	}

protected:
	// reimplemented
	virtual void paintNoContactsStub(QPainter* p)
	{
		QString text = tr("In order to add a contact, enter a username to the above"
		                  " field (e.g. user@jabbin.com) and press \"Enter\". After"
		                  " doing so the request to authorize him to your contact list"
		                  " would be sent. After authorization confirmation you would"
		                  " see his status and would be able to exchange messages.");
		int margin = 5;
		QRect rect(viewport()->rect().adjusted(margin, margin, -margin, -margin));

		int flags = Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap;

		if (!errorText_.isEmpty()) {
			QString error = tr("Error: %1").arg(errorText_);
			p->setPen(QColor(0xF0, 0x00, 0x00));
			p->drawText(rect, flags, error);

			QRect boundingRect = fontMetrics().boundingRect(rect, flags, error);
			rect.setTop(boundingRect.bottom() + margin);
		}

		// p->setPen(palette().color(QPalette::Disabled, QPalette::Text));
		p->setPen(Qt::black);

		if (!statusText_.isEmpty()) {
			p->drawText(rect, flags, statusText_);

			QRect boundingRect = fontMetrics().boundingRect(rect, flags, statusText_);
			rect.setTop(boundingRect.bottom() + margin);
		}

		p->drawText(rect, flags, text);
	}

private:
	QString errorText_;
	QString statusText_;
};

//----------------------------------------------------------------------------
// YaRosterContactsTab
//----------------------------------------------------------------------------

YaRosterContactsTab::YaRosterContactsTab(const QString& name, bool createToolButtons, YaRoster* parent)
	: YaRosterTab(name, createToolButtons, parent)
	, filterModeEnabled_(false)
	, addContactListView_(0)
	, filteredListView_(0)
	, proxyModel_(0)
	, watchTimer_(0)
{
	Q_ASSERT(createToolButtons == true);

	watchTimer_ = new QTimer(this);
	watchTimer_->setSingleShot(false);
	watchTimer_->setInterval(250);
	connect(watchTimer_, SIGNAL(timeout()), SLOT(watchForJid()));

	proxyModel_ = new YaRosterFilterProxyModel(this);

	connect(YaRosterToolTip::instance(), SIGNAL(addContact(const XMPP::Jid&)), SLOT(addContact(const XMPP::Jid&)));
}

void YaRosterContactsTab::init()
{
	YaRosterTab::init();

	contactListView()->installEventFilter(this);

	{
		// addContactListView_ = new AddContactListView(0);
		// addContactListView_->setFrameStyle(QFrame::NoFrame);
		addContactListView_ = new YaAddContactPage(0);
		addContactListView_->init();
		contactsPageButton()->addContactLineEdit()->installEventFilter(this);
		contactsPageButton()->addContactLineEditAccount()->installEventFilter(this);
		connect(contactsPageButton()->addContactLineEdit(), SIGNAL(enteredText(const QString&)), SLOT(addContactTextEntered(const QString&)));
		connect(contactsPageButton()->addContactLineEdit(), SIGNAL(cancelled()), SLOT(addContactCancelled()));

		contactsPageButton()->filterEdit()->installEventFilter(this);
		connect(contactsPageButton()->filterEdit(), SIGNAL(textChanged(const QString&)), SLOT(filterEditTextChanged(const QString&)));

		filteredListView_ = new YaFilteredContactListView(0);
		connect(filteredListView_, SIGNAL(quitFilteringMode()), SLOT(quitFilteringMode()));
		filteredListView_->installEventFilter(this);

		stackedWidget()->addWidget(filteredListView_);
		stackedWidget()->addWidget(addContactListView_);
	}

	connect(contactsPageButton()->okButton_, SIGNAL(clicked()), SLOT(okButtonClicked()));
	connect(contactsPageButton()->cancelButton_, SIGNAL(clicked()), SLOT(cancelButtonClicked()));

	setAddContactModeEnabled(false);
	setFilterModeEnabled(false);
}

void YaRosterContactsTab::okButtonClicked()
{
	Q_ASSERT(stackedWidget()->currentWidget() == addContactListView_);

	addContactTextEntered(contactsPageButton()->addContactLineEdit()->text());
}

void YaRosterContactsTab::cancelButtonClicked()
{
	setAddContactModeEnabled(false);
	setFilterModeEnabled(false);
}

YaFilteredContactListView* YaRosterContactsTab::filteredListView() const
{
	return filteredListView_;
}

YaRosterContactsTabButton* YaRosterContactsTab::contactsPageButton() const
{
	return static_cast<YaRosterContactsTabButton*>(YaRosterTab::pageButton());
}

YaAddContactPage* YaRosterContactsTab::addContactListView() const
{
	return addContactListView_;
}

void YaRosterContactsTab::watchForJid()
{
	if (watchedAccount_.isNull()) {
		setAddContactModeEnabled(false);
		return;
	}
	PsiContact* contact = watchedAccount_->findContact(watchedJid_);
	if (contact) {
		if (!contact->isOnline()) {
			contact->showOnlineTemporarily();
		}
		else {
			YaContactListModel* model = dynamic_cast<YaContactListModel*>(contactListView()->realModel());
			QModelIndexList indexes = dynamic_cast<ContactListModel*>(model)->indexesFor(contact);
			if (!indexes.isEmpty()) {
				QMimeData* selection = model->mimeData(indexes);
				contactListView()->restoreSelection(selection);
				delete selection;

				watchTimer_->stop();
				setAddContactModeEnabled(false);
			}
		}
	}
}

void YaRosterContactsTab::addContact(const XMPP::Jid& jid)
{
	dynamic_cast<YaRosterTabButton*>(pageButton())->activate();
	setAddContactModeEnabled(true);
	bringToFront(contactListView());
	contactsPageButton()->addContactLineEdit()->setText(jid.full());
	addContactTextEntered(jid.full());
}

void YaRosterContactsTab::addButtonClicked()
{
	contactsPageButton()->addButton()->setChecked(false);

	if (stackedWidget()->currentWidget() == addContactListView_) {
		// if (!contactsPageButton()->addContactLineEdit()->text().isEmpty()) {
		// 	addContactTextEntered(contactsPageButton()->addContactLineEdit()->text());
		// }
		// else {
			setAddContactModeEnabled(false);
		// }
	}
	else {
		setAddContactModeEnabled(true);
		contactsPageButton()->addButton()->setChecked(true);
	}
}

void YaRosterContactsTab::setAddContactModeEnabled(bool _enabled)
{
	bool enabled = _enabled && yaRoster()->haveAvailableAccounts();

	watchTimer_->stop();
	addContactListView()->setError(QString());
	addContactListView()->setStatusText(QString());
	contactsPageButton()->addButton()->setChecked(enabled);
	contactsPageButton()->addButton()->setEnabled(!enabled);
	// contactsPageButton()->addButton()->setIcon(enabled ?
	//         QPixmap(":images/addcontactok.png") :
	//         QPixmap(":images/addcontact.png"));
	contactsPageButton()->addButton()->setIcon(QPixmap(":images/addcontact.png"));

	contactsPageButton()->addContactLineEdit()->setVisible(enabled);
	if (enabled) {
		setFilterModeEnabled(false);

		contactsPageButton()->leftFrame_->hide();
		contactsPageButton()->middleFrame_->hide();
		contactsPageButton()->rightFrame_->show();
		contactsPageButton()->okButton_->show();
		contactsPageButton()->cancelButton_->hide();
		contactsPageButton()->filterButton()->hide();
		contactsPageButton()->setExpandedModeEnabled(true);

		stackedWidget()->setCurrentWidget(addContactListView_);
		addContactListView_->setUpdatesEnabled(true);
		contactsPageButton()->addContactLineEdit()->clear();
		contactsPageButton()->addContactLineEdit()->setFocus();
		contactsPageButton()->activate();

#ifndef CONTACTLIST_UNSELECT_ON_CLICK_OUTSIDE
		qApp->installEventFilter(this);
#endif
	}
	else {
		contactsPageButton()->leftFrame_->show();
		contactsPageButton()->middleFrame_->show();
		contactsPageButton()->rightFrame_->hide();
		contactsPageButton()->okButton_->hide();
		contactsPageButton()->cancelButton_->hide();
		contactsPageButton()->filterButton()->show();
		contactsPageButton()->setExpandedModeEnabled(false);

		stackedWidget()->setCurrentWidget(contactListViewStackedWidget());
		contactListView()->setFocus();

#ifndef CONTACTLIST_UNSELECT_ON_CLICK_OUTSIDE
		qApp->removeEventFilter(this);
#endif
	}
}

void YaRosterContactsTab::addContactTextEntered(const QString& text)
{
	QString c = text.trimmed();
	QString transport = contactsPageButton()->addContactLineEditAccount()->transport();
	qDebug() << "Transport is " << transport;
	if (!transport.isEmpty()) {
		c = c.replace("@", "\\40");
		c.append("@" + transport);
		qDebug() << "jid is " << c;
	}


	if (c.isEmpty()) {
		setAddContactModeEnabled(false);
		return;
	}

	QRegExp rx("[.@/]");
	if (rx.indexIn(c) == -1) {
		c = c + "@jabbin.com";
	}

	c = Ya::yaRuAliasing(c);

	PsiContactList* contactList = dynamic_cast<ContactListModel*>(contactListView()->realModel())->contactList();

	Jid jid(c);
	jid = jid.bare();

	QRegExp invalidJidRx(".*@.*@.*|\\s");
	if (invalidJidRx.indexIn(c) != -1 || jid.node().isEmpty() || jid.host().isEmpty()) {
		addContactListView()->setError(tr("Invalid Jabber ID: %1")
		                               .arg(c));
		return;
	}

	PsiAccount* acc = 0;
	foreach(PsiAccount* account, contactList->sortedEnabledAccounts()) {
		if (!account->isAvailable())
			continue;
//		if (jid.host() != "yandex-team.ru")
//			continue;
		if (account->jid().host() == jid.host()) {
			acc = account;
			break;
		}
	}

	if (!acc) {
		foreach(PsiAccount* account, contactList->sortedEnabledAccounts()) {
			if (account->isAvailable()) {
				acc = account;
				break;
			}
		}
	}

	if (acc) {
		bool unblocked = false;
		addContactListView()->setError(QString());
		YaPrivacyManager* privacyManager = dynamic_cast<YaPrivacyManager*>(acc->privacyManager());
		if (privacyManager && privacyManager->isContactBlocked(jid)) {
			privacyManager->setContactBlocked(jid, false);
			unblocked = true;
		}

		if (acc->findContact(jid.full()) && acc->findContact(jid.full())->inList()) {
			if (unblocked) {
				addContactListView()->setStatusText(tr("Unblocked %1").arg(jid.full()));
				setAddContactModeEnabled(false);
				return;
			}

			addContactListView()->setError(tr("Tried to add existing contact %1 to account %2")
			                               .arg(jid.full()).arg(acc->name()));
			return;
		}

		if (acc->jid().compare(jid, false)) {
			addContactListView()->setError(tr("Tried to add self to the roster: %1").arg(acc->jid().full()));
			return;
		}

		QString name;
		{
			// vcardFinished() relies on jid.bare() as default non-initialized name
			PsiContact* contact = acc->findContact(jid.bare());
			name = contact ? contact->name() : jid.bare();
		}

		acc->dj_add(jid.full(), name, QStringList(), true);
		acc->dj_authInternal(jid.full());
		qWarning("Added %s to the %s account.", qPrintable(c), qPrintable(acc->jid().full()));

		VCardFactory::instance()->getVCard(jid.full(), acc->client()->rootTask(), this, SLOT(vcardFinished()));
		addContactListView()->setStatusText(tr("Adding a contact..."));

		watchedAccount_ = acc;
		watchedJid_ = jid;
		watchTimer_->start();
	}
	else {
		addContactListView()->setError(tr("No suitable on-line accounts found to add %1.").arg(jid.full()));
		return;
	}
}

void YaRosterContactsTab::vcardFinished()
{
	if (watchedAccount_.isNull()) {
		return;
	}

	XMPP::JT_VCard* j = static_cast<XMPP::JT_VCard*>(sender());
	QString nick = j->jid().user();
	if (j->success() && j->statusCode() == Task::ErrDisc) {
		if (!j->vcard().nickName().isEmpty()) {
			nick = j->vcard().nickName();
		}
		else if (!j->vcard().fullName().isEmpty()) {
			nick = j->vcard().fullName();
		}
	}

	PsiContact* contact = watchedAccount_->findContact(j->jid());
	if (contact) {
		if (contact->name() == j->jid().bare()) {
			contact->setName(nick);
		}
	}
}

void YaRosterContactsTab::filterEditTextChanged(const QString& text)
{
	updateFilterMode();
	proxyModel_->setFilterRegExp(QString("^%1|[@]%1|\\s%1").arg(QRegExp::escape(text)));
}

void YaRosterContactsTab::quitFilteringMode()
{
	setFilterModeEnabled(false);
}

void YaRosterContactsTab::addContactCancelled()
{
	setAddContactModeEnabled(false);
}

bool YaRosterContactsTab::filterModeEnabled() const
{
	// return stackedWidget()->currentWidget() == filteredListView_;
	return filterModeEnabled_;
}

void YaRosterContactsTab::updateFilterMode()
{
	Q_ASSERT(filterModeEnabled());
	if (!contactsPageButton()->filterEdit()->text().isEmpty()) {
		stackedWidget()->setCurrentWidget(filteredListView_);
	}
	else {
		stackedWidget()->setCurrentWidget(contactListViewStackedWidget());
	}
}

void YaRosterContactsTab::setFilterModeEnabled(bool enabled)
{
	if (!enabled && !filterModeEnabled()) {
		return;
	}

	YaContactListView* selectionSource = 0;
	YaContactListView* selectionDestination = 0;

	contactsPageButton()->filterButton()->setEnabled(!enabled);

	contactsPageButton()->filterEdit()->setVisible(enabled);
	filterModeEnabled_ = enabled;
	if (enabled) {
		selectionSource = contactListView();
		selectionDestination = filteredListView_;

		setAddContactModeEnabled(false);

		contactsPageButton()->leftFrame_->hide();
		contactsPageButton()->middleFrame_->hide();
		contactsPageButton()->rightFrame_->show();
		contactsPageButton()->okButton_->hide();
		contactsPageButton()->cancelButton_->show();
		contactsPageButton()->addButton()->hide();
		contactsPageButton()->setExpandedModeEnabled(true);

		contactsPageButton()->filterEdit()->selectAll();
		contactsPageButton()->filterEdit()->setFocus();
		contactsPageButton()->activate();
		updateFilterMode();

#ifndef CONTACTLIST_UNSELECT_ON_CLICK_OUTSIDE
		qApp->installEventFilter(this);
#endif
	}
	else {
		contactsPageButton()->leftFrame_->show();
		contactsPageButton()->middleFrame_->show();
		contactsPageButton()->rightFrame_->hide();
		contactsPageButton()->okButton_->hide();
		contactsPageButton()->cancelButton_->hide();
		contactsPageButton()->addButton()->show();
		contactsPageButton()->setExpandedModeEnabled(false);

		selectionSource = filteredListView_;
		selectionDestination = contactListView();

		stackedWidget()->setCurrentWidget(contactListViewStackedWidget());
		contactListView()->setFocus();

#ifndef CONTACTLIST_UNSELECT_ON_CLICK_OUTSIDE
		qApp->removeEventFilter(this);
#endif
	}

	QMimeData* selection = selectionSource->selection();
	selectionDestination->restoreSelection(selection);
	delete selection;

	emit filterModeChanged(enabled);
}

bool YaRosterContactsTab::eventFilter(QObject* obj, QEvent* e)
{
	if (e->type() == QEvent::DeferredDelete || e->type() == QEvent::Destroy) {
		return false;
	}

	if (
		e->type() == QEvent::Resize &&
		(obj == contactsPageButton()->addContactLineEdit() ||
		obj == contactsPageButton()->addContactLineEditAccount())
	) {
		qDebug() << "positioning the service chooser for add contact";
		int width = contactsPageButton()->addContactLineEditAccount()->width();
		contactsPageButton()->addContactLineEditAccount()->move(
			contactsPageButton()->addContactLineEdit()->width() - width, 0);
	}
	// if (obj == contactsPageButton()->addContactLineEdit() && e->type() == QEvent::Resize) {
	// 	contactsPageButton()->addContactLineEditAccount()->move(
	// 		contactsPageButton()->addContactLineEdit()->width()
	// 		- contactsPageButton()->addContactLineEditAccount()->width(),
	// 		0
	// 		);
	// 	return false;
	// }

#ifndef CONTACTLIST_UNSELECT_ON_CLICK_OUTSIDE
	// needs testing
	Q_ASSERT(false);
#endif
	if (e->type() == QEvent::WindowDeactivate) {
		setFilterModeEnabled(false);
		setAddContactModeEnabled(false);
	}

	if (!contactListView() || !contactListView()->isActiveWindow())
		return false;

	if (e->type() == QEvent::KeyPress) {
		QKeyEvent* ke = static_cast<QKeyEvent*>(e);
		QString text = ke->text().trimmed();
		if (!text.isEmpty() && (obj == contactListView())) {
			bool correctChar = (text[0].isLetterOrNumber() || text[0].isPunct()) &&
			                   (ke->modifiers() == Qt::NoModifier);
			if (correctChar && !contactListView()->textInputInProgress()) {
				setFilterModeEnabled(true);
				contactsPageButton()->filterEdit()->setText(text);
				return true;
			}
		}

		if (obj == contactsPageButton()->filterEdit() || obj == filteredListView_) {
			if (ke->key() == Qt::Key_Escape) {
				setFilterModeEnabled(false);
				return true;
			}

			if (ke->key() == Qt::Key_Backspace && contactsPageButton()->filterEdit()->text().isEmpty()) {
				setFilterModeEnabled(false);
				return true;
			}

			if (filteredListView_->handleKeyPressEvent(ke)) {
				return true;
			}
		}

		if (obj == contactsPageButton()->addContactLineEdit()) {
			if (ke->key() == Qt::Key_Escape) {
				if (!contactsPageButton()->addContactLineEdit()->text().isEmpty()) {
					contactsPageButton()->addContactLineEdit()->setText(QString());
					return true;
				}
			}
		}
	}

	if (stackedWidget() && e->type() == QEvent::MouseButtonPress) {
		QMouseEvent* me = static_cast<QMouseEvent*>(e);

#ifdef CONTACTLIST_UNSELECT_ON_CLICK_OUTSIDE
		if (stackedWidget()->currentWidget() == contactListView() && !filterModeEnabled()) {
			QList<QWidget*> closeWidgets;
			closeWidgets << filteredListView_->window()->findChildren<QWidget*>();
			closeWidgets.removeAll(contactsPageButton()->addContactLineEdit());
			closeWidgets.removeAll(contactsPageButton()->addButton());
			closeWidgets.removeAll(contactsPageButton()->filterEdit());
			closeWidgets.removeAll(contactsPageButton()->filterButton());
			foreach(QWidget* w, contactListView()->findChildren<QWidget*>()) {
				closeWidgets.removeAll(w);
			}
			foreach(QWidget* w, contactsPageButton()->filterEdit()->findChildren<QWidget*>()) {
				closeWidgets.removeAll(w);
			}
			closeWidgets.removeAll(contactsPageButton()->okButton_);
			closeWidgets.removeAll(contactsPageButton()->cancelButton_);

			if (closeWidgets.contains(dynamic_cast<QWidget*>(obj))) {
				contactListView()->clearSelection();
				return true;
			}
		}
#endif

		if (stackedWidget()->currentWidget() == addContactListView_) {
			QLabel* label = dynamic_cast<QLabel*>(obj);
			if (label && label->cursor().shape() == Qt::PointingHandCursor) {
				return false;
			}

			QList<QWidget*> closeWidgets;
			closeWidgets << filteredListView_->window()->findChildren<QWidget*>();
			closeWidgets.removeAll(contactsPageButton()->addContactLineEdit());
			closeWidgets.removeAll(contactsPageButton()->addButton());
			foreach(QWidget* w, contactsPageButton()->addContactLineEdit()->findChildren<QWidget*>()) {
				closeWidgets.removeAll(w);
			}
			foreach(QWidget* w, filteredListView_->window()->findChildren<QSizeGrip*>()) {
				closeWidgets.removeAll(w);
			}
			closeWidgets.removeAll(contactsPageButton()->okButton_);
			closeWidgets.removeAll(contactsPageButton()->cancelButton_);

			if (closeWidgets.contains(dynamic_cast<QWidget*>(obj))) {
				setAddContactModeEnabled(false);
				return true;
			}
		}

		if (stackedWidget()->currentWidget() == filteredListView_ || filterModeEnabled()) {
			if (stackedWidget()->currentWidget() == filteredListView_ &&
			    (obj == filteredListView_ || obj == filteredListView_->viewport()))
			{
				QModelIndex index = filteredListView_->indexAt(me->pos());
				if (!index.isValid()) {
					setFilterModeEnabled(false);
					return true;
				}
			}

			QList<QWidget*> closeWidgets;
			closeWidgets << filteredListView_->window()->findChildren<QWidget*>();
			closeWidgets.removeAll(contactsPageButton()->filterEdit());
			closeWidgets.removeAll(contactsPageButton()->filterButton());
			closeWidgets.removeAll(filteredListView_);
			foreach(QWidget* w, filteredListView_->findChildren<QWidget*>()) {
				closeWidgets.removeAll(w);
			}
			foreach(QWidget* w, filteredListView_->window()->findChildren<QSizeGrip*>()) {
				closeWidgets.removeAll(w);
			}
			foreach(QWidget* w, contactsPageButton()->filterEdit()->findChildren<QWidget*>()) {
				closeWidgets.removeAll(w);
			}
			closeWidgets.removeAll(contactsPageButton()->okButton_);
			closeWidgets.removeAll(contactsPageButton()->cancelButton_);

			if (closeWidgets.contains(dynamic_cast<QWidget*>(obj))) {
				setFilterModeEnabled(false);
				return true;
			}
		}
	}

	return QObject::eventFilter(obj, e);
}

void YaRosterContactsTab::setModel(QAbstractItemModel* model)
{
	YaRosterTab::setModel(model);
}

void YaRosterContactsTab::setModelForFilter(QAbstractItemModel* model)
{
	Q_ASSERT(proxyModel_);
	if (proxyModel_) {
		QAbstractItemModel* realModel = model;
		QSortFilterProxyModel* proxy = dynamic_cast<QSortFilterProxyModel*>(model);
		if (proxy)
			realModel = proxy->sourceModel();

		QAbstractItemModel* oldModel = proxyModel_->sourceModel();
		ContactListModel* contactListModel = dynamic_cast<ContactListModel*>(realModel);
		Q_ASSERT(contactListModel);

		ContactListModel* clone = contactListModel->clone();
		clone->invalidateLayout();

		proxyModel_->setSourceModel(clone);
		filteredListView_->setModel(proxyModel_);
		if (oldModel)
			delete oldModel;
	}
}

void YaRosterContactsTab::setAvatarMode(YaContactListView::AvatarMode avatarMode)
{
	YaRosterTab::setAvatarMode(avatarMode);
	filteredListView_->setAvatarMode(avatarMode);
}

QList<QWidget*> YaRosterContactsTab::proxyableWidgets() const
{
	QList<QWidget*> result = YaRosterTab::proxyableWidgets();
	result += contactsPageButton()->addButton();
	result += contactsPageButton()->filterButton();
	return result;
}

//----------------------------------------------------------------------------
// YaRosterTab
//----------------------------------------------------------------------------

YaRosterTab::YaRosterTab(const QString& name, bool createToolButtons, YaRoster* parent)
	: QObject(parent)
	, contactListView_(0)
{
	stackedWidget_ = new QStackedWidget(0);
	contactListViewStackedWidget_ = new QStackedWidget(0);

	noContactsPage_ = new YaNoContactsPage(0);
	noContactsPage_->init();
	connect(noContactsPage_, SIGNAL(addContact()), SIGNAL(addContact()));

	if (createToolButtons)
		pageButton_ = new YaRosterContactsTabButton(this, name);
	else
		pageButton_ = new YaRosterTabButton(this, name);
}

YaRosterTab::~YaRosterTab()
{
	if (!stackedWidget_.isNull())
		delete stackedWidget_;
	if (!contactListViewStackedWidget_.isNull())
		delete contactListViewStackedWidget_;
	if (!noContactsPage_.isNull())
		delete noContactsPage_;
	if (!pageButton_.isNull())
		delete pageButton_;
}

void YaRosterTab::init()
{
	pageButton_->init();

	contactListView_ = new YaContactListView(0);
	contactListView_->setFrameStyle(QFrame::NoFrame);
	connect(YaRosterToolTip::instance(), SIGNAL(toolTipEntered(PsiContact*, QMimeData*)), contactListView_, SLOT(toolTipEntered(PsiContact*, QMimeData*)));
	connect(YaRosterToolTip::instance(), SIGNAL(toolTipHidden(PsiContact*, QMimeData*)), contactListView_, SLOT(toolTipHidden(PsiContact*, QMimeData*)));

	stackedWidget_->addWidget(contactListViewStackedWidget_);
	stackedWidget_->setCurrentWidget(contactListViewStackedWidget_);

	contactListViewStackedWidget_->addWidget(contactListView_);
	contactListViewStackedWidget_->addWidget(noContactsPage_);
	contactListViewStackedWidget_->setCurrentWidget(contactListView());
}

YaContactListView* YaRosterTab::contactListView() const
{
	return contactListView_;
}

QStackedWidget* YaRosterTab::contactListViewStackedWidget() const
{
	return contactListViewStackedWidget_;
}

QStackedWidget* YaRosterTab::stackedWidget() const
{
	return stackedWidget_;
}

YaRoster* YaRosterTab::yaRoster() const
{
	return static_cast<YaRoster*>(parent());
}

YaToolBoxPageButton* YaRosterTab::pageButton() const
{
	return pageButton_;
}

QWidget* YaRosterTab::widget() const
{
	return stackedWidget_;
}

void YaRosterTab::setModel(QAbstractItemModel* model)
{
	Q_ASSERT(model);
	contactListView_->setModel(model);

	connect(model, SIGNAL(rowsInserted(const QModelIndex&, int, int)), SLOT(updateContactListViewStackedWidget()));
	connect(model, SIGNAL(rowsRemoved(const QModelIndex&, int, int)), SLOT(updateContactListViewStackedWidget()));
	connect(model, SIGNAL(modelReset()), SLOT(updateContactListViewStackedWidget()));
	connect(model, SIGNAL(layoutChanged()), SLOT(updateContactListViewStackedWidget()));
	updateContactListViewStackedWidget();
}

void YaRosterTab::updateContactListViewStackedWidget()
{
	YaContactListModel* model = contactListView_ ? dynamic_cast<YaContactListModel*>(contactListView()->realModel()) : 0;

	if (model &&
	    model->rowCount(QModelIndex()) == 0 &&
	    model->contactList() &&
	    model->contactList()->accountsLoaded() &&
	    !model->contactList()->haveConnectingAccounts())
	{
		contactListViewStackedWidget_->setCurrentWidget(noContactsPage_);
	}
	else {
		contactListViewStackedWidget_->setCurrentWidget(contactListView());
	}
}

void YaRosterTab::setModelForFilter(QAbstractItemModel* model)
{
	Q_UNUSED(model);
}

void YaRosterTab::setAvatarMode(YaContactListView::AvatarMode avatarMode)
{
	contactListView_->setAvatarMode(avatarMode);
}

QList<QWidget*> YaRosterTab::proxyableWidgets() const
{
	QList<QWidget*> result;
	return result;
}

//----------------------------------------------------------------------------
// YaRoster
//----------------------------------------------------------------------------

YaRoster::YaRoster(QWidget* parent)
	: QFrame(parent)
	, avatarMode_(YaContactListView::AvatarMode_Auto)
	, stackedWidget_(0)
	, rosterPage_(0)
	, loginPage_(0)
	, contacts_(0)
	, friends_(0)
	, filterModel_(0)
	, contactsModel_(0)
	, informersModel_(0)
	, friendListModel_(0)
	, contactListModel_(0)
	, haveAvailableAccounts_(false)
{
	setFrameShape(NoFrame);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(0);

	stackedWidget_ = new AnimatedStackedWidget(this);
	stackedWidget_->init();
	stackedWidget_->setAnimationStyle(AnimatedStackedWidget::Animation_Push_Horizontal);
	stackedWidget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	layout->addWidget(stackedWidget_);

	rosterPage_ = new YaToolBox(0);
	rosterPage_->init();
	// rosterPage_->layout()->setContentsMargins(0, 1, 0, 0);
	rosterPage_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	rosterPage_->installEventFilter(this);
	stackedWidget_->addWidget(rosterPage_);

	loginPage_ = new YaLoginPage();
	loginPage_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	stackedWidget_->addWidget(loginPage_);
	connect(loginPage_, SIGNAL(updateVisibility()), SLOT(updateCurrentPage()));

	stackedWidget_->setWidgetPriority(loginPage_, 0);
	stackedWidget_->setWidgetPriority(rosterPage_, 1);

	updateCurrentPage();

	connect(PsiOptions::instance(), SIGNAL(optionChanged(const QString&)), SLOT(optionChanged(const QString&)));
}

YaRoster::~YaRoster()
{
#ifdef CONTACTLIST_UNSELECT_ON_CLICK_OUTSIDE
	qApp->removeEventFilter(contacts_);
#endif

	PsiOptions::instance()->setOption(tabIndexOptionPath, rosterPage_->currentIndex());
}

bool YaRoster::eventFilter(QObject* obj, QEvent* e)
{
	if (obj == rosterPage_ && e->type() == QEvent::Paint) {
		QPainter p(rosterPage_);
		p.fillRect(QRect(0, 0, rosterPage_->width(), 1), Qt::white);
		return true;
	}

	return QFrame::eventFilter(obj, e);
}

bool YaRoster::selfWidgetsShouldBeVisible()
{
	return !loginPage_->shouldBeVisible();
}

void YaRoster::updateCurrentPage()
{
	if (loginPage_->shouldBeVisible())
		stackedWidget_->setCurrentWidget(loginPage_);
	else
		stackedWidget_->setCurrentWidget(rosterPage_);

	// TODO: could lead to strange crashes on Mac OS X when quitting application
	emit updateSelfWidgetsVisibility();
}

QList<YaRosterTab*> YaRoster::rosterTabs() const
{
	QList<YaRosterTab*> result;
#ifdef YAPSI_YARU_FRIENDS
	result << friends_;
#endif
	result << contacts_;
	return result;
}

void YaRoster::setContactList(PsiContactList* contactList)
{
	Q_ASSERT(!contactListModel_);
	contactList_ = contactList;

	loginPage_->setContactList(contactList);

	contacts_ = new YaRosterContactsTab(tr("Contacts"), true, this);
#ifdef YAPSI_YARU_FRIENDS
	friends_  = new YaRosterTab(tr("Friends from jabbin.com"), false, this);
#endif

#ifdef CONTACTLIST_UNSELECT_ON_CLICK_OUTSIDE
	qApp->installEventFilter(contacts_);
#endif

	foreach(YaRosterTab* tab, rosterTabs()) {
		tab->init();
	}

	filterModel_ = new YaContactListModel(contactList);
	filterModel_->invalidateLayout();
	filterModel_->setGroupsEnabled(false);

	// FIXME: see comment in PsiContact::groupOperationPermitted()
	// contactsModel_ = new YaContactListModel(contactList);
	// contactsModel_->invalidateLayout();
	// contactsModel_->setGroupsEnabled(true);

	contactListModel_ = new YaContactListContactsModel(contactList);
	contactListModel_->invalidateLayout();
	contactListModel_->setGroupsEnabled(true);
	contactListModel_->storeGroupState("contacts");
#ifdef MODELTEST
	new ModelTest(contactListModel_, this);
#endif
	ContactListProxyModel* contactListProxyModel = new ContactListProxyModel(this, contactList);
	contactListProxyModel->setSourceModel(contactListModel_);
#ifdef MODELTEST
	new ModelTest(contactListProxyModel, this);
#endif
	// contacts_->setModel(contactListModel_);
	contacts_->setModel(contactListProxyModel);
	contacts_->setModelForFilter(filterModel_);

	contacts_->contactListView()->setObjectName("contacts");

#ifdef YAPSI_YARU_FRIENDS
	friendListModel_ = new YaContactListFriendsModel(contactList);
	friendListModel_->invalidateLayout();
	friendListModel_->setGroupsEnabled(false);
#ifdef MODELTEST
	new ModelTest(friendListModel_, this);
#endif
	ContactListProxyModel* friendListProxyModel = new ContactListProxyModel(this, contactList);
	friendListProxyModel->setSourceModel(friendListModel_);
#ifdef MODELTEST
	new ModelTest(friendListProxyModel, this);
#endif
	friends_->setModel(friendListProxyModel);
	// friends_->setModel(friendListModel_);
#endif

	QList<YaContactListView*> contactListViews;
	foreach(YaRosterTab* tab, rosterTabs()) {
		rosterPage_->addPage(tab->pageButton(), tab->widget());
		contactListViews << tab->contactListView();
		connect(tab, SIGNAL(addContact()), SLOT(addContact()));
	}

	contactListViews << contacts_->filteredListView();

	foreach(YaContactListView* contactListView, contactListViews) {
		connect(contactListView, SIGNAL(removeSelection(QMimeData*)), SLOT(removeSelection(QMimeData*)));
		connect(contactListView, SIGNAL(addSelection(QMimeData*)), SLOT(addSelection(QMimeData*)));
		connect(contactListView, SIGNAL(addGroup()), SLOT(addGroup()));
	}

	connect(YaRosterToolTip::instance(), SIGNAL(removeContact(PsiContact*, QMimeData*)), SLOT(removeContact(PsiContact*, QMimeData*)));
	connect(YaRosterToolTip::instance(), SIGNAL(renameContact(PsiContact*, QMimeData*)), SLOT(renameContact(PsiContact*, QMimeData*)));
#ifdef YAPSI_FANCY_DELETE_CONFIRMATIONS
	connect(YaMultiContactConfirmationToolTip::instance(), SIGNAL(removeContactConfirmation(QMimeData*)), SLOT(removeContactConfirmation(QMimeData*)));
#endif

	optionChanged(showContactListGroupsOptionPath);
	optionChanged(avatarStyleOptionPath);

	int tabIndex = PsiOptions::instance()->getOption(tabIndexOptionPath).toInt();
	if (tabIndex < 0 || tabIndex >= rosterPage_->count())
		tabIndex = qMin(1, rosterPage_->count() - 1);
	Q_ASSERT(tabIndex >= 0 && tabIndex < rosterPage_->count());
	rosterPage_->setCurrentPage(rosterPage_->button(tabIndex));

	if (contactList_) {
		connect(contactList_, SIGNAL(accountCountChanged()), SLOT(accountCountChanged()));
		connect(contactList_, SIGNAL(accountActivityChanged()), SLOT(accountCountChanged()));
	}
	accountCountChanged();
}

YaContactListModel* YaRoster::modelForDeleteOperations() const
{
	return dynamic_cast<YaContactListModel*>(contacts_->contactListView()->realModel());
}

void YaRoster::removeContact(PsiContact* contact, QMimeData* _selection)
{
	YaContactListModel* model = modelForDeleteOperations();
	QModelIndexList indexes = model->indexesFor(contact, _selection);
	if (model && !indexes.isEmpty()) {
		QMimeData* selection = model->mimeData(indexes);
		// WARNING: selection could also contain groups. and when there are groups,
		// all groups' contacts are marked for deletion too
		QList<PsiContact*> contactsLost = model->contactsLostByRemove(selection);
		bool removeConfirmed = contactsLost.isEmpty();

		YaRosterToolTip* toolTip = YaChatToolTip::instance()->isVisible() ?
		                           YaChatToolTip::instance() : YaRosterToolTip::instance();
		QRect currentRect;
		if (!removeConfirmed) {
			rosterPage_->setCurrentPage(contacts_->pageButton());
			contacts_->setFilterModeEnabled(false);
			currentRect = toolTip->currentRect();
			if (currentRect.isNull()) {
				QWidget* w = contacts_->contactListView()->viewport();
				currentRect = QRect(w->mapToGlobal(w->rect().topLeft()),
				                    w->mapToGlobal(w->rect().bottomRight()));
			}
		}

#ifdef YAPSI_FANCY_DELETE_CONFIRMATIONS
		if (contactsLost.isEmpty()) {
			// immediately hide any possible tooltip left-overs
			toolTip->hide();
		}
		else if (contactsLost.count() == 1) {
			QMimeData* selection2 = model->mimeData(indexes);
			toolTip->removeContactConfirmation(currentRect, contactsLost.first(), contacts_->contactListView(), selection2);
		}
		else {
			Q_ASSERT(contactsLost.count() > 1);
			QMimeData* selection2 = model->mimeData(indexes);
			YaMultiContactConfirmationToolTip::instance()->show(currentRect, model, selection2, 0);
		}
#else
		toolTip->hide();

		if (!removeConfirmed) {
			QStringList contactNames;
			foreach(PsiContact* contact, contactsLost) {
				if (dynamic_cast<FakeGroupContact*>(contact)) {
					continue;
				}

				QString name = QString("<b>%1</b>").arg(Qt::escape(contact->name()));
				if (name != contact->jid().full()) {
					// TODO: ideally it should be wrapped in <nobr></nobr>,
					// but it breaks layout in Qt 4.3.4
					name = tr("%1 (%2)").arg(name, Qt::escape(contact->jid().full()));
				}
				contactNames << name;
			}

			// и группу «Фубар» из списка контактов
			// и 2 групп{ы|} из списка контактов

			QString msg = tr("This will permanently remove<br>"
			                 "%1"
			                 "<br>from your contact list."
			                ).arg(contactNames.join(", "));

#ifdef YAPSI
			YaContactListModelSelection contactListSelection(selection);
			if (contactNames.count() > 1 || contactListSelection.groups().count()) {
				QStringList tmp;
				QStringList tmpContactNames = contactNames;
				while (!tmpContactNames.isEmpty()) {
					if (tmp.count() >= 10)
						break;

					tmp << QString("%1. %2").arg(tmp.count() + 1).arg(tmpContactNames.takeFirst());
				}

				QString andNContacts;
				if (!tmpContactNames.isEmpty()) {
					andNContacts = tr("and %n contacts ", 0,
					                  tmpContactNames.count());
				}

				if (contactListSelection.groups().count() > 1) {
					msg = tr("This will permanently remove:<br>"
					         "%1"
					         "<br>%2and %n groups from your contact list.",
					         0,
					         contactListSelection.groups().count()
					        ).arg(tmp.join("<br>"))
					         .arg(andNContacts);
				}
				else if (contactListSelection.groups().count() == 1) {
					msg = tr("This will permanently remove:<br>"
					         "%1"
					         "<br>%2and \"%3\" group from your contact list."
					        ).arg(tmp.join("<br>"))
					         .arg(andNContacts)
					         .arg(contactListSelection.groups().first().fullName);
				}
				else {
					msg = tr("This will permanently remove:<br>"
					         "%1"
					         "<br>%2from your contact list."
					        ).arg(tmp.join("<br>"))
					         .arg(andNContacts);
				}
			}

			if (indexes.count() == 1 && model->indexType(indexes.first()) == ContactListModel::GroupType) {
				msg = tr("This will permanently remove<br>"
				         "%1"
				         "<br>group and all its contacts from your contact list.").arg(indexes.first().data().toString());
			}
#endif

			YaRemoveConfirmationMessageBox msgBox(
			    tr("Deleting contacts"),
			    msg, this);
			removeConfirmed = msgBox.confirmRemoval();
		}
#endif

		if (removeConfirmed) {
			removeContactConfirmation(selection);
		}

		// message box steals focus, so we're restoring it back
		contacts_->contactListView()->restoreSelection(selection);
		bringToFront(contacts_->contactListView());
		contacts_->contactListView()->setFocus();

		delete selection;
	}
}

void YaRoster::renameContact(PsiContact* contact, QMimeData* selection)
{
	Q_UNUSED(selection);
	rosterPage_->setCurrentPage(contacts_->pageButton());
	contacts_->setFilterModeEnabled(false);
	contacts_->setAddContactModeEnabled(false);
	bringToFront(this);

	YaContactListModel* model = dynamic_cast<YaContactListModel*>(contactListModel_);
	Q_ASSERT(model);
	if (!model)
		return;
	QMimeData* mimeData = model->mimeData(model->indexesFor(contact, 0));
	contacts_->contactListView()->restoreSelection(mimeData);
	delete mimeData;

	contacts_->contactListView()->setFocus();

	if (contacts_->contactListView()->currentIndex().isValid()) {
		contacts_->contactListView()->rename();
	}
}

void YaRoster::removeContactConfirmation(QMimeData* contactSelection)
{
	Q_ASSERT(contactSelection);
	YaContactListModel* model = modelForDeleteOperations();
	QModelIndexList indexes = model->indexesFor(0, contactSelection);
	if (model && !indexes.isEmpty()) {
		QMimeData* selection = model->mimeData(indexes);
		model->removeIndexes(selection);
		delete selection;
	}
}

void YaRoster::removeSelection(QMimeData* selection)
{
	removeContact(0, selection);
}

void YaRoster::addSelection(QMimeData* data)
{
	Q_ASSERT(data);
	YaContactListModelSelection selection(data);
	if (!selection.contacts().count())
		return;

	contacts_->addContact(selection.contacts().first().jid);
}

void YaRoster::setAvatarMode(YaContactListView::AvatarMode avatarMode)
{
	avatarMode_ = avatarMode;

	foreach(YaRosterTab* tab, rosterTabs()) {
		if (tab)
			tab->setAvatarMode(avatarMode_);
	}
}

void YaRoster::resizeEvent(QResizeEvent* e)
{
	// contacts_->setFilterModeEnabled(false);
	// contacts_->setAddContactModeEnabled(false);

	QFrame::resizeEvent(e);
	// setMask(Ya::VisualUtil::roundedMask(size(), Ya::VisualUtil::rosterGroupCornerRadius(), Ya::VisualUtil::TopBorders));
}

void YaRoster::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	p.fillRect(rect(), Qt::white);
}

void YaRoster::toggleFilterContacts()
{
	bringToFront(this);
	rosterPage_->setCurrentPage(contacts_->pageButton());
	contacts_->setFilterModeEnabled(!contacts_->filterModeEnabled());
}

void YaRoster::addGroup()
{
	bringToFront(this);

	rosterPage_->setCurrentPage(contacts_->pageButton());
	contacts_->setFilterModeEnabled(false);
	contacts_->setAddContactModeEnabled(false);

	// FIXME: Probably the better way to do it is to add fake contacts
	// globally to PsiContactList
	YaContactListModel* model = modelForDeleteOperations();
	if (!model)
		return;
	Q_ASSERT(contacts_->contactListView()->realModel() == model);
	QModelIndex groupIndex = contacts_->contactListView()->proxyIndex(model->addGroup());
	if (groupIndex.isValid()) {
		contacts_->contactListView()->scrollTo(groupIndex);
		contacts_->contactListView()->setCurrentIndex(groupIndex);
		contacts_->contactListView()->edit(groupIndex);
	}
}

void YaRoster::addContact()
{
	bringToFront(this);

	rosterPage_->setCurrentPage(contacts_->pageButton());
	contacts_->setAddContactModeEnabled(true);
}

void YaRoster::optionChanged(const QString& option)
{
	if (option == showContactListGroupsOptionPath) {
		bool showGroups = PsiOptions::instance()->getOption(showContactListGroupsOptionPath).toBool();
		if (contactListModel_)
			contactListModel_->setGroupsEnabled(showGroups);

		emit enableAddGroupAction(showGroups);
	}
	else if (option == avatarStyleOptionPath) {
		setAvatarMode(static_cast<YaContactListView::AvatarMode>(PsiOptions::instance()->getOption(avatarStyleOptionPath).toInt()));
	}
}

void YaRoster::contextMenuEvent(QContextMenuEvent* e)
{
	e->accept();
}

void YaRoster::setContactListViewportMenu(QMenu* menu)
{
	contacts_->contactListView()->setViewportMenu(menu);
}

QList<QWidget*> YaRoster::proxyableWidgets() const
{
	QList<QWidget*> result;
	foreach(YaRosterTab* tab, rosterTabs()) {
		result += tab->proxyableWidgets();
	}
	return result;
}

void YaRoster::doSetFocus()
{
	contacts_->setFilterModeEnabled(false);
	contacts_->setAddContactModeEnabled(false);

	contacts_->contactListView()->setFocus();
}

void YaRoster::accountCountChanged()
{
	bool tmp = !contactList_.isNull() && contactList_->haveAvailableAccounts() ? contactList_->haveAvailableAccounts() : false;
#ifdef YAPSI_ACTIVEX_SERVER
	tmp &= !contactList_.isNull() && contactList_->accountsLoaded() && contactList_->onlineAccount()->isAvailable();
#endif
	if (tmp != haveAvailableAccounts_) {
		haveAvailableAccounts_ = tmp;
		emit availableAccountsChanged(haveAvailableAccounts_);

		// if (!haveAvailableAccounts_) {
		// 	contacts_->setFilterModeEnabled(false);
		// 	contacts_->setAddContactModeEnabled(false);
		// }
	}
}

bool YaRoster::haveAvailableAccounts() const
{
	return haveAvailableAccounts_;
}

#include "yaroster.moc"
