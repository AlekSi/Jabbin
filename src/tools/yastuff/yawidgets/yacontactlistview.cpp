/*
 * yacontactlistview.cpp - custom contact list widget
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

#include <QStack>
#include <QString>
#include <QAbstractItemView>
#include <QTreeView>

#include "yacontactlistview.h"

#include <QScrollBar>
#include <QTimer>
#include <QPainter>
#include <QPainterPath>
#include <QDragMoveEvent>
#include <QSortFilterProxyModel>

#include "contactlistgroup.h"
#include "yarostertooltip.h"
#include "yamulticontactconfirmationtooltip.h"
#include "contactlistmodel.h"
#include "contactlistitemproxy.h"
#include "psioptions.h"
#include "psicontact.h"
#include "yacontactlistviewdelegate.h"
#include "smoothscrollbar.h"
#include "yavisualutil.h"
#include "yacontactlistmodel.h"
#include "iconaction.h"
#include "shortcutmanager.h"
#include "contactlistitemmenu.h"
#include "yaofficebackgroundhelper.h"
#include "contactlistgroupstate.h"

#include "yacontactlistviewdelegate.h"
#include "yacontactlistviewslimdelegate.h"
#include "yacontactlistviewlargedelegate.h"
#include "yacontactlistmodelselection.h"

static const int SCROLL_SINGLE_STEP = 32;
static const QString optionPathTemplate = QString::fromUtf8("options.ya.roster.%1");
static const QString altRowColorOptionPath = optionPathTemplate.arg("altColor");

/**
 * Custom class that subclasses PsiContactListView in order to
 * provide custom delegate to handle paint events.
 */
YaContactListView::YaContactListView(QWidget* parent)
	: ContactListView(parent)
	, backedUpSelection_(0)
	, backedUpVerticalScrollBarValue_(-1)
	, invalidateDelegateTimer_(0)
	, slimDelegate_(0)
	, normalDelegate_(0)
	, largeDelegate_(0)
	, removeAction_(0)
	, avatarMode_(AvatarMode_Auto)
	, dropIndicatorRect_(QRect())
	, dropIndicatorPosition_(QAbstractItemView::OnViewport)
	, keyboardModifiers_(Qt::NoModifier)
	, dirty_(false)
	, pressedIndexWasSelected_(false)
	, viewportMenu_(0)
{
	removeAction_ = new IconAction("", "psi/remove", QString(), ShortcutManager::instance()->shortcuts("contactlist.delete"), this, "act_remove");
	connect(removeAction_, SIGNAL(activated()), SLOT(removeSelection()));
	addAction(removeAction_);

	QAbstractItemDelegate* delegate = itemDelegate();
	delete delegate;

	slimDelegate_   = new YaContactListViewSlimDelegate(this);
	normalDelegate_ = new YaContactListViewDelegate(this);
	largeDelegate_  = new YaContactListViewLargeDelegate(this);
	invalidateDelegate();

	setAlternatingRowColors(PsiOptions::instance()->getOption(altRowColorOptionPath).toBool());
	setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	verticalScrollBar()->setSingleStep(SCROLL_SINGLE_STEP);
	setIndentation(8);

	setLargeIcons(true);

	connect(this, SIGNAL(entered(const QModelIndex&)), SLOT(updateCursorMouseHover(const QModelIndex&)));
	connect(this, SIGNAL(clicked(const QModelIndex&)), SLOT(itemClicked(const QModelIndex&)));
	connect(this, SIGNAL(viewportEntered()), SLOT(updateCursorMouseHover()));
	
	setSelectionMode(ExtendedSelection);
	viewport()->installEventFilter(this);

	setDragEnabled(true);
	setAcceptDrops(true);
	setDropIndicatorShown(false); // we're painting it by ourselves

	// There are crashes related to this in Qt 4.2.3. Disabling for now.
// #ifndef Q_WS_X11
// 	setAnimated(true);
// #endif

	invalidateDelegateTimer_ = new QTimer(this);
	invalidateDelegateTimer_->setInterval(0);
	invalidateDelegateTimer_->setSingleShot(true);
	connect(invalidateDelegateTimer_, SIGNAL(timeout()), SLOT(invalidateDelegate()));

	YaOfficeBackgroundHelper::instance()->registerWidget(this);

	SmoothScrollBar::install(this);
	connect(verticalScrollBar(), SIGNAL(valueChanged(int)), SLOT(scrollbarValueChanged()));
}

YaContactListView::~YaContactListView()
{
	if (backedUpSelection_) {
		delete backedUpSelection_;
	}

	PsiOptions::instance()->setOption(altRowColorOptionPath, alternatingRowColors());
}

