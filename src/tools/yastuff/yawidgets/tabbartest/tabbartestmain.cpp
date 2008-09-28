#include <QApplication>

#include <QPainter>

#include "yatabwidget.h"
#include "yatabbar.h"

class MyWidget : public QWidget {
public:
	MyWidget() {}

protected:
	void paintEvent(QPaintEvent*) {
		QPainter p(this);
		p.fillRect(rect(), Qt::white);
	}
};

int main (int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setStyleSheet(
	"QWidget {"
	"	font-family: \"Arial\";"
	"}"
	);

	YaTabWidget w(0);
	w.show();

	for (int i = 1; i <= 10; ++i) {
		QWidget* widget = new MyWidget();
		w.addTab(widget, QPixmap(":/iconsets/roster/roster-small/icon-online.png"), QString("Label %1").arg(i));

		w.setTabHighlighted(w.indexOf(widget), i % 3 == 0);
	}

	return app.exec();
}
