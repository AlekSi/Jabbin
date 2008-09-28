/*
 * calldlg.h - dialog for handling calls
 * Copyright (C) 2005 Oleksandr Yakovlyev <me@yshurik.kiev.ua>
 */

#ifndef CALLDLG_H
#define CALLDLG_H

#include "im.h"
#include "advwidget.h"



using namespace XMPP;

class PsiAccount;
class VoiceCaller;
class QToolButton;

class CallDlg : public AdvancedWidget<QWidget>
{
	Q_OBJECT
public:
    enum CallStatus {
        Default,
        Calling, Accepting, Rejecting, Terminating,
        Accepted, Rejected, InProgress, Terminated, Incoming
    };
	CallDlg(const Jid &, PsiAccount *, VoiceCaller *);
	~CallDlg();

	const Jid & jid() const;
	void setJid(const Jid &);
	static QSize defaultSize();

public slots:
    void call();
	void phoneCall();
    void incoming();
    void accept_call();
    void reject_call();
    void terminate_call();

	void optionsUpdate();
    void detectedRemoteHost(QString host, int port);
	void updateContact(const Jid &, bool);


private slots:
	void countDuration();
	void dspValueChanged(int level);
    void micValueChanged(int level);

    void accepted(const Jid &);
    void rejected(const Jid &);
    void terminated(const Jid &);
    void in_progress(const Jid &);

    void finalize();

    void sendDTMF();
    void dtmfButton(const QString &);

protected:

    void closeEvent(QCloseEvent *);

	void showEvent( QShowEvent *ev );
	void keyPressEvent(QKeyEvent *);

    void resizeEvent(QResizeEvent *);

    void setStatus(CallStatus);

protected:
	bool eventFilter( QObject *o, QEvent *e );

public:
    QToolButton * tb_accept;
    QToolButton * tb_reject;

public:
	class Private;

private:
	Private *d;

};

class SoundStreamView : public QWidget
{
    Q_OBJECT
public:
    SoundStreamView(QColor color, QWidget * parent, const char * name = 0L);
    ~SoundStreamView();

public slots:
    void tick(int, int);
    void clean();

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
    void paintLine(QPainter * p, int i);

private slots:
    void prepareBuffer();
    void scrollBuffer();

private:
    class Private;
    Private * d;
};


#endif // CALLDLG_H

