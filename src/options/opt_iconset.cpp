#include "opt_iconset.h"
#include "common.h"
#include "iconwidget.h"
#include "applicationinfo.h"
#include "psiiconset.h"
#include "psicon.h"

#include <QDir>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qfileinfo.h>
#include <qapplication.h>
#include <qthread.h>
#include <qmutex.h>
#include <qevent.h>
#include <qcursor.h>
#include <qpalette.h>
#include <qtabwidget.h>
#include <QFont>
#include <QTreeWidget>

#include "ui_opt_iconset_emo.h"
#include "ui_opt_iconset_system.h"
#include "ui_opt_iconset_roster.h"
#include "ui_ui_isdetails.h"

class IconsetEmoUI : public QWidget, public Ui::IconsetEmo
{
public:
	IconsetEmoUI() : QWidget() { setupUi(this); }
};

class IconsetSystemUI : public QWidget, public Ui::IconsetSystem
{
public:
	IconsetSystemUI() : QWidget() { setupUi(this); }
};

class IconsetRosterUI : public QWidget, public Ui::IconsetRoster
{
public:
	IconsetRosterUI() : QWidget() { setupUi(this); }
};

class IconsetDetailsDlg : public QDialog, public Ui::IconsetDetailsDlg
{
public:
	IconsetDetailsDlg(QWidget* parent, const char* name, bool modal)
		: QDialog(parent)
	{
		setAttribute(Qt::WA_DeleteOnClose);
		setupUi(this); 
		
		QStringList bold_labels;
		bold_labels << "lb_name2";
		bold_labels << "lb_version2";
		bold_labels << "lb_date2";
		bold_labels << "lb_home2";
		bold_labels << "lb_desc2";
		bold_labels << "lb_authors";
		
		QList<QLabel *> labels = findChildren<QLabel *>();
		foreach (QLabel *l, labels) {
			if (bold_labels.contains(l->objectName())) {
				QFont font = l->font();
				font.setBold(true);
				l->setFont(font);
			}
		}
		
		setName(name); 
		setModal(modal); 
	}
	
	void setIconset( const Iconset &is ) {
		setWindowTitle(caption().arg(is.name()));
		
		lb_name->setText(is.name());
		lb_version->setText(is.version());
		
		if ( is.description().isEmpty() ) {
			lb_desc->hide();
			lb_desc2->hide();
		}
		else
			lb_desc->setText(is.description());
		
		if ( !is.homeUrl().isEmpty() ) {
			lb_home->setTitle(is.homeUrl());
			lb_home->setUrl(is.homeUrl());
		}
		else {
			lb_home->hide();
			lb_home2->hide();
		}
			
		if ( is.creation().isEmpty() ) {
			lb_date->hide();
			lb_date2->hide();
		}
		else {
			QDate date = QDate::fromString(is.creation(), Qt::ISODate);	
			lb_date->setText(date.toString(Qt::LocalDate));
		}
		
		if ( is.authors().count() == 0 ) {
			ptv_authors->hide();
			lb_authors->hide();
		}
		else {
			QString authors;
			QStringList::ConstIterator it = is.authors().begin();
			for ( ; it != is.authors().end(); ++it) {
				if ( !authors.isEmpty() )
					authors += "<br><br>";
				authors += *it;
			}
			ptv_authors->setText( "<qt><nobr>" + authors + "</nobr></qt>" );
		}
		
		isd_iconset->setIconset(is);
		
		resize(sizeHint());
	}
};

static void isDetails(const Iconset &is, QWidget *parent)
{
	IconsetDetailsDlg *isd = new IconsetDetailsDlg(parent, "IconsetDetailsDlg", false);
	isd->setIconset(is);
	isd->show();
}

static QStringList dirs;

static int countIconsets(QString addDir, QStringList excludeList)
{
	int count = 0;

	QStringList::Iterator it = dirs.begin();
	for ( ; it != dirs.end(); ++it) {
		QString fileName = *it + "/iconsets" + addDir;
		QDir dir (fileName);

		QStringList list = dir.entryList("*");
		QStringList::Iterator it2 = list.begin();
		for ( ; it2 != list.end(); ++it2) {
			if ( *it2 == "." || *it2 == ".." )
				continue;

			bool found = false;
			QStringList::Iterator it3 = excludeList.begin();
			for ( ; it3 != excludeList.end(); ++it3) {
				if ( *it2 == *it3 ) {
					found = true;
					break;
				}
			}

			if ( found )
				continue;

			count++;
			excludeList << *it2;
		}
	}

	return count;
}

