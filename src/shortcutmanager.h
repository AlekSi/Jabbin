#ifndef SHORTCUTMANAGER_H
#define SHORTCUTMANAGER_H

#include <QObject>
#include <QList>

class QAction;
class QKeySequence;
class QString;

class ShortcutManager : public QObject
{
public:
	static ShortcutManager* instance();
	static void connect(const QString& path, QObject *parent, const char* slot);
	QKeySequence shortcut(const QString& name);
	QList<QKeySequence> shortcuts(const QString& name);

private:
	ShortcutManager();
	static ShortcutManager* instance_;
};

#endif
