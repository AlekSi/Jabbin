#include "psitiplabel.h"

#include <QApplication>
#include <QAbstractTextDocumentLayout>
#include <QTextFrame>
#include <QStyle>
#include <QStyleOption>
#include <QStylePainter>
#include <QTimer>
#include <QToolTip>
#include <QTextEdit>

#include "psirichtext.h"

PsiTipLabel *PsiTipLabel::instance_ = 0;

PsiTipLabel* PsiTipLabel::instance()
{
	return instance_;
}

PsiTipLabel::PsiTipLabel(QWidget* parent, bool isHelperTipLabel)
	: QFrame(parent, Qt::ToolTip)
	, doc(0)
	, isHelperTipLabel_(isHelperTipLabel)
{
	if (!this->isHelperTipLabel()) {
		delete instance_;
		instance_ = this;
	}
}

bool PsiTipLabel::isHelperTipLabel() const
{
	return isHelperTipLabel_;
}

void PsiTipLabel::init(const QString& text)
{
	setText(text);
	initUi();
	resize(sizeHint());
	qApp->installEventFilter(this);
	startHideTimer();
	setWindowOpacity(style()->styleHint(QStyle::SH_ToolTipLabel_Opacity, 0, this) / 255.0);
	setPalette(QToolTip::palette());
}

void PsiTipLabel::setText(const QString& text)
{
	theText_ = text;
	isRichText = false;
	if (doc) {
		if (Qt::mightBeRichText(theText_)) {
			isRichText = true;
			PsiRichText::install(doc);
			PsiRichText::setText(doc, theText_);
		}
		else {
			doc->setPlainText(theText_);
		}
	}
}

void PsiTipLabel::initUi()
{
	margin = 1 + style()->pixelMetric(QStyle::PM_ToolTipLabelFrameWidth, 0, this);
	setFrameStyle(QFrame::NoFrame);

	// doc = new QTextDocument(this);
	// QTextDocumentLayout is private in Qt4
	// and it's impossible to set wrapping mode directly.
	// So we create this QTextEdit instance and use its QTextDocument,
	// just because QTextEdit can set the wrapping mode.
	// Yes, this is crazy...
	QTextEdit *edit = new QTextEdit(this);
	edit->hide();
	edit->setWordWrapMode(QTextOption::WordWrap);
	doc = edit->document();
	doc->setUndoRedoEnabled(false);
	doc->setDefaultFont(font());

	ensurePolished();
	setText(theText_);
}

void PsiTipLabel::startHideTimer()
{
	hideTimer.start(10000, this);
}

QString PsiTipLabel::theText() const
{
	return theText_;
}
/*
QSize PsiTipLabel::sizeForWidth(int w) const
{
	QRect br;

	int hextra = 2 * margin;
	int vextra = hextra;

	if (isRichText) {
		hextra = 1;
		vextra = 1;
	}

	PsiRichText::ensureTextLayouted(doc, w);
	const qreal oldTextWidth = doc->textWidth();

	doc->adjustSize();
	br = QRect(QPoint(0, 0), doc->size().toSize());
	doc->setTextWidth(oldTextWidth);

	QFontMetrics fm(font());
	QSize extra(hextra + 1, vextra);

	// Make it look good with the default ToolTip font on Mac, which has a small descent.
	if (fm.descent() == 2 && fm.ascent() >= 11)
		vextra++;

	const QSize contentsSize(br.width() + hextra, br.height() + vextra);
	return contentsSize;
}
*/
QSize PsiTipLabel::sizeHint() const
{
	QTextFrameFormat fmt = doc->rootFrame()->frameFormat();
	fmt.setMargin(0);
	doc->rootFrame()->setFrameFormat(fmt);
	// PsiRichText::ensureTextLayouted(doc, -1);

	doc->adjustSize();
	// br = QRect(QPoint(0, 0), doc->size().toSize());
	// this way helps to fight empty space on the right:
	QSize docSize = QSize(doc->idealWidth(), doc->size().toSize().height());

	QFontMetrics fm(font());
	QSize extra(2*margin + 2, 2*margin + 1);	// "+" for tip's frame
	// Make it look good with the default ToolTip font on Mac, which has a small descent.
	if (fm.descent() == 2 && fm.ascent() >= 11)
		++extra.rheight();

	return docSize + extra;
}

QSize PsiTipLabel::minimumSizeHint() const
{
	return sizeHint();
	// qWarning("PsiTipLabel::minimumSizeHint");
	// ensurePolished();
	// QSize sh = sizeForWidth(-1);
	// QSize msh(-1, -1);
	//
	// msh.rheight() = sizeForWidth(QWIDGETSIZE_MAX).height(); // height for one line
	// msh.rwidth()  = sizeForWidth(0).width(); // wrap ? size of biggest word : min doc size
	// if (sh.height() < msh.height())
	// 	msh.rheight() = sh.height();
	//
	// return msh;
}

void PsiTipLabel::paintEvent(QPaintEvent *)
{
	QStylePainter p(this);
	QStyleOptionFrame opt;
	opt.init(this);
	p.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
	p.end();

	// stolen from QLabel::paintEvent
	QPainter painter(this);
	drawFrame(&painter);
	QRect cr = contentsRect();
	cr.adjust(margin, margin, -margin, -margin);

	PsiRichText::ensureTextLayouted(doc, width() - 2*margin);
	QAbstractTextDocumentLayout *layout = doc->documentLayout();
	// QRect lr = rect();
	QRect lr = cr;

	QAbstractTextDocumentLayout::PaintContext context;

	// Adjust the palette
	context.palette = palette();
	if (foregroundRole() != QPalette::Text && isEnabled())
		context.palette.setColor(QPalette::Text, context.palette.color(foregroundRole()));

	painter.save();
	painter.translate(lr.x() + 1, lr.y() + 1);
	painter.setClipRect(lr.translated(-lr.x() - 1, -lr.y() - 1));
	layout->draw(&painter, context);
	painter.restore();
}

PsiTipLabel::~PsiTipLabel()
{
	if (!isHelperTipLabel()) {
		instance_ = 0;
	}
}

void PsiTipLabel::hideTip()
{
	hide();
	// timer based deletion to prevent animation
	deleteTimer.start(250, this);
}

void PsiTipLabel::enterEvent(QEvent*)
{
	hideTip();
}

void PsiTipLabel::timerEvent(QTimerEvent *e)
{
	if (e->timerId() == hideTimer.timerId())
		hideTip();
	else if (e->timerId() == deleteTimer.timerId())
		deleteLater();
}

bool PsiTipLabel::eventFilter(QObject* obj, QEvent* e)
{
	Q_UNUSED(obj);
	switch (e->type()) {
	case QEvent::KeyPress:
	case QEvent::KeyRelease: {
		int key = static_cast<QKeyEvent *>(e)->key();
		Qt::KeyboardModifiers mody = static_cast<QKeyEvent *>(e)->modifiers();

		if ((mody & Qt::KeyboardModifierMask)
		    || (key == Qt::Key_Shift || key == Qt::Key_Control
		    || key == Qt::Key_Alt || key == Qt::Key_Meta))
			break;

		hideTip();
		return true;
	}
	case QEvent::Leave:
	case QEvent::WindowActivate:
	case QEvent::WindowDeactivate:
	case QEvent::MouseButtonPress:
	case QEvent::MouseButtonRelease:
	case QEvent::MouseButtonDblClick:
	case QEvent::FocusIn:
	case QEvent::FocusOut:
	case QEvent::Wheel:
		hideTip();
	default:
		;
	}
	return false;
}
