#include "contactlistview.h"

#include <QHeaderView>

ContactListView::ContactListView(QWidget* parent)
: QTreeView(parent)
{
	setUniformRowHeights(false);
	setAlternatingRowColors(true);
	// setLargeIcons(false);
	// setShowIcons(true);
	setRootIsDecorated(false);
	// on Macs Enter key is the default EditKey, so we can't use EditKeyPressed
	setEditTriggers(QAbstractItemView::EditKeyPressed); // lesser evil, otherwise no editing will be possible at all due to Qt bug
	// setEditTriggers(QAbstractItemView::NoEditTriggers);
	setIndentation(5);
	header()->hide();

	// setItemDelegate(new PsiContactListViewDelegate(this));

#ifdef Q_WS_MAC
	setFrameShape(QFrame::NoFrame);
#endif
}

bool ContactListView::largeIcons() const
{
	return true;
}

bool ContactListView::showIcons() const
{
	return true;
}
