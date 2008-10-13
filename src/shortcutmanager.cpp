#include <QAction>
#include <QCoreApplication>

#include "psioptions.h"
#include "shortcutmanager.h"
#include "globalshortcutmanager.h"


/**
 * \brief The Construtor of the Shortcutmanager
 */
ShortcutManager::ShortcutManager() : QObject(QCoreApplication::instance())
{
	// Make sure that there is at least one shortcut for sending messages
	if (shortcuts("chat.send").isEmpty()) {
		qWarning("Restoring chat.send shortcut");
		QVariantList vl;
		vl  << qVariantFromValue(QKeySequence(Qt::Key_Enter/* + Qt::CTRL*/))
		    << qVariantFromValue(QKeySequence(Qt::Key_Return/* + Qt::CTRL*/));
		PsiOptions::instance()->setOption("options.shortcuts.chat.send",vl);
	}
}

/**
 * "The one and only instance" of the ShortcutManager
 */
ShortcutManager* ShortcutManager::instance_ = NULL;

/**
 * \brief The Instantiator of the Shortcutmanager
 */
ShortcutManager* ShortcutManager::instance()
{
	if(!instance_)
		instance_ = new ShortcutManager();
	return instance_;
}

/**
 * \brief get the QKeySequence associated with the keyname "name"
 * \param the shortcut name e.g. "misc.sendmessage" which is in the options xml then
 *        mirrored as options.shortcuts.misc.sendmessage
 * \return QKeySequence, the Keysequence associated with the keyname, or
 *   the first key sequence if there are multiple
 */
QKeySequence ShortcutManager::shortcut(const QString& name)
{
	QVariant variant = PsiOptions::instance()->getOption(QString("options.shortcuts.%1").arg(name));
	QString type = variant.typeName();
	if (type == "QVariantList")
		variant = variant.toList().first();
	//qDebug() << "shortcut: " << name << variant.value<QKeySequence>().toString();
	return variant.value<QKeySequence>();
}

/**
 * \brief get the QVariantList associated with the keyname "name"
 * \param the shortcut name e.g. "misc.sendmessage" which is in the options xml then
 *        mirrored as options.shortcuts.misc.sendmessage
 * \return List of sequences
 */
QList<QKeySequence> ShortcutManager::shortcuts(const QString& name)
{
	QList<QKeySequence> list;
	QVariant variant = PsiOptions::instance()->getOption(QString("options.shortcuts.%1").arg(name));
	QString type = variant.typeName();
	if (type == "QVariantList") {
		foreach(QVariant variant, variant.toList()) {
			list += variant.value<QKeySequence>();
		}
	}
	else {
		QKeySequence k = variant.value<QKeySequence>();
		if (!k.isEmpty())
			list += k;
	}
	return list;
}


/**
 * \brief this function connects the Key or Keys associated with the keyname "path" with the slot "slot"
 *        of the Widget "parent"
 * \param path, the shortcut name e.g. "misc.sendmessage" which is in the options xml then
 *        mirrored as options.shortcuts.misc.sendmessage
 * \param parent, the widget to which the new QAction should be connected to
 * \param slot, the SLOT() of the parent which should be triggerd if the KeySequence is activated
 */
void ShortcutManager::connect(const QString& path, QObject* parent, const char* slot)
{
	if (parent == NULL || slot == NULL)
		return;

	if (!path.startsWith("global.")) {
		QList<QKeySequence> shortcuts;
		foreach(QKeySequence sequence, ShortcutManager::instance()->shortcuts(path))
			if (!sequence.isEmpty())
				shortcuts << sequence;

		if (!shortcuts.isEmpty()) {
			bool appWide = path.startsWith("appwide.");
			QAction* act = new QAction(parent);
			act->setShortcuts(shortcuts);
			act->setShortcutContext(appWide ?
			                        Qt::ApplicationShortcut : Qt::WindowShortcut);
			if (parent->isWidgetType())
				((QWidget*) parent)->addAction(act);
			parent->connect(act, SIGNAL(activated()), slot);
		}
	}
	else {
		foreach(QKeySequence sequence, ShortcutManager::instance()->shortcuts(path)) {
			if (!sequence.isEmpty()) {
				GlobalShortcutManager::instance()->connect(sequence, parent, slot);
			}
		}
	}
}

