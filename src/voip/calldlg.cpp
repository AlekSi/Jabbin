/*
 * calldlg.cpp - dialog for handling calls
 * Copyright (C) 2005 Oleksandr Yakovlyev <me@yshurik.kiev.ua>
 * Copyright (C) 2008 Dmitriy Kazimirov <dkazimirov@issart.com>
 */

#include "common.h"
#include "calldlg.h"
#include "common.h"
#include "psiaccount.h"
#include "voicecaller.h"
#include "profiles.h"
#include "jinglevoicecaller.h"
#include "qapplication.h"


#include "callslog.h"

#ifdef Q_WS_WIN
#include <windows.h>
#endif

#include <QLabel>
#include <QGridLayout>
#include <QLineEdit>
#include <QTimer>
#include <QTime>
#include <QSlider>
#include <QDebug>
#include <QToolButton>
#include <QPushButton>
#include <QSignalMapper>
#include <QPainter>
#include <QVector>
#include <QResizeEvent>

#include <qcolor.h>

//#include "jabbinoptions.h"

int call_dlg_mic_level = 100;
int call_dlg_dsp_level = 100;

class CallDlg::Private
{
public:
    Private(CallDlg *d) {dlg = d; }
    CallDlg *dlg;

    PsiAccount* pa;

	Jid jid;

    QGridLayout* layout;

	QLabel* l_from;
    QLabel* l_to;
    QLabel* l_time;
    QLabel* l_status;
    QLabel* l_number;
    QLabel* l_format;

    QLineEdit* le_number;

    int sec;
    QTimer* secTimer;
    int secondaryTimerId;
	QTime timeSpent;
	QTime timeSinceLastFire;
	SoundStreamView * s_mic;
	SoundStreamView * s_dsp;

	QSlider * sl_mic;
	QSlider * sl_dsp;

	bool answer;

	VoiceCaller * voiceCaller; //voice caller instance for actualy VoIP calls

	CallStatus status;
	//CallItem logItem; // to save calls history

	bool isPhoneCall();
};


bool CallDlg::Private::isPhoneCall()
{
    return jid.compare(pa->userAccount().dtmf_resource);
}