//----------------------------------------------------------------------------
// IconsetLoadEvent
//----------------------------------------------------------------------------

class IconsetLoadEvent : public QEvent
{
public:
	IconsetLoadEvent(IconsetLoadThread *par, Iconset *i)
	: QEvent(QEvent::User)
	{
		p  = par;
		is = i;
	}

	IconsetLoadThread *thread() const { return p; }

	// if iconset() is '0' then it means that iconset wasn't loaded successfully
	Iconset *iconset() const { return is; }

private:
	IconsetLoadThread *p;
	Iconset *is;
};

//----------------------------------------------------------------------------
// IconsetFinishEvent
//----------------------------------------------------------------------------

class IconsetFinishEvent : public QEvent
{
public:
	IconsetFinishEvent()
	: QEvent( (QEvent::Type)(QEvent::User + 1) )
	{
	}
};


//----------------------------------------------------------------------------
// IconsetLoadThreadDestroyEvent
//----------------------------------------------------------------------------

class IconsetLoadThreadDestroyEvent : public QEvent
{
public:
	IconsetLoadThreadDestroyEvent(QThread *t)
	: QEvent( (QEvent::Type)(QEvent::User + 2) )
	{
		thread = t;
	}

	~IconsetLoadThreadDestroyEvent()
	{
		thread->wait();
		delete thread;
	}

private:
	QThread *thread;
};

//----------------------------------------------------------------------------
// IconsetLoadThread
//----------------------------------------------------------------------------

class IconsetLoadThread : public QThread
{
public:
	IconsetLoadThread(QObject *parent, QString addPath);
	void excludeIconsets(QStringList);

	bool cancelled;

protected:
	void run();
	void postEvent(QEvent *);

private:
	QObject *parent;
	QString addPath;
	QStringList excludeList;
};

IconsetLoadThread::IconsetLoadThread(QObject *p, QString path)
{
	cancelled = false;
	parent  = p;
	addPath = path;
}

void IconsetLoadThread::excludeIconsets(QStringList l)
{
	excludeList += l;
}

static QMutex threadCancelled, threadMutex;

void IconsetLoadThread::postEvent(QEvent *e)
{
	threadCancelled.lock();
	bool cancel = cancelled;
	threadCancelled.unlock();

	if ( cancel ) {
		delete e;
		return;
	}

	QApplication::postEvent(parent, e);
}

void IconsetLoadThread::run()
{
	threadMutex.lock();
	QStringList dirs = ::dirs;
	threadMutex.unlock();

	QStringList::Iterator it = dirs.begin();
	for ( ; it != dirs.end(); ++it) {
		QString fileName = *it + "/iconsets" + addPath;
		QDir dir (fileName);

		QStringList list = dir.entryList("*");
		QStringList::Iterator it2 = list.begin();
		for ( ; it2 != list.end(); ++it2) {
			if ( *it2 == "." || *it2 == ".." )
				continue;

			threadCancelled.lock();
			bool cancel = cancelled;
			threadCancelled.unlock();

			if ( cancel )
				goto getout;

			bool found = false;
			QStringList::Iterator it3 = excludeList.begin();
			for ( ; it3 != excludeList.end(); ++it3) {
				if ( *it2 == *it3 ) {
					found = true;
					break;
				}
			}

			if ( found )
				continue;

			IconsetLoadEvent *event = 0;
			Iconset *is = new Iconset;

			if ( is->load (fileName + "/" + *it2) ) {
				excludeList << *it2;

				// don't forget to delete iconset in ::event()!
				event = new IconsetLoadEvent(this, is);
			}
			else {
				delete is;
				event = new IconsetLoadEvent(this, 0);

				// without excluding corrupted iconset,
				// counter will go nuts! read more comments
				// about that...
				excludeList << *it2;
				// TODO: there is possibility,
				// that there's a bunch of same-named
				// iconsets, and some of them are corrupted.
				// It is possible to write a hack that
				// loads iconset even in that case.

				// logic:
				//   tried to load iconset --> unable to load
				//   checking if there's same-named iconsets in
				//   other directories
				//   emit IconsetLoadEvent() only on success
				//   or when last corrupted iconset was unable
				//   to load
			}

			postEvent(event);
		}
	}

getout:
	postEvent(new IconsetFinishEvent());
	QApplication::postEvent(qApp, new IconsetLoadThreadDestroyEvent(this)); // self destruct
}

