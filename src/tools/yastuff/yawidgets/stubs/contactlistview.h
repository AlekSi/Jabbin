#ifndef CONTACTLISTVIEW_H
#define CONTACTLISTVIEW_H

#include <QTreeView>

class ContactListItemProxy;

class ContactListView : public QTreeView
{
	Q_OBJECT
public:
	ContactListView(QWidget* parent);

	void setLargeIcons(bool) {}
	void showToolTip(const QModelIndex& index, const QPoint& globalPos) const {}
	ContactListItemProxy* itemProxy(const QModelIndex&) const { return 0; }

	bool largeIcons() const;
	bool showIcons() const;
	bool isContextMenuVisible() const { return false; }

protected slots:
	virtual void itemActivated(const QModelIndex& index) {}
	virtual void itemExpanded(const QModelIndex& index) {}
	virtual void itemCollapsed(const QModelIndex& index) {}
};

#endif