CallDlg::CallDlg(const Jid &jid, PsiAccount *pa, VoiceCaller * voiceCaller)
:AdvancedWidget<QWidget>(0L,0L)
{
	d = new Private(this);
	d->jid = jid;
	d->pa = pa;
    d->sec = 0;
    d->secTimer = 0L;
	//dkzm:allocate our own personal voice caller
    d->voiceCaller = voiceCaller;
	d->secondaryTimerId=0;
    setCaption(tr("Calling ")+jid.user());
    qDebug( tr("************ Calling") +jid.full() );

    //d->logItem.callTime = QDateTime::currentDateTime();
    //d->logItem.jid = jid.full();
    //d->logItem.isAccepted = false;


    d->layout = new QGridLayout(this);
    d->layout->setSpacing(5);
    d->layout->setMargin(5);

    tb_accept = new QToolButton(this);
    tb_accept->setTextLabel(tr("Call"));
    //tb_accept->setIconSet(IconsetFactory::iconPixmap("psi/makeCall22"));
    tb_accept->setUsesTextLabel(true);
    tb_accept->setTextPosition(QToolButton::BesideIcon);
    tb_accept->setAutoRaise(true);

    tb_reject = new QToolButton(this);
    tb_reject->setTextLabel(tr("Hangup"));
    //tb_reject->setIconSet(IconsetFactory::iconPixmap("psi/callReject22"));
    tb_reject->setUsesTextLabel(true);
    tb_reject->setTextPosition(QToolButton::BesideIcon);
    tb_reject->setAutoRaise(true);

    tb_accept->setMinimumWidth(70);
    tb_reject->setMinimumWidth(70);
    tb_accept->setMinimumHeight(30);
    tb_reject->setMinimumHeight(30);

    QGridLayout * hb1 = new QGridLayout();
    hb1->addWidget(tb_accept, 0, 0);
    hb1->addWidget(tb_reject, 0, 1);
    hb1->setSpacing(5);

    d->l_from = new QLabel(this);
    d->l_from->setText(d->jid.user());
    
    //const VCard * vcard = VCardFactory::vcard(jid);

	//if (vcard && !vcard->nickName().isEmpty()) {
    //    d->l_from->setText(vcard->nickName());
    //}
    
    d->l_time = new QLabel(tr("Duration:"), this);
    d->l_status = new QLabel(tr("Status:"), this);
    
    QFont f = d->l_from->font();
    
    if ( f.pixelSize() >0 )
        f.setPixelSize( f.pixelSize()+2 );
    else if ( f.pointSize() >0 )
        f.setPointSize( f.pointSize()+2 );
    
    d->l_status->setFont( f );
    f.setItalic(true);
    d->l_time->setFont( f );
    f.setItalic(false);
    f.setBold(true);
    d->l_from->setFont( f );

    QGridLayout * hb3 = new QGridLayout();
	if ( !d->isPhoneCall()) {
	    QLabel * l_face = new QLabel(this);
	    //l_face->setPixmap( IconsetFactory::icon("psi/face") );
	    l_face->setFixedSize(64,64);

		/*if (vcard) {
			if ( !vcard->photo().isEmpty() ) {
			    QImage img(vcard->photo());
			
				int w = img.width();
				int h = img.height();

				QImage s_img;
				if (w>h)
				    s_img = img.smoothScale(64, (64*h)/w);
				else
				    s_img = img.smoothScale((64*w)/h, 64);
            
				QPixmap p_img(64,64);
				p_img.fill(Qt::color0);
				QPainter p( &p_img );
				p.drawPixmap((64-s_img.width())/2 ,0, s_img);
				p.end();

				l_face->setPixmap(p_img);
				l_face->setFixedSize(p_img.size());
			}
		}*/

        hb3->addMultiCellWidget(l_face, 0,4, 0,0);
		hb3->addWidget(d->l_from, 0,1);
		hb3->addWidget(d->l_status, 1,1);
		hb3->addWidget(d->l_time, 2,1);
	}
    
    d->sl_dsp = new QSlider(Qt::Horizontal, this);
    d->sl_mic = new QSlider(Qt::Horizontal, this);

    d->sl_dsp->setMaxValue(100);
    d->sl_mic->setMaxValue(100);
    d->sl_dsp->setFocusPolicy(Qt::NoFocus);
    d->sl_mic->setFocusPolicy(Qt::NoFocus);

    //call_dlg_dsp_level = SJabbinOptions::instance()->dsp_level;
    //call_dlg_mic_level = SJabbinOptions::instance()->mic_level;

    d->sl_dsp->setValue(call_dlg_dsp_level);
    d->sl_mic->setValue(call_dlg_mic_level);

	connect(d->sl_dsp, SIGNAL(valueChanged(int)), this, SLOT(dspValueChanged(int)));
	connect(d->sl_mic, SIGNAL(valueChanged(int)), this, SLOT(micValueChanged(int)));

    QGridLayout * gl = new QGridLayout();
    QLabel * l_dsp = new QLabel(tr("Speakers:"), this);
    QLabel * l_mic = new QLabel(tr("Microphone:"), this);
    //l_dsp->setPixmap(IconsetFactory::iconPixmap("psi/music_player"));
    //l_mic->setPixmap(IconsetFactory::iconPixmap("psi/microphone"));

	if ( !d->isPhoneCall()) {
		hb3->addMultiCellLayout(gl ,5,5, 0,3);

		gl->addWidget(l_dsp, 0,0);
		gl->addWidget(l_mic, 0,2);
		gl->addWidget(d->sl_dsp, 0,1);
		gl->addWidget(d->sl_mic, 0,3);
	}
	else {
	    QLabel * l_face = new QLabel(this);
	    //l_face->setPixmap( IconsetFactory::icon("psi/face") );
	    l_face->setFixedSize(64,64);

		hb3->addMultiCellWidget(l_face, 0,4, 0,0);

		hb3->addWidget(d->l_status, 0,1);
		hb3->addWidget(d->l_time, 1,1);

		hb3->addMultiCellLayout(gl ,5,5, 0,3);

		gl->addWidget(l_dsp, 0,0);
		gl->addWidget(l_mic, 0,2);
		gl->addWidget(d->sl_dsp, 0,1);
		gl->addWidget(d->sl_mic, 0,3);

		d->l_from->hide();

        d->l_number = new QLabel(this);
        d->l_number->setText ("Enter a phone number"); 
		hb3->addWidget(d->l_number, 6,1);
  
        d->l_format = new QLabel(this);
        d->l_format->setText ("Example: 00 + country code + area code + number"); 
		d->l_format->setFont( QFont( "helvetica", 7 ) );
        d->l_format->setPaletteForegroundColor(QColor("#555555"));
		hb3->addWidget(d->l_format, 8,1);

		d->le_number = new QLineEdit(this);
		d->le_number->setFrameStyle(QFrame::Box | QFrame::Plain);
		d->le_number->setLineWidth(1);
		d->le_number->setFont( QFont( "helvetica", 14, QFont::Bold ) );
		hb3->addWidget(d->le_number, 7,1);

		QGridLayout * kbd = new QGridLayout;
		hb3->addMultiCellLayout(kbd ,9,9, 0,3);

   		kbd->addItem(new QSpacerItem(1,1, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum), 0,0);

		// comix
		QPushButton * p1 = new QPushButton("1", this);
		QPushButton * p2 = new QPushButton("2", this);
		QPushButton * p3 = new QPushButton("3", this);
		QPushButton * p4 = new QPushButton("4", this);
		QPushButton * p5 = new QPushButton("5", this);
		QPushButton * p6 = new QPushButton("6", this);
		QPushButton * p7 = new QPushButton("7", this);
		QPushButton * p8 = new QPushButton("8", this);
		QPushButton * p9 = new QPushButton("9", this);
		QPushButton * pa = new QPushButton("*", this);
		QPushButton * p0 = new QPushButton("0", this);
        QPushButton * pd = new QPushButton("#", this);

		p1->setFlat(true);
		p2->setFlat(true);
		p3->setFlat(true);
		p4->setFlat(true);
		p5->setFlat(true);
		p6->setFlat(true);
		p7->setFlat(true);
		p8->setFlat(true);
		p9->setFlat(true);
		pa->setFlat(true);
		p0->setFlat(true);
		pd->setFlat(true);


		//p1->setPixmap(IconsetFactory::icon("psi/dialpad_button1"));
		//p2->setPixmap(IconsetFactory::icon("psi/dialpad_button2"));
		//p3->setPixmap(IconsetFactory::icon("psi/dialpad_button3"));
		//p4->setPixmap(IconsetFactory::icon("psi/dialpad_button4"));
		//p5->setPixmap(IconsetFactory::icon("psi/dialpad_button5"));
		//p6->setPixmap(IconsetFactory::icon("psi/dialpad_button6"));
		//p7->setPixmap(IconsetFactory::icon("psi/dialpad_button7"));
		//p8->setPixmap(IconsetFactory::icon("psi/dialpad_button8"));
		//p9->setPixmap(IconsetFactory::icon("psi/dialpad_button9"));
		//pa->setPixmap(IconsetFactory::icon("psi/dialpad_button10"));
		//p0->setPixmap(IconsetFactory::icon("psi/dialpad_button11"));
		//pd->setPixmap(IconsetFactory::icon("psi/dialpad_button12"));

		kbd->addWidget(p1, 1, 1);
		kbd->addWidget(p2, 1, 2);
		kbd->addWidget(p3, 1, 3);
		kbd->addWidget(p4, 2, 1);
		kbd->addWidget(p5, 2, 2);
		kbd->addWidget(p6, 2, 3);
		kbd->addWidget(p7, 3, 1);
		kbd->addWidget(p8, 3, 2);
		kbd->addWidget(p9, 3, 3);
		kbd->addWidget(pa, 4, 1);
		kbd->addWidget(p0, 4, 2);
		kbd->addWidget(pd, 4, 3);

		QSignalMapper * mapper = new QSignalMapper(this);
		mapper->setMapping( p0, "0" );
		mapper->setMapping( p1, "1" );
		mapper->setMapping( p2, "2" );
		mapper->setMapping( p3, "3" );
		mapper->setMapping( p4, "4" );
		mapper->setMapping( p5, "5" );
		mapper->setMapping( p6, "6" );
		mapper->setMapping( p7, "7" );
		mapper->setMapping( p8, "8" );
		mapper->setMapping( p9, "9" );
		mapper->setMapping( pa, "*" );
		mapper->setMapping( pd, "#" );

		connect(p0, SIGNAL(clicked()), mapper, SLOT(map()));
		connect(p1, SIGNAL(clicked()), mapper, SLOT(map()));
		connect(p2, SIGNAL(clicked()), mapper, SLOT(map()));
		connect(p3, SIGNAL(clicked()), mapper, SLOT(map()));
		connect(p4, SIGNAL(clicked()), mapper, SLOT(map()));
		connect(p5, SIGNAL(clicked()), mapper, SLOT(map()));
		connect(p6, SIGNAL(clicked()), mapper, SLOT(map()));
		connect(p7, SIGNAL(clicked()), mapper, SLOT(map()));
		connect(p8, SIGNAL(clicked()), mapper, SLOT(map()));
		connect(p9, SIGNAL(clicked()), mapper, SLOT(map()));
		connect(pa, SIGNAL(clicked()), mapper, SLOT(map()));
		connect(pd, SIGNAL(clicked()), mapper, SLOT(map()));

		connect(mapper, SIGNAL(mapped(const QString &)), this, SLOT(dtmfButton(const QString &)));

		kbd->addItem(new QSpacerItem(1,1, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum), 0,4);
    }

	d->layout->addLayout(hb3,   0,0);
	d->layout->addItem(new QSpacerItem(10,10,
	QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding), 1,0);
	QFrame * hl = new QFrame(this);
	hl->setFrameStyle(QFrame::HLine);
	hl->setFrameShadow(QFrame::Sunken);
	d->layout->addWidget(hl, 2,0);
	d->layout->addLayout(hb1,   3,0);
	resize( QSize(300, 200));

	d->answer = false;
	tb_accept->setFocus();
	tb_reject->setFocus();

	connect(voiceCaller, SIGNAL(accepted(const Jid&)),    SLOT(accepted(const Jid&)));
	connect(voiceCaller, SIGNAL(rejected(const Jid&)),    SLOT(rejected(const Jid&)));
	connect(voiceCaller, SIGNAL(terminated(const Jid&)),  SLOT(terminated(const Jid&)));
	connect(voiceCaller, SIGNAL(in_progress(const Jid&)), SLOT(in_progress(const Jid&)));

	// Buttons
	tb_accept->setEnabled(false);
	tb_reject->setEnabled(false);

	if ( d->isPhoneCall() ) {
		tb_accept->setEnabled(true);
		tb_accept->setTextLabel(tr("Call"));
		connect(tb_accept, SIGNAL(clicked()), this, SLOT(phoneCall()));
	}

	d->pa->dialogRegister(this, d->jid);
}