//----------------------------------------------------------------------------
// OptionsTabIconsetSystem
//----------------------------------------------------------------------------

OptionsTabIconsetSystem::OptionsTabIconsetSystem(QObject *parent)
: OptionsTab(parent, "iconset_system", "", tr("System Icons"), tr("Select the system iconset"))
{
	w = 0;
	thread = 0;

	if ( dirs.isEmpty() ) {
		dirs << ":";
		dirs << ".";
		dirs << ApplicationInfo::homeDir();
		dirs << ApplicationInfo::resourcesDir();
	}
}

OptionsTabIconsetSystem::~OptionsTabIconsetSystem()
{
	cancelThread();
}

QWidget *OptionsTabIconsetSystem::widget()
{
	if ( w )
		return 0;

	w = new IconsetSystemUI;
	IconsetSystemUI *d = (IconsetSystemUI *)w;

	connect(d->pb_sysDetails, SIGNAL(clicked()), SLOT(previewIconset()));

	// TODO: add QWhatsThis

	return w;
}

void OptionsTabIconsetSystem::applyOptions(Options *opt)
{
	if ( !w || thread )
		return;

	IconsetSystemUI *d = (IconsetSystemUI *)w;
	const Iconset *is = d->iss_system->iconset();
	if ( is ) {
		QFileInfo fi( is->fileName() );
		opt->systemIconset = fi.fileName();
	}
}

void OptionsTabIconsetSystem::restoreOptions(const Options *opt)
{
	if ( !w || thread )
		return;

	o = (Options *)opt;
	IconsetSystemUI *d = (IconsetSystemUI *)w;
	d->iss_system->clear();
	QStringList loaded;

	d->setCursor(Qt::WaitCursor);

	QPalette customPal = d->palette();
	customPal.setDisabled(customPal.inactive());
	d->iss_system->setEnabled(false);
	d->iss_system->setPalette(customPal);

	d->pb_sysDetails->setEnabled(false);
	d->pb_sysDetails->setPalette(customPal);

	d->progress->show();
	d->progress->setValue( 0 );

	numIconsets = countIconsets("/system", loaded);
	iconsetsLoaded = 0;

	cancelThread();

	thread = new IconsetLoadThread(this, "/system");
	thread->start();
}

bool OptionsTabIconsetSystem::event(QEvent *e)
{
	IconsetSystemUI *d = (IconsetSystemUI *)w;
	if ( e->type() == QEvent::User ) { // iconset load event
		IconsetLoadEvent *le = (IconsetLoadEvent *)e;

		if ( thread != le->thread() )
			return false;

		if ( !numIconsets )
			numIconsets = 1;
		d->progress->setValue( (int)((float)100 * ++iconsetsLoaded / numIconsets) );

		Iconset *i = le->iconset();
		if ( i ) {
			PsiIconset::instance()->stripFirstAnimFrame(i);
			d->iss_system->insert(*i);

			QFileInfo fi( i->fileName() );
			if ( fi.fileName() == o->systemIconset )
				d->iss_system->setItemSelected(d->iss_system->lastItem(), true);

			delete i;
		}

		return true;
	}
	else if ( e->type() == QEvent::User + 1 ) { // finish event
		d->iss_system->setEnabled(true);
		d->iss_system->unsetPalette();

		d->pb_sysDetails->setEnabled(true);
		d->pb_sysDetails->unsetPalette();

		connect(d->iss_system, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), SIGNAL(dataChanged()));

		d->progress->hide();

		d->unsetCursor();
		thread = 0;

		return true;
	}

	return false;
}

void OptionsTabIconsetSystem::previewIconset()
{
	IconsetSystemUI *d = (IconsetSystemUI *)w;
	const Iconset *is = d->iss_system->iconset();
	if ( is )
		isDetails(*is, parentWidget);
}

void OptionsTabIconsetSystem::setData(PsiCon *, QWidget *p)
{
	parentWidget = p;
}

void OptionsTabIconsetSystem::cancelThread()
{
	if ( thread ) {
		threadCancelled.lock();
		thread->cancelled = true;
		threadCancelled.unlock();

		thread = 0;
	}
}

