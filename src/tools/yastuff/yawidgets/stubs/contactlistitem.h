#ifndef CONTACTLISTITEM_H
#define CONTACTLISTITEM_H

class ContactListItem
{
public:
	ContactListItem() {}
	virtual ~ContactListItem() {}

	virtual void foo() {}
	bool expanded() const { return false; }
};

#endif