CallDlg::~CallDlg()
{
	//disconnect signals(was in finalize)
	disconnect(d->voiceCaller, SIGNAL(accepted(const Jid&)),    this, SLOT(accepted(const Jid&)));
	disconnect(d->voiceCaller, SIGNAL(rejected(const Jid&)),    this, SLOT(rejected(const Jid&)));
	disconnect(d->voiceCaller, SIGNAL(in_progress(const Jid&)), this, SLOT(in_progress(const Jid&)));
	disconnect(d->voiceCaller, SIGNAL(terminated(const Jid&)),  this, SLOT(terminated(const Jid&)));

    d->pa->dialogUnregister(this);
	delete d;
}

QSize CallDlg::defaultSize()
{
	return QSize(500, 450);
}
void CallDlg::resizeEvent(QResizeEvent *)
{
	//if(SJabbinOptions::instance()->keepSizes)
	//	SJabbinOptions::instance()->sizeCallDlg = e->size();
}

void CallDlg::closeEvent(QCloseEvent *e)
{
//    setStatus(Terminating);
//    d->voiceCaller->terminate(d->jid);
//    finalize();
//    close();
      if ( d->status == InProgress || 
             d->status == Calling || 
             d->status == Accepting || 
             d->status == Accepted || d->status == Incoming ) {	
		     e->ignore();
      }
      else {
             //setWFlags(getWFlags() | WDestructiveClose);
	         e->accept();
	  }
}

