#ifndef PSITOOLTIP_H
#define PSITOOLTIP_H

class PsiToolTip
{
	PsiToolTip()
	{}
public:
	static void showText(const QPoint &pos, const QString &text, QWidget *w = 0) {
		Q_UNUSED(pos);
		Q_UNUSED(text);
		Q_UNUSED(w);
	}
	static void install(QWidget*) {}
};

#endif
