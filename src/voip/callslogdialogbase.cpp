//
// ivan: This file is no longer used
//
#error This file should not be used: callslogdialogbase.cpp

#include "callslogdialogbase.h"

#include <qvariant.h>
/*
 *  Constructs a CallsLogDialogBase as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
CallsLogDialogBase::CallsLogDialogBase(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);

}

/*
 *  Destroys the object and frees any allocated resources
 */
CallsLogDialogBase::~CallsLogDialogBase()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void CallsLogDialogBase::languageChange()
{
    retranslateUi(this);
}

void CallsLogDialogBase::showOnlyAccepted(bool)
{
    qWarning("CallsLogDialogBase::showOnlyAccepted(bool): Not implemented yet");
}

void CallsLogDialogBase::showCallsWith( const QString&)
{
    qWarning("CallsLogDialogBase::showCallsWith( const QString&): Not implemented yet");
}