//----------------------------------------------------------------------------
// OptionsTabIconsetEmoticons
//----------------------------------------------------------------------------

OptionsTabIconsetEmoticons::OptionsTabIconsetEmoticons(QObject *parent)
: OptionsTab(parent, "iconset_emoticons", "", tr("Emoticons"), tr("Select your emoticon iconsets"))
{
	w = 0;
	thread = 0;
}

OptionsTabIconsetEmoticons::~OptionsTabIconsetEmoticons()
{
	cancelThread();
}

QWidget *OptionsTabIconsetEmoticons::widget()
{
	if ( w )
		return 0;

	w = new IconsetEmoUI;
	IconsetEmoUI *d = (IconsetEmoUI *)w;

	connect(d->pb_emoUp, SIGNAL(clicked()), d->iss_emoticons, SLOT(moveItemUp()));
	connect(d->pb_emoUp, SIGNAL(clicked()), SIGNAL(dataChanged()));
	connect(d->pb_emoDown, SIGNAL(clicked()), d->iss_emoticons, SLOT(moveItemDown()));
	connect(d->pb_emoDown, SIGNAL(clicked()), SIGNAL(dataChanged()));
	connect(d->pb_emoDetails, SIGNAL(clicked()), SLOT(previewIconset()));

	d->ck_useEmoticons->setWhatsThis(
		tr("<P>Emoticons are short sequences of characters that are used to convey an emotion or idea.</P>"
		"<P>Enable this option if you want Psi to replace common emoticons with a graphical image.</P>"
		"<P>For example, <B>:-)</B> would be replaced by <icon name=\"psi/smile\"></P>"));

	// TODO: add QWhatsThis

	return w;
}

void OptionsTabIconsetEmoticons::applyOptions(Options *opt)
{
	if ( !w || thread )
		return;

	IconsetEmoUI *d = (IconsetEmoUI *)w;
	opt->useEmoticons = d->ck_useEmoticons->isChecked();

	opt->emoticons.clear();
	for (int row = 0; row < d->iss_emoticons->count(); row++) {
		IconWidgetItem *item = (IconWidgetItem *)d->iss_emoticons->item(row);

		if ( d->iss_emoticons->isItemSelected(item) ) {
			const Iconset *is = item->iconset();
			if ( is ) {
				QFileInfo fi( is->fileName() );
				opt->emoticons << fi.fileName();
			}
		}
	}
}

void OptionsTabIconsetEmoticons::restoreOptions(const Options *opt)
{
	if ( !w || thread )
		return;

	IconsetEmoUI *d = (IconsetEmoUI *)w;
	d->ck_useEmoticons->setChecked( opt->useEmoticons );

	// fill in the iconset view
	d->iss_emoticons->clear();

	{
		Q3PtrListIterator<Iconset> it ( PsiIconset::instance()->emoticons );
		Iconset *is;
		for ( ; (is = it.current()); ++it) {
			d->iss_emoticons->insert(*is);
			d->iss_emoticons->setItemSelected(d->iss_emoticons->lastItem(), true);
		}
	}


	{
		QStringList loaded;
		{
			Q3PtrListIterator<Iconset> it( PsiIconset::instance()->emoticons );
			Iconset *tmp;
			for ( ; (tmp = it.current()); ++it) {
				QFileInfo fi ( tmp->fileName() );
				loaded << fi.fileName();
			}
		}

		d->setCursor(Qt::WaitCursor);

		QPalette customPal = d->palette();
		customPal.setDisabled(customPal.inactive());
		d->ck_useEmoticons->setEnabled(false);
		d->ck_useEmoticons->setPalette(customPal);

		d->groupBox9->setEnabled(false);
		d->groupBox9->setPalette(customPal);

		d->progress->show();
		d->progress->setValue( 0 );

		numIconsets = countIconsets("/emoticons", loaded);
		iconsetsLoaded = 0;

		cancelThread();

		thread = new IconsetLoadThread(this, "/emoticons");
		thread->excludeIconsets(loaded);
		thread->start();
	}
}