/**
 * Make sure that all keyboard shortcuts are unique in order to avoid
 * "QAction::eventFilter: Ambiguous shortcut overload: Del" messages.
 */
void YaContactListView::addContextMenuAction(QAction* action)
{
	foreach(QAction* act, findChildren<QAction*>()) {
		// TODO: maybe check individual shortcuts too?
		if (act->shortcuts() == action->shortcuts()) {
			return;
		}
	}

	ContactListView::addContextMenuAction(action);
}

void YaContactListView::setItemDelegate(QAbstractItemDelegate* delegate)
{
	if (delegate == itemDelegate())
		return;
	ContactListView::setItemDelegate(delegate);
	modelChanged();
	doItemsLayout();
}

void YaContactListView::resizeEvent(QResizeEvent* e)
{
	ContactListView::resizeEvent(e);
	startInvalidateDelegate();
}

/**
 * This slot is called by YaToolBoxPage when current page is being
 * changed. The purpose of this slot is to avoid visual glitches
 * like scrollbars appearing in the middle of contact list and
 * group header lines being not wide enough.
 */
void YaContactListView::resized()
{
	setViewportMargins(0, 0, 0, 0);
	modelChanged();
	doItemsLayout();
}

void YaContactListView::leaveEvent(QEvent* e)
{
	ContactListView::leaveEvent(e);
	updateCursorMouseHover();
}

int YaContactListView::suggestedItemHeight()
{
	int rowCount = model()->rowCount(QModelIndex());
	if (!rowCount)
		return 0;
	return qMin(viewport()->height() / rowCount - 2, dynamic_cast<YaContactListViewDelegate*>(itemDelegate())->avatarSize());
}

void YaContactListView::mouseDoubleClickEvent(QMouseEvent* e)
{
	if (!style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick, 0, this)) {
		ContactListView::mouseDoubleClickEvent(e);
	}
}

void YaContactListView::itemActivated(const QModelIndex& index)
{
	if (ContactListModel::indexType(index) == ContactListModel::GroupType &&
	    style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick, 0, this)) {
		setExpanded(index, !index.data(ContactListModel::ExpandedRole).toBool());
		return;
	}

	ContactListView::itemActivated(index);
}

/**
 * Returns a list of real-model indexes.
 */
QModelIndexList YaContactListView::indexesFor(PsiContact* contact, QMimeData* contactSelection) const
{
	QModelIndexList indexes;
	YaContactListModel* model = dynamic_cast<YaContactListModel*>(realModel());
	if (model) {
		indexes = model->indexesFor(contact, contactSelection);
	}
	return indexes;
}

void YaContactListView::toolTipEntered(PsiContact* contact, QMimeData* contactSelection)
{
	Q_UNUSED(contact); // we don't want tooltips on multiple selections
	QModelIndexList indexes = indexesFor(0, contactSelection);
	if (indexes.count() == 1) {
		updateCursor(proxyIndex(indexes.first()), UC_TooltipEntered, false);
	}
}

void YaContactListView::toolTipHidden(PsiContact* contact, QMimeData* contactSelection)
{
	Q_UNUSED(contact);
	Q_UNUSED(contactSelection);
	if (!drawSelectionBackground())
		updateCursor(QModelIndex(), UC_TooltipHidden, false);
}

void YaContactListView::updateCursorMouseHover()
{
	updateCursor(QModelIndex(), UC_MouseHover, false);
}

void YaContactListView::updateCursor(const QModelIndex& index, UpdateCursorOrigin origin, bool force)
{
	if (isContextMenuVisible()     ||
	    extendedSelectionAllowed() ||
	    state() != NoState)
	{
		if (!force) {
			return;
		}
	}

	setCursor(!index.isValid() ? Qt::ArrowCursor : Qt::PointingHandCursor);

	if (origin == UC_MouseClick) {
		int value = verticalScrollBar()->value();
		if (index.isValid())
			setCurrentIndex(index);
		else
			clearSelection();
		dynamic_cast<SmoothScrollBar*>(verticalScrollBar())->setValueImmediately(value);
	}

	viewport()->update();
}

void YaContactListView::updateCursorMouseHover(const QModelIndex& index)
{
	updateCursor(index, UC_MouseHover, false);
}

