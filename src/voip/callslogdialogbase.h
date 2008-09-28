#ifndef CALLSLOGDIALOGBASE_H
#define CALLSLOGDIALOGBASE_H

#include <qvariant.h>


#include <Qt3Support/Q3Header>
#include <Qt3Support/Q3ListView>
#include <Qt3Support/Q3MimeSourceFactory>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_CallsLogDialogBase
{
public:
    QGridLayout *gridLayout;
    Q3ListView *listLogs;
    QVBoxLayout *vboxLayout;
    QCheckBox *checkAccepted;
    QLabel *textLabel1;
    QComboBox *combocalls;
    QFrame *line2;
    QPushButton *closeButton;
    QSpacerItem *spacer3;

    void setupUi(QDialog *CallsLogDialogBase)
    {
    if (CallsLogDialogBase->objectName().isEmpty())
        CallsLogDialogBase->setObjectName(QString::fromUtf8("CallsLogDialogBase"));
    CallsLogDialogBase->resize(590, 488);
    gridLayout = new QGridLayout(CallsLogDialogBase);
    gridLayout->setSpacing(6);
    gridLayout->setMargin(11);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    listLogs = new Q3ListView(CallsLogDialogBase);
    listLogs->addColumn(QApplication::translate("CallsLogDialogBase", "Call", 0, QApplication::UnicodeUTF8));
    listLogs->header()->setClickEnabled(true, listLogs->header()->count() - 1);
    listLogs->header()->setResizeEnabled(true, listLogs->header()->count() - 1);
    listLogs->setObjectName(QString::fromUtf8("listLogs"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(7), static_cast<QSizePolicy::Policy>(7));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(10);
    sizePolicy.setHeightForWidth(listLogs->sizePolicy().hasHeightForWidth());
    listLogs->setSizePolicy(sizePolicy);

    gridLayout->addWidget(listLogs, 0, 0, 1, 2);

    vboxLayout = new QVBoxLayout();
    vboxLayout->setSpacing(6);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    checkAccepted = new QCheckBox(CallsLogDialogBase);
    checkAccepted->setObjectName(QString::fromUtf8("checkAccepted"));

    vboxLayout->addWidget(checkAccepted);

    textLabel1 = new QLabel(CallsLogDialogBase);
    textLabel1->setObjectName(QString::fromUtf8("textLabel1"));
    textLabel1->setWordWrap(false);

    vboxLayout->addWidget(textLabel1);

    combocalls = new QComboBox(CallsLogDialogBase);
    combocalls->setObjectName(QString::fromUtf8("combocalls"));
    QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(0));
    sizePolicy1.setHorizontalStretch(3);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(combocalls->sizePolicy().hasHeightForWidth());
    combocalls->setSizePolicy(sizePolicy1);

    vboxLayout->addWidget(combocalls);


    gridLayout->addLayout(vboxLayout, 1, 0, 1, 2);

    line2 = new QFrame(CallsLogDialogBase);
    line2->setObjectName(QString::fromUtf8("line2"));
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Sunken);

    gridLayout->addWidget(line2, 2, 0, 2, 2);

    closeButton = new QPushButton(CallsLogDialogBase);
    closeButton->setObjectName(QString::fromUtf8("closeButton"));

    gridLayout->addWidget(closeButton, 3, 1, 2, 1);

    spacer3 = new QSpacerItem(411, 21, QSizePolicy::Expanding, QSizePolicy::Minimum);

    gridLayout->addItem(spacer3, 4, 0, 1, 1);


    retranslateUi(CallsLogDialogBase);
    QObject::connect(closeButton, SIGNAL(clicked()), CallsLogDialogBase, SLOT(close()));
    QObject::connect(checkAccepted, SIGNAL(toggled(bool)), CallsLogDialogBase, SLOT(showOnlyAccepted(bool)));
    QObject::connect(combocalls, SIGNAL(activated(QString)), CallsLogDialogBase, SLOT(showCallsWith(QString)));

    QMetaObject::connectSlotsByName(CallsLogDialogBase);
    } // setupUi

    void retranslateUi(QDialog *CallsLogDialogBase)
    {
    CallsLogDialogBase->setWindowTitle(QApplication::translate("CallsLogDialogBase", "Calls log", 0, QApplication::UnicodeUTF8));
    listLogs->header()->setLabel(0, QApplication::translate("CallsLogDialogBase", "Call", 0, QApplication::UnicodeUTF8));
    checkAccepted->setText(QApplication::translate("CallsLogDialogBase", "Show only accepted calls", 0, QApplication::UnicodeUTF8));
    textLabel1->setText(QApplication::translate("CallsLogDialogBase", "Show:", 0, QApplication::UnicodeUTF8));
    combocalls->clear();
    combocalls->insertItems(0, QStringList()
     << QApplication::translate("CallsLogDialogBase", "All calls", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("CallsLogDialogBase", "Missed calls", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("CallsLogDialogBase", "Outgoing calls", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("CallsLogDialogBase", "Incoming calls", 0, QApplication::UnicodeUTF8)
    );
    closeButton->setText(QApplication::translate("CallsLogDialogBase", "Close", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(CallsLogDialogBase);
    } // retranslateUi

};

namespace Ui {
    class CallsLogDialogBase: public Ui_CallsLogDialogBase {};
} // namespace Ui

QT_END_NAMESPACE

class CallsLogDialogBase : public QDialog, public Ui::CallsLogDialogBase
{
    Q_OBJECT

public:
    CallsLogDialogBase(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~CallsLogDialogBase();

public slots:
    virtual void showOnlyAccepted(bool);
    virtual void showCallsWith( const QString& );

protected slots:
    virtual void languageChange();

};

#endif // CALLSLOGDIALOGBASE_H
