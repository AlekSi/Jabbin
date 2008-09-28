#ifndef M11EDITABLELABEL_H
#define M11EDITABLELABEL_H

#include <QLabel>

namespace Ya
{

class M11EditableLabel : public QLabel
{
	Q_OBJECT
public:
	M11EditableLabel(QWidget *parent = 0)
		: QLabel(parent)
	{}

	void setEditable(bool) {}
};

}

#endif
