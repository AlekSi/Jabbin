#ifndef CONTACTLISTGROUPITEM_H
#define CONTACTLISTGROUPITEM_H

#include "contactlistitem.h"

class ContactListGroupItem : public ContactListItem
{
public:
	ContactListGroupItem() {}
	bool expanded() const { return false; }
};

#endif