bool OptionsTabIconsetEmoticons::event(QEvent *e)
{
	IconsetEmoUI *d = (IconsetEmoUI *)w;
	if ( e->type() == QEvent::User ) { // iconset load event
		IconsetLoadEvent *le = (IconsetLoadEvent *)e;

		if ( thread != le->thread() )
			return false;

		if ( !numIconsets )
			numIconsets = 1;
		d->progress->setValue( (int)((float)100 * ++iconsetsLoaded / numIconsets) );

		Iconset *is = le->iconset();
		if ( is ) {
			PsiIconset::removeAnimation(is);
			d->iss_emoticons->insert(*is);
			delete is;
		}

		return true;
	}
	else if ( e->type() == QEvent::User + 1 ) { // finish event
		d->ck_useEmoticons->setEnabled(true);
		d->ck_useEmoticons->unsetPalette();

		d->groupBox9->setEnabled(true);
		d->groupBox9->unsetPalette();

		connect(d->iss_emoticons, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), SIGNAL(dataChanged()));

		bool checked = d->ck_useEmoticons->isChecked();
		d->ck_useEmoticons->setChecked( true );
		d->ck_useEmoticons->setChecked( checked );

		d->progress->hide();

		d->unsetCursor();
		thread = 0;

		return true;
	}

	return false;
}

void OptionsTabIconsetEmoticons::previewIconset()
{
	IconsetEmoUI *d = (IconsetEmoUI *)w;
	const Iconset *is = d->iss_emoticons->iconset();
	if ( is )
		isDetails(*is, parentWidget);
}

void OptionsTabIconsetEmoticons::setData(PsiCon *, QWidget *p)
{
	parentWidget = p;
}

void OptionsTabIconsetEmoticons::cancelThread()
{
	if ( thread ) {
		threadCancelled.lock();
		thread->cancelled = true;
		threadCancelled.unlock();

		thread = 0;
	}
}

//----------------------------------------------------------------------------
// OptionsTabIconsetRoster
//----------------------------------------------------------------------------

OptionsTabIconsetRoster::OptionsTabIconsetRoster(QObject *parent)
: OptionsTab(parent, "iconset_roster", "", tr("Roster Icons"), tr("Select iconsets for your roster"))
{
	w = 0;
	thread = 0;
}

OptionsTabIconsetRoster::~OptionsTabIconsetRoster()
{
	cancelThread();
}

QWidget *OptionsTabIconsetRoster::widget()
{
	if ( w )
		return 0;

	w = new IconsetRosterUI;
	IconsetRosterUI *d = (IconsetRosterUI *)w;

	// Initialize transport iconsets
	QList<QTreeWidgetItem *> items;
	QTreeWidgetItem *ti = new QTreeWidgetItem(d->tw_isServices, QStringList(QString("AIM")));
	ti->setData(0, Qt::UserRole, QVariant(QString("aim")));
	items.append(ti);
        ti = new QTreeWidgetItem(d->tw_isServices, QStringList(QString("Gadu-Gadu")));
        ti->setData(0, Qt::UserRole, QVariant(QString("gadugadu")));
        items.append(ti);
        ti = new QTreeWidgetItem(d->tw_isServices, QStringList(QString("ICQ")));
        ti->setData(0, Qt::UserRole, QVariant(QString("icq")));
        items.append(ti);
        ti = new QTreeWidgetItem(d->tw_isServices, QStringList(QString("MSN")));
        ti->setData(0, Qt::UserRole, QVariant(QString("msn")));
        items.append(ti);
        ti = new QTreeWidgetItem(d->tw_isServices, QStringList(QString("SMS")));
        ti->setData(0, Qt::UserRole, QVariant(QString("sms")));
        items.append(ti);
        ti = new QTreeWidgetItem(d->tw_isServices, QStringList(QString("Yahoo!")));
        ti->setData(0, Qt::UserRole, QVariant(QString("yahoo")));
        items.append(ti);
	d->tw_isServices->insertTopLevelItems(0, items);
	
	d->tw_isServices->resizeColumnToContents(0);

	connect(d->pb_defRosterDetails, SIGNAL(clicked()), SLOT(defaultDetails()));

	connect(d->iss_servicesRoster, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), SLOT(isServices_iconsetSelected(QListWidgetItem *, QListWidgetItem *)));
	connect(d->tw_isServices, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), SLOT(isServices_selectionChanged(QTreeWidgetItem *)));
	connect(d->pb_servicesDetails, SIGNAL(clicked()), SLOT(servicesDetails()));
	isServices_selectionChanged(0);

	connect(d->iss_customRoster, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), SLOT(isCustom_iconsetSelected(QListWidgetItem *, QListWidgetItem *)));
	connect(d->tw_customRoster, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), SLOT(isCustom_selectionChanged(QTreeWidgetItem *)));
	connect(d->tw_customRoster, SIGNAL(itemClicked(QTreeWidgetItem *, int )), SLOT(isCustom_selectionChanged(QTreeWidgetItem *)));

	connect(d->pb_customRosterDetails, SIGNAL(clicked()), SLOT(customDetails()));
	connect(d->le_customRoster, SIGNAL(textChanged(const QString &)), SLOT(isCustom_textChanged()));
	connect(d->pb_customRosterAdd, SIGNAL(clicked()), SLOT(isCustom_add()));
	connect(d->pb_customRosterDelete, SIGNAL(clicked()), SLOT(isCustom_delete()));
	isCustom_selectionChanged(0);

	d->ck_useTransportIconsForContacts->setWhatsThis(
		tr("Toggles use of transport icons to the contacts, that use that transports."));

	// TODO: add QWhatsThis

	return w;
}