void CallDlg::showEvent( QShowEvent * )
{
	// grab the focus in order to use the keyboard as an alternative to dtmf buttons
	setFocus();
}



void CallDlg::keyPressEvent(QKeyEvent *e)
{
	//if(e->key() == Key_Escape) close();
	switch(e->key()) {
		case Qt::Key_Backspace:
			// FIXME: how to handle the backspace key?
			e->ignore();
			break;

		case Qt::Key_0:
		case Qt::Key_1:
		case Qt::Key_2:
		case Qt::Key_3:
		case Qt::Key_4:
		case Qt::Key_5:
		case Qt::Key_6:
		case Qt::Key_7:
		case Qt::Key_8:
		case Qt::Key_9:
		case Qt::Key_Asterisk:
		case '#':
			fprintf(stderr, "[CallDlg::keyPressEvent] %c\n", e->key());

			// send the proper dtmf tone
			dtmfButton(QString(QChar(e->ascii())));
			break;

		default:
			e->ignore();
	}
}

const Jid & CallDlg::jid() const
{
	return d->jid;
}

void CallDlg::setJid(const Jid &jid)
{
	if(!jid.compare(d->jid)) {
		d->pa->dialogUnregister(this);
		d->jid = jid;
		d->pa->dialogRegister(this, d->jid);
		updateContact(d->jid, false);

        //d->logItem.jid = jid.full();
	}
}