void YaContactListView::showToolTip(const QModelIndex& index, const QPoint& globalPos) const
{
	Q_UNUSED(globalPos);
	// if (ContactListModel::indexType(index) != ContactListModel::ContactType) {
	// 	ContactListView::showToolTip(index, globalPos);
	// 	return;
	// }

	ContactListItemProxy* item = itemProxy(index);

	if (item && !extendedSelectionAllowed() && ContactListModel::indexType(index) == ContactListModel::ContactType)
	{
		QRect rect = dynamic_cast<YaContactListViewDelegate*>(itemDelegate())->rosterToolTipArea(visualRect(index));
		QRect itemRect = QRect(viewport()->mapToGlobal(rect.topLeft()),
		                       viewport()->mapToGlobal(rect.bottomRight()));
		YaRosterToolTip::instance()->showText(itemRect,
		                                      dynamic_cast<PsiContact*>(item->item()),
		                                      this,
		                                      model()->mimeData(QModelIndexList() << index));
	}
	else if (ContactListModel::indexType(index) == ContactListModel::GroupType) {
		item = 0;
	}
#ifdef YAPSI_FANCY_DELETE_CONFIRMATIONS
	else if (item) {
		QMimeData* selection = this->selection();
		YaContactListModelSelection selectionHelper(selection);
		YaContactListModel* model = dynamic_cast<YaContactListModel*>(realModel());
		if (model && selection && selectionHelper.isMultiSelection()) {
			QRect rect = QRect(viewport()->mapToGlobal(viewport()->rect().topLeft()),
			                   viewport()->mapToGlobal(viewport()->rect().bottomRight()));
			YaMultiContactConfirmationToolTip::instance()->show(rect, model, selection, this);
		}
		else if (selection) {
			item = 0;
			delete selection;
		}
	}
#endif

	if (!item) {
		YaRosterToolTip::instance()->hide();
	}
}

void YaContactListView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	ContactListView::selectionChanged(selected, deselected);

	// we must avoid backupCurrentSelection() as result of modelChanged()
	// as modelAboutToBeReset() could get emitted after model is already reset,
	// and as result we'll reference bad pointers
	if (!dirty_) {
		backupCurrentSelection();
	}
}

void YaContactListView::itemExpanded(const QModelIndex& index)
{
	ContactListView::itemExpanded(index);
	startInvalidateDelegate();
}

void YaContactListView::itemCollapsed(const QModelIndex& index)
{
	ContactListView::itemCollapsed(index);
	startInvalidateDelegate();
}

void YaContactListView::startInvalidateDelegate()
{
	invalidateDelegateTimer_->start();
}

int YaContactListView::indexCombinedHeight(const QModelIndex& parent, QAbstractItemDelegate* delegate) const
{
	if (!delegate || !model())
		return 0;
	int result = delegate->sizeHint(QStyleOptionViewItem(), parent).height();
	for (int row = 0; row < model()->rowCount(parent); ++row) {
		QModelIndex index = model()->index(row, 0, parent);
		if (isExpanded(index))
			result += indexCombinedHeight(index, delegate);
		else
			result += delegate->sizeHint(QStyleOptionViewItem(), index).height();
	}
	return result;
}

void YaContactListView::invalidateDelegate()
{
	switch (avatarMode()) {
	case AvatarMode_Disable:
		setItemDelegate(slimDelegate_);
		break;
	case AvatarMode_Big:
		setItemDelegate(largeDelegate_);
		break;
	case AvatarMode_Small:
		setItemDelegate(normalDelegate_);
		break;
	case AvatarMode_Auto:
	default:
		if (indexCombinedHeight(QModelIndex(), largeDelegate_) <= viewport()->height())
			setItemDelegate(largeDelegate_);
		else
			setItemDelegate(normalDelegate_);
		break;
	}
}

void YaContactListView::setModel(QAbstractItemModel* newModel)
{
	if (model()) {
		disconnect(model(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)), this, SLOT(startInvalidateDelegate()));
		disconnect(model(), SIGNAL(rowsInserted(const QModelIndex&, int, int)), this, SLOT(startInvalidateDelegate()));
		disconnect(model(), SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)), this, SLOT(modelChanged()));
		disconnect(model(), SIGNAL(rowsAboutToBeInserted(const QModelIndex&, int, int)), this, SLOT(modelChanged()));
		disconnect(model(), SIGNAL(modelAboutToBeReset()), this, SLOT(modelChanged()));
		disconnect(model(), SIGNAL(layoutAboutToBeChanged()), this, SLOT(modelChanged()));
		disconnect(model(), SIGNAL(layoutChanged()), this, SLOT(doItemsLayout()));
	}

	// it's critical that we hook on signals prior to selectionModel,
	// otherwise it would be pretty hard to maintain consistent selection
	if (newModel) {
		connect(newModel, SIGNAL(rowsRemoved(const QModelIndex&, int, int)), this, SLOT(startInvalidateDelegate()));
		connect(newModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)), this, SLOT(startInvalidateDelegate()));
		connect(newModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)), this, SLOT(modelChanged()));
		connect(newModel, SIGNAL(rowsAboutToBeInserted(const QModelIndex&, int, int)), this, SLOT(modelChanged()));
		connect(newModel, SIGNAL(modelAboutToBeReset()), this, SLOT(modelChanged()));
		connect(newModel, SIGNAL(layoutAboutToBeChanged()), this, SLOT(modelChanged()));
		// this is necessary because we could use some data immediately after
		// invalidating proxy model, and we want tree state to be up to date in order
		// to avoid weird impossible crashes
		connect(newModel, SIGNAL(layoutChanged()), this, SLOT(doItemsLayout()));
	}

	ContactListView::setModel(newModel);
	startInvalidateDelegate();
}