void OptionsTabIconsetRoster::applyOptions(Options *opt)
{
	if ( !w || thread )
		return;

	IconsetRosterUI *d = (IconsetRosterUI *)w;
	opt->useTransportIconsForContacts = d->ck_useTransportIconsForContacts->isChecked();

	// roster - default
	{
		const Iconset *is = d->iss_defRoster->iconset();
		if ( is ) {
			QFileInfo fi( is->fileName() );
			opt->defaultRosterIconset = fi.fileName();
		}
	}

	// roster - services
	{
		opt->serviceRosterIconset.clear();

		QTreeWidgetItemIterator it( d->tw_isServices );
		while(*it) {
			opt->serviceRosterIconset[(*it)->data(0, Qt::UserRole).toString()] = (*it)->data(1, Qt::UserRole).toString();
			++it;
		}
	}

	// roster - custom
	{
		opt->customRosterIconset.clear();

		QTreeWidgetItemIterator it( d->tw_customRoster );
		while(*it) {
			opt->customRosterIconset[(*it)->data(0, Qt::UserRole).toString()] = (*it)->data(1, Qt::UserRole).toString();
			++it;
		}
	}
}

void OptionsTabIconsetRoster::restoreOptions(const Options *opt)
{
	if ( !w || thread )
		return;

	o = (Options *)opt;
	IconsetRosterUI *d = (IconsetRosterUI *)w;
	d->ck_useTransportIconsForContacts->setChecked( opt->useTransportIconsForContacts );

	d->iss_defRoster->clear();
	d->iss_servicesRoster->clear();
	d->iss_customRoster->clear();

	QStringList loaded;

	d->setCursor(Qt::WaitCursor);

	QPalette customPal = d->palette();
	customPal.setDisabled(customPal.inactive());
	d->tabWidget3->setEnabled(false);
	d->tabWidget3->setPalette(customPal);

	d->progress->show();
	d->progress->setValue( 0 );

	numIconsets = countIconsets("/roster", loaded);
	iconsetsLoaded = 0;

	cancelThread();

	thread = new IconsetLoadThread(this, "/roster");
	thread->start();
}