void CallDlg::updateContact(const Jid &, bool)
{

}

void CallDlg::optionsUpdate()
{

}

void CallDlg::call()
{
    setStatus(Calling);
    d->voiceCaller->call( d->jid );
}

void CallDlg::phoneCall()
{
    setStatus(Calling);
    d->voiceCaller->call( d->jid );
}

void CallDlg::incoming()
{
    setStatus(Incoming);
}

void CallDlg::accept_call()
{
    qDebug("CallDlg: accpet_call");
    setStatus(Accepting);
    d->voiceCaller->accept(d->jid);
}

void CallDlg::reject_call()
{
    //d->logItem.isAccepted = false;
    setStatus(Rejecting);
    d->voiceCaller->reject(d->jid);

    QDateTime::currentDateTime();

    finalize();
//  close();
}

void CallDlg::terminate_call()
{
    setStatus(Terminating);
    d->voiceCaller->terminate(d->jid);
    finalize();
//  close();
}

void CallDlg::detectedRemoteHost(QString , int)
{
}

void CallDlg::countDuration()
{
    d->sec += 1;
    d->l_time->setText(tr("Duration: %1 sec").arg(d->sec));
}

bool CallDlg::eventFilter( QObject *o, QEvent *e )
{
	if ( e->type() == QEvent::Timer) 
	{
		QTimerEvent *timer = (QTimerEvent *)e;
		if (d->secTimer)
		{
			//There will be _some_ working timer always while we are here
			//(for RTP timeouts for example) and if OUR timer doesn't work for ANY reason
			//we just use alternative way.
			//this may not work if there is NO timers active with period less than second
			if (d->timeSinceLastFire.elapsed()>3000)
			{
				if (d->timeSpent.elapsed()>1000)
				{
					qDebug("Alternative pseudo-timer will timeout NOW");
					d->timeSpent.restart();
					countDuration();
				}
			}
			if (timer->timerId()==d->secTimer->timerId())
			{
				//restart guard timer
				d->timeSinceLastFire.restart();
			}

		}
		return FALSE; //perform standard event processing
	} 
	else {
		// standard event processing
		return FALSE;
	}
}