YaContactListView::AvatarMode YaContactListView::avatarMode() const
{
	return avatarMode_;
}

void YaContactListView::setAvatarMode(YaContactListView::AvatarMode avatarMode)
{
	avatarMode_ = avatarMode;
	invalidateDelegate();
}

bool YaContactListView::textInputInProgress() const
{
	return state() == QAbstractItemView::EditingState;
}

void YaContactListView::paintEvent(QPaintEvent* e)
{
	// Q_ASSERT(!dirty_);
	// if (dirty_)
	// 	return;
	ContactListView::paintEvent(e);

	// drawDragIndicator!!
	if (state() == QAbstractItemView::DraggingState &&
	    viewport()->cursor().shape() != Qt::ForbiddenCursor &&
	    !dropIndicatorRect_.isEmpty())
	{
		QPainter p(viewport());
		// QColor dragIndicatorColor = QColor(0x17, 0x0B, 0xFF);
		QColor dragIndicatorColor = QColor(0x00, 0x00, 0x00);
		p.setPen(QPen(dragIndicatorColor, 2));
		p.setRenderHint(QPainter::Antialiasing, true);
		QPainterPath path;
		path.addRoundRect(dropIndicatorRect_.adjusted(2, 1, 0, 0), 5);
		p.drawPath(path);
	}

	if (!model() || model()->rowCount() == 0) {
		QPainter p(viewport());
		paintNoContactsStub(&p);
	}
}

void YaContactListView::paintNoContactsStub(QPainter* p)
{
	YaContactListModel* model = dynamic_cast<YaContactListModel*>(realModel());
	if (model &&
	    model->contactList() &&
	    model->contactList()->accountsLoaded() &&
	    !model->contactList()->haveConnectingAccounts())
	{
		return;
	}

	// TODO: combine this with YaFilteredContactListView::paintNoContactsStub() code?
	QString text = tr("Loading contact list...");
	int margin = 5;
	QRect rect(viewport()->rect().adjusted(margin, margin, -margin, -margin));

	int flags = Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap;

	p->setPen(Qt::black);
	p->drawText(rect, flags, text);
}

bool YaContactListView::supportsDropOnIndex(QDropEvent* e, const QModelIndex& index) const
{
	YaContactListModelSelection selection(e->mimeData());
	YaContactListModel* model = dynamic_cast<YaContactListModel*>(realModel());
	if (!selection.haveRosterSelection())
		model = 0;

	if (model && !model->supportsMimeDataOnIndex(e->mimeData(), realIndex(index))) {
		return false;
	}

	if (dropPosition(e, selection, index) == OnViewport) {
		return false;
	}

	return true;
}

static void updateDefaultDropAction(QDropEvent* e)
{
	if (e->keyboardModifiers() == Qt::NoModifier) {
		e->setDropAction(Qt::MoveAction);
	}
}

static void acceptDropAction(QDropEvent* e)
{
	if (e->keyboardModifiers() == Qt::NoModifier) {
		Q_ASSERT(e->dropAction() == Qt::MoveAction);
		e->accept();
	}
	else {
		e->acceptProposedAction();
	}
}

void YaContactListView::dragMoveEvent(QDragMoveEvent* e)
{
	QModelIndex index = indexAt(e->pos());
	dropIndicatorRect_ = QRect();
	dropIndicatorPosition_ = OnViewport;

	// this is crucial at least for auto-scrolling, possibly other things
	ContactListView::dragMoveEvent(e);

	if (supportsDropOnIndex(e, index)) {
		bool ok = true;
		YaContactListModelSelection selection(e->mimeData());
		dropIndicatorPosition_ = dropPosition(e, selection, index);
		if (dropIndicatorPosition_ == AboveItem || dropIndicatorPosition_ == BelowItem) {
			dropIndicatorRect_ = groupReorderDropRect(dropIndicatorPosition_, selection, index);
		}
		else if (dropIndicatorPosition_ == OnItem) {
			dropIndicatorRect_ = onItemDropRect(index);
		}
		else {
			ok = false;
		}

		if (ok) {
			updateDefaultDropAction(e);
			acceptDropAction(e);
			return;
		}
	}

	e->ignore();
	viewport()->update();
}