bool OptionsTabIconsetRoster::event(QEvent *e)
{
	IconsetRosterUI *d = (IconsetRosterUI *)w;
	if ( e->type() == QEvent::User ) { // iconset load event
		IconsetLoadEvent *le = (IconsetLoadEvent *)e;

		if ( thread != le->thread() )
			return false;

		if ( !numIconsets )
			numIconsets = 1;
		d->progress->setValue( (int)((float)100 * ++iconsetsLoaded / numIconsets) );

		Iconset *i = le->iconset();
		if ( i ) {
			PsiIconset::instance()->stripFirstAnimFrame(i);
			QFileInfo fi( i->fileName() );

			// roster - default
			d->iss_defRoster->insert(*i);
			if ( fi.fileName() == o->defaultRosterIconset )
				d->iss_defRoster->setItemSelected(d->iss_defRoster->lastItem(), true);

			// roster - service
			d->iss_servicesRoster->insert(*i);

			// roster - custom
			d->iss_customRoster->insert(*i);

			delete i;
		}

		return true;
	}
	else if ( e->type() == QEvent::User + 1 ) { // finish event
		// roster - service
		{
			// fill the QTreeWidget
			QTreeWidgetItemIterator it( d->tw_isServices );
			while(*it) {
				QTreeWidgetItem *i = *it;
				if ( !i->data(0, Qt::UserRole).toString().isEmpty() ) {
					Iconset *iss = PsiIconset::instance()->roster[o->serviceRosterIconset[i->data(0, Qt::UserRole).toString()]];
					if ( iss ) {
						i->setText(1, iss->name());
						QFileInfo fi ( iss->fileName() );
						i->setData(1, Qt::UserRole, fi.fileName());
					}
				}
				++it;
			}
		}

		// roster - custom
		{
			// Then, fill the QListView
			QTreeWidgetItem *last = 0;
			QMap<QString, QString>::ConstIterator it = o->customRosterIconset.begin();
			for ( ; it != o->customRosterIconset.end(); ++it) {
				QTreeWidgetItem *item = new QTreeWidgetItem(d->tw_customRoster, last);
				last = item;

				item->setText(0, clipCustomText(it.key())); // RegExp
				item->setData(0, Qt::UserRole, it.key());

				Iconset *iss = PsiIconset::instance()->roster[it.data()];
				if ( iss ) {
					item->setText(1, iss->name());
					QFileInfo fi ( iss->fileName() );
					item->setData(1, Qt::UserRole, fi.fileName());
				}
			}
			d->tw_customRoster->resizeColumnToContents(0);
		}

		d->tabWidget3->setEnabled(true);
		d->tabWidget3->unsetPalette();

		connect(d->iss_defRoster, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), SIGNAL(dataChanged()));
		connect(d->iss_servicesRoster, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), SIGNAL(dataChanged()));
		connect(d->iss_customRoster, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), SIGNAL(dataChanged()));

		d->progress->hide();

		d->unsetCursor();
		thread = 0;

		return true;
	}

	return false;
}

void OptionsTabIconsetRoster::setData(PsiCon *, QWidget *p)
{
	parentWidget = p;
}

void OptionsTabIconsetRoster::defaultDetails()
{
	IconsetRosterUI *d = (IconsetRosterUI *)w;
	const Iconset *is = d->iss_defRoster->iconset();
	if ( is )
		isDetails(*is, parentWidget);
}

void OptionsTabIconsetRoster::servicesDetails()
{
	IconsetRosterUI *d = (IconsetRosterUI *)w;
	const Iconset *is = d->iss_servicesRoster->iconset();
	if ( is )
		isDetails(*is, parentWidget);
}

void OptionsTabIconsetRoster::customDetails()
{
	IconsetRosterUI *d = (IconsetRosterUI *)w;
	const Iconset *is = d->iss_customRoster->iconset();
	if ( is )
		isDetails(*is, parentWidget);
}

//------------------------------------------------------------

void OptionsTabIconsetRoster::isServices_iconsetSelected(QListWidgetItem *item, QListWidgetItem *)
{
	if ( !item )
		return;

	IconsetRosterUI *d = (IconsetRosterUI *)w;
	QTreeWidgetItem *it = d->tw_isServices->currentItem();
	if ( !it )
		return;

	const Iconset *is = ((IconWidgetItem *)item)->iconset();
	if ( !is )
		return;

	it->setText(1, is->name());
	QFileInfo fi ( is->fileName() );
	it->setData(1, Qt::UserRole, fi.fileName());
}

void OptionsTabIconsetRoster::isServices_selectionChanged(QTreeWidgetItem *it)
{
	IconsetRosterUI *d = (IconsetRosterUI *)w;
	d->iss_servicesRoster->setEnabled( it != 0 );
	d->pb_servicesDetails->setEnabled( it != 0 );
	d->iss_servicesRoster->clearSelection();
	if ( !it )
		return;

	if ( it->data(1, Qt::UserRole).toString().isEmpty() )
		return;

	QString name = it->data(1, Qt::UserRole).toString();

	emit noDirty(true);
	for (int row = 0; row < d->iss_servicesRoster->count(); row++) {
		IconWidgetItem *item = (IconWidgetItem *)d->iss_servicesRoster->item(row);
		const Iconset *is = item->iconset();
		if ( is ) {
			QFileInfo fi ( is->fileName() );
			if ( fi.fileName() == name ) {
				emit noDirty(true);
				d->iss_servicesRoster->setItemSelected(item, true);
				d->iss_servicesRoster->scrollToItem(item);
				emit noDirty(false);
				break;
			}
		}
	}
	qApp->processEvents();
	emit noDirty(false);
}