void CallDlg::setStatus(CallStatus s)
{
    qDebug("state changed");

    disconnect(tb_accept, SIGNAL(clicked()), 0L, 0L);
    disconnect(tb_reject, SIGNAL(clicked()), 0L, 0L);

	int timer_id(0);
    d->status = s;
    switch (s) {
    case Calling:
        //d->logItem.direction = CallItem::Outgoing;
        qDebug("state Calling");
        d->l_status->setText(tr("Calling"));
        tb_accept->setEnabled(false);
        tb_reject->setEnabled(true);
        tb_accept->setTextLabel(tr("Call"));
        tb_reject->setTextLabel(tr("Cancel"));

        connect(tb_reject, SIGNAL(clicked()), SLOT(terminate_call()));

        //SoundPlayer::instance()->playContinuosSound(SJabbinOptions::instance()->onevent[eIncomingCall], 60*1000);
        break;

    case Accepting:
        //d->logItem.isAccepted = true;
        qDebug("state Accepting");
        d->l_status->setText(tr("Accepting"));
        tb_accept->setEnabled(false);
        tb_reject->setEnabled(true);
        tb_reject->setTextLabel(tr("Hangup"));

        connect(tb_reject, SIGNAL(clicked()), SLOT(terminate_call()));

        SoundPlayer::instance()->stop();
        break;

    case Rejecting:
        //d->logItem.isAccepted = false;
        //CallsLog::instance()->addItem( d->logItem );
        qDebug("state Rejecting");
        d->l_status->setText(tr("Rejecting"));
        tb_accept->setEnabled(false);
        tb_reject->setEnabled(false);

        SoundPlayer::instance()->stop();
        break;

    case Terminating:
        //d->logItem.duration = d->sec;
        //CallsLog::instance()->addItem( d->logItem );
        qDebug("state Terminating");
        d->l_status->setText(tr("Hanging up"));
        tb_accept->setEnabled(false);
        tb_reject->setEnabled(false);

        SoundPlayer::instance()->stop();
        break;

    case Accepted:
        //d->logItem.isAccepted = true;
        qDebug("state Accepted");
        d->l_status->setText(tr("Accepted"));
        tb_accept->setEnabled(false);
        tb_reject->setEnabled(false);
		//correct start time
		d->sec = 0;
		d->l_time = new QLabel(tr("Duration:"), this);

        connect(tb_reject, SIGNAL(clicked()), SLOT(terminate_call()));

        SoundPlayer::instance()->stop();
        break;

    case Rejected:
        //d->logItem.isAccepted = false;
        //CallsLog::instance()->addItem( d->logItem );
        qDebug("state Rejected");
        d->l_status->setText(tr("Rejected"));
        tb_accept->setEnabled(false);
        tb_reject->setEnabled(false);

        //QTimer::singleShot(100, this, SLOT(close()));

        if ( d->isPhoneCall() ) {
            tb_accept->setEnabled(true);
            tb_accept->setTextLabel(tr("Call"));
            connect(tb_accept, SIGNAL(clicked()), SLOT(phoneCall()));
        }

        SoundPlayer::instance()->stop();
        break;

    case InProgress:
        //d->logItem.callTime = QDateTime::currentDateTime();
        qDebug("state InProgress");
        d->l_status->setText(tr("Connected"));
        tb_accept->setEnabled(false);
        tb_reject->setEnabled(true);
        tb_reject->setTextLabel(tr("Hangup"));

        d->secTimer = new QTimer(this);
		connect(d->secTimer, SIGNAL(timeout()),this,SLOT(countDuration()));
		d->secTimer->start(1000);
		/*if (timer_id == 0)
		{
			qDebug("Failed to start call duration timer");
		}
		else
		{
			qDebug(QString("call duration timer started.Id %1").arg(timer_id));
		}*/
		//install our workaround for situations where d->secTimer simple doesn't work(problem with moc's understanding of 
		//template classes???)
		d->timeSpent.start();
		d->timeSinceLastFire.start();
		qApp->installEventFilter(this);

        if ( d->isPhoneCall() ) {
            sendDTMF();
        }

        connect(tb_reject, SIGNAL(clicked()), SLOT(terminate_call()));
        break;

    case Terminated:
        //d->logItem.duration = d->sec;
        //CallsLog::instance()->addItem( d->logItem );
        qDebug("state Terminated");
        d->l_status->setText(tr("Terminated"));

        tb_accept->setEnabled(false);
        tb_reject->setEnabled(false);
        tb_accept->setTextLabel(tr("Call"));

        if (d->secTimer)
            d->secTimer->stop();

		//deactivate our workaround
		qApp->removeEventFilter(this);

        //QTimer::singleShot(100, this, SLOT(close()));

        if ( d->isPhoneCall() ) {
            tb_accept->setEnabled(true);
            tb_accept->setTextLabel(tr("Call"));
            connect(tb_accept, SIGNAL(clicked()), SLOT(phoneCall()));
        }

        SoundPlayer::instance()->stop();
        break;

    case Incoming:
        //d->logItem.direction = CallItem::Incoming;
        qDebug("state Incoming");
        d->l_status->setText(tr("Incoming Call"));

        tb_accept->setEnabled(true);
        tb_reject->setEnabled(true);
        tb_accept->setTextLabel(tr("Accept"));
        tb_reject->setTextLabel(tr("Reject"));

        connect(tb_accept, SIGNAL(clicked()), SLOT(accept_call()));
        connect(tb_reject, SIGNAL(clicked()), SLOT(reject_call()));

        if ( d->isPhoneCall() ) {
            tb_accept->setEnabled(false);
            tb_reject->setEnabled(false);
            QTimer::singleShot(0, this, SLOT(accept_call()));
        }
        else
            //SoundPlayer::instance()->playContinuosSound(SJabbinOptions::instance()->onevent[eIncomingCall], 60*1000);

        break;

    default:
            break;
    }
}