void YaContactListView::scrollbarValueChanged()
{
	dropIndicatorRect_ = QRect();
	dropIndicatorPosition_ = OnViewport;
	viewport()->update();
}

void YaContactListView::dragEnterEvent(QDragEnterEvent* e)
{
	dynamic_cast<SmoothScrollBar*>(verticalScrollBar())->setEnableSmoothScrolling(false);

	ContactListView::dragEnterEvent(e);
	updateDefaultDropAction(e);
	acceptDropAction(e);
}

void YaContactListView::dragLeaveEvent(QDragLeaveEvent* e)
{
	dynamic_cast<SmoothScrollBar*>(verticalScrollBar())->setEnableSmoothScrolling(true);

	ContactListView::dragLeaveEvent(e);
}

void YaContactListView::dropEvent(QDropEvent* e)
{
	updateDefaultDropAction(e);
	dynamic_cast<SmoothScrollBar*>(verticalScrollBar())->setEnableSmoothScrolling(true);

	QModelIndex index = indexAt(e->pos());
	dropIndicatorRect_ = QRect();

	if (dropIndicatorPosition_ == OnViewport || !supportsDropOnIndex(e, index)) {
		e->ignore();
		viewport()->update();
		return;
	}

	if (dropIndicatorPosition_ == OnItem) {
		if (model()->dropMimeData(e->mimeData(),
		                          e->dropAction(), -1, -1, index)) {
		}
		acceptDropAction(e);
	}
	else if (dropIndicatorPosition_ == AboveItem || dropIndicatorPosition_ == BelowItem) {
		reorderGroups(e, index);
		acceptDropAction(e);
	}

	stopAutoScroll();
	setState(NoState);
	viewport()->update();
}

QAbstractItemView::DropIndicatorPosition YaContactListView::dropPosition(QDropEvent* e, const YaContactListModelSelection& selection, const QModelIndex& index) const
{
	if (selection.groups().count() > 0) {
		// TODO: return OnItem in case the special modifiers are pressed
		// even in case there are multiple groups selected
		// in order to create nested groups
		if (selection.groups().count() == 1) {
			QModelIndex group = itemToReorderGroup(selection, index);
			if (group.isValid()) {
				QRect rect = groupVisualRect(group);
				if (e->pos().y() >= rect.center().y()) {
					return BelowItem;
				}
				else {
					return AboveItem;
				}
			}
		}

		return OnViewport;
	}

	if (selection.groups().count() + selection.contacts().count() > 0)
		return OnItem;
	else
		return OnViewport;
}

struct OrderedGroup {
	QString name;
	int order;
};

void YaContactListView::reorderGroups(QDropEvent* e, const QModelIndex& index)
{
	YaContactListModelSelection selection(e->mimeData());
	QModelIndex item = itemToReorderGroup(selection, index);
	if (!item.isValid()) {
		return;
	}
	YaContactListModel* model = dynamic_cast<YaContactListModel*>(realModel());
	Q_ASSERT(model);
	Q_ASSERT(selection.groups().count() > 0);
	Q_ASSERT(selection.contacts().count() == 0);
	QModelIndexList selectedGroups = model->indexesFor(&selection);
	if (!selectedGroups.count())
		return;

	// we need to work in terms of proxy indexes because we need actually-sorted items
	QModelIndex selectedGroup = proxyIndex(selectedGroups.first());
	QModelIndex groupParent = selectedGroup.parent();

	QModelIndexList groups;
	for (int row = 0; row < this->model()->rowCount(groupParent); ++row) {
		QModelIndex i = this->model()->index(row, selectedGroup.column(), groupParent);
		if (ContactListModel::indexType(i) == ContactListModel::GroupType)
			groups << i;
	}

	// re-order
	groups.removeAll(selectedGroup);
	if (dropIndicatorPosition_ == AboveItem) {
		groups.insert(groups.indexOf(item), selectedGroup);
	}
	else {
		Q_ASSERT(dropIndicatorPosition_ == BelowItem);
		groups.insert(groups.indexOf(item) + 1, selectedGroup);
	}

	foreach(QModelIndex i, groups) {
		model->setGroupOrder(i.data(ContactListModel::FullGroupNameRole).toString(),
		                     groups.indexOf(i));
	}
}