//------------------------------------------------------------

void OptionsTabIconsetRoster::isCustom_iconsetSelected(QListWidgetItem *item, QListWidgetItem *)
{
	if ( !item )
		return;

	IconsetRosterUI *d = (IconsetRosterUI *)w;
	QTreeWidgetItem *it = d->tw_customRoster->currentItem();
	if ( !it )
		return;

	const Iconset *is = ((IconWidgetItem *)item)->iconset();
	if ( !is )
		return;

	it->setText(1, is->name());
	QFileInfo fi ( is->fileName() );
	it->setData(1, Qt::UserRole,  fi.fileName());
}

void OptionsTabIconsetRoster::isCustom_selectionChanged(QTreeWidgetItem *it)
{
	IconsetRosterUI *d = (IconsetRosterUI *)w;
	d->iss_customRoster->setEnabled( it != 0 );
	d->pb_customRosterDetails->setEnabled( it != 0 );
	//d->pb_customRosterAdd->setEnabled( it != 0 );
	d->pb_customRosterDelete->setEnabled( it != 0 );
	d->le_customRoster->setEnabled( it != 0 );
	d->iss_customRoster->clearSelection();
	if ( !it )
		return;

	if ( it->data(1, Qt::UserRole).toString().isEmpty() )
		return;

	emit noDirty(true);
	d->le_customRoster->setText(it->data(0, Qt::UserRole).toString());
	QString name = it->data(1, Qt::UserRole).toString();

	for (int row = 0; row < d->iss_customRoster->count(); row++) {
		IconWidgetItem *item = (IconWidgetItem *)d->iss_customRoster->item(row);
		const Iconset *is = item->iconset();
		if ( is ) {
			QFileInfo fi ( is->fileName() );
			if ( fi.fileName() == name ) {
				d->iss_customRoster->setItemSelected(item, true);
				d->iss_customRoster->scrollToItem(item);
				break;
			}
		}
	}
	qApp->processEvents();
	emit noDirty(false);
}

void OptionsTabIconsetRoster::isCustom_textChanged()
{
	IconsetRosterUI *d = (IconsetRosterUI *)w;
	QTreeWidgetItem *item = d->tw_customRoster->currentItem();
	if ( !item )
		return;

	item->setText( 0, clipCustomText(d->le_customRoster->text()) );
	item->setData( 0, Qt::UserRole, d->le_customRoster->text() );
}

void OptionsTabIconsetRoster::isCustom_add()
{
	IconsetRosterUI *d = (IconsetRosterUI *)w;

	QString def;
	const Iconset *iconset = d->iss_defRoster->iconset();
	if ( iconset ) {
		QFileInfo fi( iconset->fileName() );
		def = fi.fileName();
	}
	else
		qWarning("OptionsTabIconsetRoster::isCustom_add(): 'def' is null!");

	QTreeWidgetItem *item = new QTreeWidgetItem(d->tw_customRoster);
	const Iconset *i = PsiIconset::instance()->roster[def];
	if ( i ) {
		item->setText(1, i->name());

		QFileInfo fi(i->fileName());
		item->setData(1, Qt::UserRole, fi.fileName());
	}

	d->tw_customRoster->setCurrentItem(item);
	emit dataChanged();

	d->le_customRoster->setFocus();
}

void OptionsTabIconsetRoster::isCustom_delete()
{
	IconsetRosterUI *d = (IconsetRosterUI *)w;
	QTreeWidgetItem *item = d->tw_customRoster->currentItem();
	if ( !item )
		return;

	delete item;
	isCustom_selectionChanged(0);
	d->tw_customRoster->clearSelection();
	emit dataChanged();
}

QString OptionsTabIconsetRoster::clipCustomText(QString s)
{
	if ( s.length() > 10 ) {
		s = s.left(9);
		s += "...";
	}

	return s;
}

void OptionsTabIconsetRoster::cancelThread()
{
	if ( thread ) {
		threadCancelled.lock();
		thread->cancelled = true;
		threadCancelled.unlock();

		thread = 0;
	}
}
