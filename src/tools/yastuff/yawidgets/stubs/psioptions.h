#ifndef PSIOPTIONS_H
#define PSIOPTIONS_H

#include <QObject>
#include <QVariant>

class RealPsiOptions : public QObject
{
	Q_OBJECT
public:
	RealPsiOptions() {}
	QVariant getOption(QString name)
	{
		Q_UNUSED(name);
		return QVariant(false);
	}

	void setOption(QString name, QVariant value)
	{
		Q_UNUSED(name);
		Q_UNUSED(value);
	}
};

class PsiOptions
{
public:
	static RealPsiOptions* instance()
	{
		static RealPsiOptions* opt = 0;
		if (!opt)
			opt = new RealPsiOptions();
		return opt;
	}
};

#endif