QModelIndex YaContactListView::itemToReorderGroup(const YaContactListModelSelection& selection, const QModelIndex& index) const
{
	YaContactListModel* model = dynamic_cast<YaContactListModel*>(realModel());
	Q_ASSERT(model);
	Q_ASSERT(selection.groups().count() == 1);
	QModelIndexList selectedGroups = model->indexesFor(&selection);
	if (!selectedGroups.count())
		return QModelIndex();
	QModelIndex selectedGroup = selectedGroups.first();
	// we need to work in terms of proxy indexes because we need actually-sorted items
	selectedGroup = proxyIndex(selectedGroup);
	const QModelIndex groupParent = selectedGroup.parent();

	QModelIndex indexParent = index;
	while (indexParent.isValid() && indexParent.parent() != groupParent) {
		indexParent = indexParent.parent();
	}
	if (indexParent.parent() != groupParent || indexParent == selectedGroup)
		return QModelIndex();

	// this code is necessary if we allow contacts on the same level as group items
	bool breakAtFirstGroup = (ContactListModel::indexType(index) != ContactListModel::GroupType) && index.parent() == groupParent;
	QModelIndex result;
	for (int row = 0; row < this->model()->rowCount(groupParent); ++row) {
		result = this->model()->index(row, indexParent.column(), groupParent);
		if (breakAtFirstGroup) {
			if (ContactListModel::indexType(result) == ContactListModel::GroupType)
				break;
		}
		else if (result.row() == indexParent.row()) {
			break;
		}
	}
	return result;
}

QRect YaContactListView::groupReorderDropRect(DropIndicatorPosition dropIndicatorPosition, const YaContactListModelSelection& selection, const QModelIndex& index) const
{
	QModelIndex group = itemToReorderGroup(selection, index);
	QRect r(groupVisualRect(group));
	if (dropIndicatorPosition == AboveItem) {
		return QRect(r.left(), r.top(), r.width(), 1);
	}
	else {
		Q_ASSERT(dropIndicatorPosition == BelowItem);
		QRect result(r.left(), r.bottom(), r.width(), 1);
		if (result.bottom() < viewport()->height() - 2)
			result.translate(0, 2);
		return result;
	}
}

QRect YaContactListView::onItemDropRect(const QModelIndex& index) const
{
	if (!index.isValid()) {
		return viewport()->rect().adjusted(0, 0, -1, -1);
	}

	if (ContactListModel::indexType(index) != ContactListModel::GroupType) {
		return onItemDropRect(index.parent());
	}

	return groupVisualRect(index);
}

QRect YaContactListView::groupVisualRect(const QModelIndex& index) const
{
	Q_ASSERT(ContactListModel::indexType(index) == ContactListModel::GroupType);

	QRect result;
	combineVisualRects(index, &result);
	return result.adjusted(0, 0, -1, -1);
}

void YaContactListView::combineVisualRects(const QModelIndex& parent, QRect* result) const
{
	Q_ASSERT(result);
	*result = result->united(visualRect(parent));
	for (int row = 0; row < model()->rowCount(parent); ++row) {
		QModelIndex index = model()->index(row, 0, parent);
		combineVisualRects(index, result);
	}
}

void YaContactListView::startDrag(Qt::DropActions supportedActions)
{
	dynamic_cast<SmoothScrollBar*>(verticalScrollBar())->setEnableSmoothScrolling(false);

	ContactListView::startDrag(supportedActions);
}

QMimeData* YaContactListView::selection() const
{
	YaContactListModel* model = dynamic_cast<YaContactListModel*>(realModel());
	if (model && model->contactList() && !selectedIndexes().isEmpty()) {
		return model->mimeData(realIndexes(selectedIndexes()));
	}
	return 0;
}

void YaContactListView::restoreSelection(QMimeData* _mimeData)
{
	// setCurrentIndex() actually fires up the timers which in turn could trigger
	// some delayed events that would result in mimeData deletion (if the mimeData
	// passed was actually backedUpSelection_). So for additional insurance if
	// clearSelection() suddenly decides to trigger timers too we put mimeData
	// into a QPointer
	QPointer<QMimeData> mimeData(_mimeData);
	setUpdatesEnabled(false);

	// setCurrentIndex(QModelIndex());
	clearSelection();

	YaContactListModel* model = dynamic_cast<YaContactListModel*>(realModel());
	if (model && !mimeData.isNull()) {
		QModelIndexList indexes = proxyIndexes(model->indexesFor(mimeData));
		if (!indexes.isEmpty()) {
			setCurrentIndex(indexes.first());
			QItemSelection selection;
			foreach(QModelIndex index, indexes)
				selection << QItemSelectionRange(index);
			selectionModel()->select(selection, QItemSelectionModel::Select);
		}
	}

	setUpdatesEnabled(true);
}

