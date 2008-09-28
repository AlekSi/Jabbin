#ifndef CONTACTLISTMODEL_H
#define CONTACTLISTMODEL_H

#include <QAbstractItemModel>

class ContactListModel : public QAbstractItemModel
{
public:
	ContactListModel() {}

	enum Type {
		InvalidType = 0,
		ContactType = 1,
		GroupType   = 2,
		AccountType = 3
	};

	static ContactListModel::Type indexType(const QModelIndex& index);

	enum {
		// generic
		TypeRole        = Qt::UserRole + 0,
		ActivateRole    = Qt::UserRole + 1,

		// contacts
		JidRole         = Qt::UserRole + 2,
		PictureRole     = Qt::UserRole + 3,
		StatusTextRole  = Qt::UserRole + 4,
		StatusTypeRole  = Qt::UserRole + 5,

		// groups
		ExpandedRole    = Qt::UserRole + 6,
		TotalItemsRole  = Qt::UserRole + 7,

#ifdef YAPSI
		NotifyValueRole = Qt::UserRole + 8,
		LastRole        = Qt::UserRole + 8
#else
		LastRole        = Qt::UserRole + 7
#endif
	};
};

#endif