void CallDlg::dspValueChanged(int level)
{
    call_dlg_dsp_level = level;
    //SJabbinOptions::instance()->dsp_level = level;
}

void CallDlg::micValueChanged(int level)
{
    call_dlg_mic_level = level;
    //SJabbinOptions::instance()->mic_level = level;
}

void CallDlg::accepted(const Jid & j)
{
    if (d->jid.compare(j)) {
        setStatus(Accepted);
		//activate timer for duration
	}
}

void CallDlg::rejected(const Jid & j)
{
    if (d->jid.compare(j)) {
        setStatus(Rejected);
        finalize();
    }
}

void CallDlg::terminated(const Jid & j)
{
    if (d->jid.compare(j)) {
        setStatus(Terminated);
        finalize();
    }
}

void CallDlg::in_progress(const Jid & j)
{
    if (d->jid.compare(j)) {
        setStatus(InProgress);
    }
}

void CallDlg::finalize()
{
    // Close connection
    if (d->status == Incoming) {
        reject_call();
    }
    else if (d->status == InProgress ||
             d->status == Calling ||
             d->status == Accepting ||
             d->status == Accepted) {
        terminate_call();
    }

    // Disconnect signals
	//disconnect(d->voiceCaller, SIGNAL(accepted(const Jid&)),    this, SLOT(accepted(const Jid&)));
    //disconnect(d->voiceCaller, SIGNAL(rejected(const Jid&)),    this, SLOT(rejected(const Jid&)));
    //disconnect(d->voiceCaller, SIGNAL(in_progress(const Jid&)), this, SLOT(in_progress(const Jid&)));
    //disconnect(d->voiceCaller, SIGNAL(terminated(const Jid&)),  this, SLOT(terminated(const Jid&)));
}

void CallDlg::sendDTMF()
{
    if ( !d->voiceCaller ) return;

    QString dtmf = d->le_number->text();
    JingleVoiceCaller * caller = (JingleVoiceCaller *)d->voiceCaller;
    caller->sendDTMF( d->jid, dtmf );
}

void CallDlg::dtmfButton(const QString & button)
{
    QString dtmf = d->le_number->text();
    d->le_number->setText( dtmf + button );
}

#define BGCOLOR "#FFFFFF"

class SoundStreamView::Private
{
public:
    QPainter * p;
    QPixmap buf;
    QVector<int> ba;
    QVector<int> ba_max;
    QColor col;
    QTimer * timer;
    int maxshort;
};