ContactListItemMenu* YaContactListView::createContextMenuFor(ContactListItem* item) const
{
	ContactListItemMenu* menu = ContactListView::createContextMenuFor(item);
	if (menu) {
		if (menu->metaObject()->indexOfSignal("removeSelection()") != -1)
			connect(menu, SIGNAL(removeSelection()), SLOT(removeSelection()));
		if (menu->metaObject()->indexOfSignal("addSelection()") != -1)
			connect(menu, SIGNAL(addSelection()), SLOT(addSelection()));
		if (menu->metaObject()->indexOfSignal("addGroup()") != -1)
			connect(menu, SIGNAL(addGroup()), SIGNAL(addGroup()));
	}
	return menu;
}

void YaContactListView::addSelection()
{
	QMimeData* mimeData = selection();
	emit addSelection(mimeData);
	delete mimeData;
}

void YaContactListView::removeSelection()
{
	QMimeData* mimeData = selection();
	emit removeSelection(mimeData);
	delete mimeData;
}

bool YaContactListView::extendedSelectionAllowed() const
{
	return selectedIndexes().count() > 1 || keyboardModifiers_ != 0;
}

bool YaContactListView::activateItemsOnSingleClick() const
{
	return false;
	// return !extendedSelectionAllowed() &&
	//        state() != QAbstractItemView::EditingState;
}

void YaContactListView::updateKeyboardModifiers(const QEvent* e)
{
	if (e->type() == QEvent::KeyPress ||
	    e->type() == QEvent::KeyRelease ||
	    e->type() == QEvent::MouseButtonDblClick ||
	    e->type() == QEvent::MouseButtonPress ||
	    e->type() == QEvent::MouseButtonRelease ||
	    e->type() == QEvent::MouseMove)
	{
		keyboardModifiers_ = static_cast<const QInputEvent*>(e)->modifiers();
	}
}

Qt::KeyboardModifiers YaContactListView::keyboardModifiers() const
{
	return keyboardModifiers_;
}

bool YaContactListView::eventFilter(QObject* obj, QEvent* e)
{
	if (obj == this || obj == viewport()) {
		updateKeyboardModifiers(e);
	}

	return ContactListView::eventFilter(obj, e);
}

YaContactListViewDelegate* YaContactListView::yaContactListViewDelegate() const
{
	YaContactListViewDelegate* delegate = dynamic_cast<YaContactListViewDelegate*>(itemDelegate());
	Q_ASSERT(delegate);
	return delegate;
}

void YaContactListView::mousePressEvent(QMouseEvent* e)
{
	pressedIndex_ = QModelIndex();
	pressedIndexWasSelected_ = false;

	QModelIndex index = indexAt(e->pos());
	if (ContactListModel::indexType(index) == ContactListModel::GroupType && e->button() == Qt::LeftButton) {
		QStyleOptionViewItem option;
		option.rect = visualRect(index);
		QPoint offset = e->pos() /*- QPoint(option.rect.left(), 0)*/;
		if (yaContactListViewDelegate() && yaContactListViewDelegate()->groupButtonRect(option, index).contains(offset)) {
			setExpanded(index, !index.data(ContactListModel::ExpandedRole).toBool());
			e->accept();
			return;
		}
	}

	pressedIndex_ = index;
	pressedIndexWasSelected_ = e->button() == Qt::LeftButton &&
	                           selectionModel() &&
	                           selectionModel()->isSelected(pressedIndex_);
	if (!index.isValid()) {
		// clear selection
		updateCursor(index, UC_MouseClick, true);
	}
	ContactListView::mousePressEvent(e);
}

void YaContactListView::mouseMoveEvent(QMouseEvent* e)
{
	// don't allow any selections after single-clicking on the group button
	if (!pressedIndex_.isValid() && !pressedIndexWasSelected_ && e->buttons() & Qt::LeftButton) {
		e->accept();
		return;
	}

	ContactListView::mouseMoveEvent(e);
}

void YaContactListView::itemClicked(const QModelIndex& index)
{
	if (activateItemsOnSingleClick())
		return;

	// if clicked item was selected prior to mouse press event, activate it on release
	// if (pressedIndexWasSelected_) {
	// 	activate(index);
	// }
	Q_UNUSED(index);
}

void YaContactListView::contextMenuEvent(QContextMenuEvent* e)
{
	QModelIndex index = indexAt(e->pos());
	if (index.isValid() && !selectedIndexes().contains(index)) {
		updateCursor(index, UC_MouseClick, true);
	}

	ContactListView::contextMenuEvent(e);

	if (!e->isAccepted() && viewportMenu_) {
		viewportMenu_->exec(e->globalPos());
	}

	repairMouseTracking();
}

