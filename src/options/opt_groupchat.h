#ifndef OPT_GROUPCHAT_H
#define OPT_GROUPCHAT_H

#include "optionstab.h"

class QWidget;
class QListWidgetItem;
struct Options;

class OptionsTabGroupchat : public OptionsTab
{
	Q_OBJECT
public:
	OptionsTabGroupchat(QObject *parent);

	QWidget *widget();
	void applyOptions(Options *opt);
	void restoreOptions(const Options *opt);

	void setData(PsiCon *, QWidget *);
	bool stretchable() const { return true; }

private slots:
	void addNickColor(QString);
	void addGCHighlight();
	void removeGCHighlight();
	void addGCNickColor();
	void removeGCNickColor();
	void chooseGCNickColor();
	void displayGCNickColor();
	void selectedGCNickColor(QListWidgetItem *item);

private:
	QWidget *w, *dlg;
};

#endif