SoundStreamView::SoundStreamView(QColor color, QWidget * parent, const char * name)
:QWidget(parent, name)
{
    d = new Private;
    d->p = new QPainter();
    d->col = color;
    setBackgroundMode(Qt::NoBackground);
    d->timer = new QTimer(this);
    connect(d->timer, SIGNAL(timeout()), this, SLOT(update()));
    d->timer->start(100);

    short ss;
    d->maxshort = 1;
    int sss = sizeof(ss);
    for (int j=0;j<sss;j++)
        d->maxshort *= 256;
    d->maxshort /= 2;

}

SoundStreamView::~SoundStreamView()
{
    delete d;
}

void SoundStreamView::paintEvent(QPaintEvent * pe)
{
    if (!d->buf.isNull())
    {
        d->p->begin(this);
        d->p->drawPixmap(0, 0, d->buf);
        d->p->end();
    }
}

void SoundStreamView::resizeEvent(QResizeEvent * re)
{
    d->buf.resize(re->size());
    uint old_size = d->ba.size();
    d->ba.resize(re->size().width());
    d->ba_max.resize(re->size().width());
    if (old_size < d->ba.size()) {
        uint i;
        for (i=old_size;i<d->ba.size();i++) {
            d->ba.replace(i, 0);
            d->ba_max.replace(i, 0);
        }
    }
    prepareBuffer();
}

void SoundStreamView::prepareBuffer()
{
    d->buf.fill(QColor(BGCOLOR));
    d->p->begin(&(d->buf));
    uint i;
    for (i=0;i<d->ba.size();i++) {
        paintLine(d->p, i);
    }
    d->p->end();
}

void SoundStreamView::scrollBuffer()
{
    QPixmap buf2;
    buf2.resize(d->buf.size());
    buf2.fill(QColor(BGCOLOR));

    copyBlt(&buf2, 1,0, &(d->buf), 0,0, buf2.width()-1, buf2.height());
    d->p->begin(&(buf2));
    paintLine(d->p, 0);
    d->p->end();
    copyBlt(&(d->buf), 0,0, &buf2, 0,0, buf2.width(), buf2.height());
}

void SoundStreamView::paintLine(QPainter * p, int i)
{
    int mx = height()/2;
    int nx = d->ba[i]*mx/d->maxshort;
    d->p->setPen(d->col);
    d->p->drawLine(i, height()-nx-mx, i, height()-mx+nx);

    int c, c_r, c_g, c_b;

    int cmin = d->ba[i];
    int cmax = d->ba_max[i];
    if (cmin >= cmax) return;

    int cmin_r = d->col.red();
    int cmin_g = d->col.green();
    int cmin_b = d->col.blue();

    int cmax_r = QColor(BGCOLOR).red();
    int cmax_g = QColor(BGCOLOR).green();
    int cmax_b = QColor(BGCOLOR).blue();

    int prev_nx = nx;
    for (c=cmin;c<=cmax;c++) {
        int nx = c * mx /d->maxshort;
        if (nx == prev_nx) continue;
        prev_nx = nx;

        c_r = cmin_r + (cmax_r - cmin_r)*(c-cmin)/(cmax-cmin);
        c_g = cmin_g + (cmax_g - cmin_g)*(c-cmin)/(cmax-cmin);
        c_b = cmin_b + (cmax_b - cmin_b)*(c-cmin)/(cmax-cmin);

        QColor col(c_r, c_g, c_b);
        d->p->setPen(col);
        d->p->drawPoint(i, height()-nx-mx);
        d->p->drawPoint(i, height()-mx+nx);
    }
    d->p->setPen(d->col);
    d->p->drawPoint(i, height()-mx);
}

void SoundStreamView::tick(int mid, int max)
{
    int i;
    for (i=d->ba.size()-1-1;i>=0;i--) {
        d->ba[i+1] = d->ba[i];
        d->ba_max[i+1] = d->ba_max[i];
    }
    d->ba[0] = abs(mid);
    d->ba_max[0] = abs(max);
    scrollBuffer();
}

void SoundStreamView::clean()
{
    uint i;
    for (i=0;i<d->ba.size();i++) {
        d->ba.replace(i, 0);
        d->ba_max.replace(i, 0);
    }
}