bool YaContactListView::drawSelectionBackground() const
{
	return focusPolicy() == Qt::NoFocus     ||
	       testAttribute(Qt::WA_UnderMouse) ||
	       extendedSelectionAllowed()       ||
	       YaRosterToolTip::instance()->isVisible();
}

void YaContactListView::closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint)
{
	ContactListView::closeEditor(editor, hint);

#if 0
	if (ContactListModel::indexType(currentIndex()) == ContactListModel::GroupType &&
	    hint != QAbstractItemDelegate::SubmitModelCache) {
		ContactListItemProxy* item = itemProxy(currentIndex());
		ContactListGroup* group = item ? dynamic_cast<ContactListGroup*>(item->item()) : 0;
		if (group->isFake()) {
			QModelIndexList indexes;
			indexes << currentIndex();

			YaContactListModel* model = dynamic_cast<YaContactListModel*>(realModel());
			Q_ASSERT(model);
			QMimeData* selection = model->mimeData(realIndexes(indexes));
			model->removeIndexes(selection);
			delete selection;
		}
	}
#endif
}

void YaContactListView::backupCurrentSelection()
{
	if (backedUpSelection_) {
		delete backedUpSelection_;
		backedUpSelection_ = 0;
	}

	backedUpSelection_ = selection();
}

void YaContactListView::restoreBackedUpSelection()
{
	restoreSelection(backedUpSelection_);

	if (backedUpSelection_) {
		delete backedUpSelection_;
		backedUpSelection_ = 0;
	}
}

void YaContactListView::modelChanged()
{
	if (!dirty_) {
		setUpdatesEnabled(false);
		backedUpVerticalScrollBarValue_ = verticalScrollBar()->value();
		if (currentEditor()) {
			backedUpEditorValue_ = currentEditor()->text();
			closeEditor(currentEditor(), QAbstractItemDelegate::NoHint);
			setEditingIndex(currentIndex(), true);
		}
		else {
			backedUpEditorValue_ = QString();
		}
		dirty_ = true;
		scheduleDelayedItemsLayout();
	}

	// TODO: save the object we're currently editing and the QVariant of the value the user inputted

	// make sure selectionModel() doesn't cache any currently selected indexes
	// otherwise it'll overwrite our correctly restored selection with its own
	if (selectionModel()) {
		selectionModel()->reset();
	}
}

void YaContactListView::doItemsLayout()
{
	if (dirty_) {
		dirty_ = false;

		YaContactListModel* model = dynamic_cast<YaContactListModel*>(realModel());
		if (model) {
			SmoothScrollBar* smoothScrollBar = dynamic_cast<SmoothScrollBar*>(verticalScrollBar());
			// on Qt 4.3.5 even if we disable the updates on widget, range updates will make the
			// scrollbar show / hide on Windows only (this bug is fixed in Qt 4.4)
			smoothScrollBar->setRangeUpdatesEnabled(false);

			ContactListGroupState::GroupExpandedState groupExpandedState;
			groupExpandedState = model->groupState()->groupExpandedState();

			// clear all QTreeView::d->expandedIndexes, in order to avoid any potential crashes
			// we're going to re-expand them correctly using updateGroupExpandedState() call
			collapseAll();
			// this is also useful since restoring selection could mess the group expanding state
			model->groupState()->restoreGroupExpandedState(ContactListGroupState::GroupExpandedState());

			ContactListView::doItemsLayout();

			model->groupState()->restoreGroupExpandedState(groupExpandedState);
			updateGroupExpandedState();

			smoothScrollBar->setRangeUpdatesEnabled(true);
			if (backedUpVerticalScrollBarValue_ != -1) {
				if (smoothScrollBar) {
					smoothScrollBar->setValueImmediately(backedUpVerticalScrollBarValue_);
				}
			}

			restoreBackedUpSelection();

			if (!backedUpEditorValue_.isNull()) {
				// QSortFilterProxyModel* proxyModel = dynamic_cast<QSortFilterProxyModel*>(model);
				// if (proxyModel) {
				// 	proxyModel->invalidate();
				// }

				setFocus();
				rename();
				if (currentEditor()) {
					currentEditor()->setText(backedUpEditorValue_);
				}
			}
		}
	}

	setUpdatesEnabled(true);
}

/**
 * Prevent emitting activated events when clicking with right mouse button.
 */
void YaContactListView::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() & Qt::LeftButton || !activateItemsOnSingleClick())
		QTreeView::mouseReleaseEvent(event);
}

void YaContactListView::setViewportMenu(QMenu* menu)
{
	viewportMenu_ = menu;
}
