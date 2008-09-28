#ifndef YAROSTERTOOLTIP_H
#define YAROSTERTOOLTIP_H

class PsiContact;

class YaRosterToolTip
{
	YaRosterToolTip()
	{}
public:
	static void showText(QRect, PsiContact*, const QWidget*) {}
	static void install(QWidget*) {}
};

#endif
