#include <QApplication>

#include "ui_test.h"

#include <QLayout>

QLayout *rw_recurseFindLayout(QLayout *lo, QWidget *w)
{
	//printf("scanning layout: %p\n", lo);
	QLayoutIterator it = lo->iterator();
	for(QLayoutItem *i; (i = it.current()); ++it) {
		//printf("found: %p,%p\n", i->layout(), i->widget());
		QLayout *slo = i->layout();
		if(slo) {
			QLayout *tlo = rw_recurseFindLayout(slo, w);
			if(tlo)
				return tlo;
		}
		else if(i->widget() == w)
			return lo;
	}
	return 0;
}

QLayout *rw_findLayoutOf(QWidget *w)
{
	return rw_recurseFindLayout(w->parentWidget()->layout(), w);
}

void replaceWidget(QWidget *a, QWidget *b)
{
	if(!a)
		return;

	QLayout *lo = rw_findLayoutOf(a);
	if(!lo)
		return;
	// printf("decided on this: %p\n", lo);

	if(lo->inherits("QBoxLayout")) {
		QBoxLayout *bo = (QBoxLayout *)lo;
		int n = bo->findWidget(a);
		bo->insertWidget(n+1, b);
		delete a;
	}
}

int main (int argc, char *argv[])
{
	QApplication app(argc, argv);
	QWidget w;
	Ui::Test test;
	test.setupUi(&w);

	// YaSelfMood must always be created the last, otherwise interaction with its
	// expanding menus will be severely limited on non-osx platforms
	YaSelfMood *selfMoodOld = test.selfMood;
	test.selfMood = new YaSelfMood(selfMoodOld->parentWidget());
	replaceWidget(selfMoodOld, test.selfMood);
	test.selfMood->raiseExtraInWidgetStack();

	w.show();
	return app.exec();
}
