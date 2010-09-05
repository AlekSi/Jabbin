INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
DEFINES += YAPSI
# DEFINES += YAPSI_DEV
# DEFINES += DEFAULT_XMLCONSOLE
# DEFINES += YAPSI_NO_STYLESHEETS
# DEFINES += YAPSI_STRESSTEST_ACCOUNTS
# DEFINES += YAPSI_YARU_FRIENDS

# DEFINES += YAPSI_FANCY_DELETE_CONFIRMATIONS
# FORMS += \
# 	$$PWD/yamulticontactconfirmationtiplabel.ui
# HEADERS += \
# 	$$PWD/yamulticontactconfirmationtiplabel.h \
# 	$$PWD/yamulticontactconfirmationtooltip.h
# SOURCES += \
# 	$$PWD/yamulticontactconfirmationtiplabel.cpp \
# 	$$PWD/yamulticontactconfirmationtooltip.cpp

DEFINES += USE_GENERAL_CONTACT_GROUP

include($$PWD/syntaxhighlighters/syntaxhighlighters.pri)
include($$PWD/../slickwidgets/slickwidgets.pri)
include($$PWD/../smoothscrollbar/smoothscrollbar.pri)
include($$PWD/../animationhelpers/animationhelpers.pri)

# there's also a section in src.pro
yapsi_activex_server {
	DEFINES += YAPSI_ACTIVEX_SERVER

	HEADERS += \
		$$PWD/yapsiserver.h \
		$$PWD/yaonline.h \
		$$PWD/ycuapiwrapper.h

	SOURCES += \
		$$PWD/yapsiserver.cpp \
		$$PWD/yaonline.cpp \
		$$PWD/ycuapiwrapper.cpp
}

debug {
	MODELTEST_PRI = $$PWD/../../../../modeltest/modeltest.pri
	exists($$MODELTEST_PRI) {
		include($$MODELTEST_PRI)
		DEFINES += MODELTEST
	}
}

HEADERS += \
	$$PWD/yaprofile.h \
	$$PWD/yarostertooltip.h \
	$$PWD/yachattooltip.h \
	$$PWD/yachattiplabel.h \
	$$PWD/yarostertiplabel.h \
	$$PWD/yamainwin.h \
	$$PWD/yaroster.h \
	$$PWD/yachatdlg.h \
	$$PWD/yatrayicon.h \
	$$PWD/yaeventnotifier.h \
	$$PWD/yatabbednotifier.h \
	$$PWD/yaloginpage.h \
	$$PWD/yacontactlistmodel.h \
	$$PWD/yacontactlistmodelselection.h \
	$$PWD/fakegroupcontact.h \
	$$PWD/yainformersmodel.h \
	$$PWD/yacontactlistfriendsmodel.h \
	$$PWD/yacontactlistcontactsmodel.h \
	$$PWD/yacommon.h \
	$$PWD/yastyle.h \
	$$PWD/yatoster.h \
	$$PWD/yapopupnotification.h \
	$$PWD/jabbinabout.h \
	$$PWD/yapreferences.h \
	$$PWD/yaprivacymanager.h \
	$$PWD/yaipc.h \
	$$PWD/yaremoveconfirmationmessagebox.h \
	$$PWD/yatoastercentral.h \
	$$PWD/yadayuse.h \
	$$PWD/yalicense.h

SOURCES += \
	$$PWD/yaprofile.cpp \
	$$PWD/yarostertooltip.cpp \
	$$PWD/yachattooltip.cpp \
	$$PWD/yachattiplabel.cpp \
	$$PWD/yarostertiplabel.cpp \
	$$PWD/yamainwin.cpp \
	$$PWD/yaroster.cpp \
	$$PWD/yachatdlg.cpp \
	$$PWD/yatrayicon.cpp \
	$$PWD/yaeventnotifier.cpp \
	$$PWD/yatabbednotifier.cpp \
	$$PWD/yaloginpage.cpp \
	$$PWD/yacontactlistmodel.cpp \
	$$PWD/yacontactlistmodelselection.cpp \
	$$PWD/fakegroupcontact.cpp \
	$$PWD/yainformersmodel.cpp \
	$$PWD/yacontactlistfriendsmodel.cpp \
	$$PWD/yacontactlistcontactsmodel.cpp \
	$$PWD/yacommon.cpp \
	$$PWD/yastyle.cpp \
	$$PWD/yatoster.cpp \
	$$PWD/yapopupnotification.cpp \
	$$PWD/jabbinabout.cpp \
	$$PWD/yapreferences.cpp \
	$$PWD/yaprivacymanager.cpp \
	$$PWD/yaipc.cpp \
	$$PWD/yaremoveconfirmationmessagebox.cpp \
	$$PWD/yatoastercentral.cpp \
	$$PWD/yadayuse.cpp \
	$$PWD/yalicense.cpp

RESOURCES += \
	$$PWD/../customwidgets/customwidgets.qrc \
	$$PWD/../services/services.qrc \
	$$PWD/yastuff.qrc \
	$$PWD/yaiconsets.qrc \
	$$PWD/../../../iconsets/custom/customicons.qrc

FORMS += \
	$$PWD/yaloginpage.ui \
	$$PWD/yamainwindow.ui \
	$$PWD/yachatdialog.ui \
	$$PWD/yarostertiplabel.ui \
	$$PWD/jabbinabout.ui \
	$$PWD/yapreferences.ui \
	$$PWD/yalicense.ui

HEADERS -= \
	$$PWD/../../accountmodifydlg.h

SOURCES -= \
	$$PWD/../../accountmodifydlg.cpp

FORMS -= \
	$$PWD/../../accountmodify.ui
