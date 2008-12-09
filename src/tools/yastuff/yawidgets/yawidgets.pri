INCLUDEPATH += $$PWD $$PWD/private $$PWD/settingsbuttontest
DEPENDPATH  += $$PWD $$PWD/private $$PWD/settingsbuttontest

INCLUDEPATH += $$PWD/newselfavatar
DEPENDPATH += $$PWD/newselfavatar
INCLUDEPATH += $$PWD/../../customwidgets
DEPENDPATH += $$PWD/../../customwidgets
SOURCES += \
	$$PWD/newselfavatar/yaselfmood.cpp \
	$$PWD/newselfavatar/yaselfmoodaction.cpp \
	$$PWD/../../customwidgets/selfmood.cpp \
	$$PWD/../../customwidgets/selfavatar.cpp \
	$$PWD/../../customwidgets/generic/advancedlineedit.cpp \
	$$PWD/../../customwidgets/generic/advancedtabwidget.cpp \
	$$PWD/../../customwidgets/generic/advancedtabbar.cpp \
	$$PWD/../../customwidgets/generic/dialpad.cpp \
	$$PWD/../../customwidgets/generic/notificationpanel.cpp \
	$$PWD/../../customwidgets/calldialog.cpp \
	$$PWD/../../customwidgets/optionsdialog.cpp \
	$$PWD/../../customwidgets/models/callhistory.cpp \
	$$PWD/../../customwidgets/models/phonebook.cpp \
	$$PWD/../../customwidgets/generic/widgetexpander.cpp \
	$$PWD/../../customwidgets/generic/avatarbutton.cpp \
	$$PWD/../../customwidgets/generic/customwidgetscommon.cpp

HEADERS += \
	$$PWD/newselfavatar/yaselfmood.h \
	$$PWD/newselfavatar/yaselfmoodaction.h \
	$$PWD/../../customwidgets/selfmood.h \
	$$PWD/../../customwidgets/selfavatar.h \
	$$PWD/../../customwidgets/generic/advancedlineedit.h \
	$$PWD/../../customwidgets/generic/advancedtabwidget.h \
	$$PWD/../../customwidgets/generic/advancedtabbar.h \
	$$PWD/../../customwidgets/generic/dialpad.h \
	$$PWD/../../customwidgets/generic/notificationpanel.h \
	$$PWD/../../customwidgets/optionsdialog.h \
	$$PWD/../../customwidgets/optionsdialog_p.h \
	$$PWD/../../customwidgets/calldialog.h \
	$$PWD/../../customwidgets/calldialog_p.h \
	$$PWD/../../customwidgets/models/callhistory.h \
	$$PWD/../../customwidgets/models/callhistory_p.h \
	$$PWD/../../customwidgets/models/phonebook.h \
	$$PWD/../../customwidgets/models/phonebook_p.h \
	$$PWD/../../customwidgets/generic/widgetexpander.h \
	$$PWD/../../customwidgets/generic/avatarbutton.h \
	$$PWD/../../customwidgets/generic/customwidgetscommon.h

INTERFACES += \
	$$PWD/../../customwidgets/generic/dialpad_base.ui \
	$$PWD/../../customwidgets/generic/notificationpanel_base.ui \
	$$PWD/../../customwidgets/calldialog_base.ui \
	$$PWD/../../customwidgets/optionsdialog_base.ui \
	$$PWD/../../customwidgets/models/callhistoryitemdelegate_base.ui \
	$$PWD/../../customwidgets/models/phonebookitemdelegate_base.ui \
	$$PWD/../../customwidgets/models/phonebookedit_base.ui


HEADERS += \
	$$PWD/visibletext.h
SOURCES += \
	$$PWD/visibletext.cpp

INCLUDEPATH += $$PWD/animatedstackedwidgettest
DEPENDPATH += $$PWD/animatedstackedwidgettest

INCLUDEPATH += $$PWD/fadingmultilinelabeltest
DEPENDPATH += $$PWD/fadingmultilinelabeltest

HEADERS += \
	$$PWD/borderrenderer.h \
	$$PWD/overlaywidget.h \
	$$PWD/expandingextrawidget.h \
	$$PWD/yarostercombobox.h \
	$$PWD/yalabel.h \
	$$PWD/yaselflabel.h \
	$$PWD/yacontactlabel.h \
	$$PWD/yacontactlistviewdelegate.h \
	$$PWD/yacontactlistviewslimdelegate.h \
	$$PWD/yacontactlistviewlargedelegate.h \
	$$PWD/yacontactlistview.h \
	$$PWD/yafilteredcontactlistview.h \
	$$PWD/yaselfstatus.h \
	$$PWD/yaavatarlabel.h \
	$$PWD/yacontactavatarlabel.h \
	$$PWD/yaselfavatarlabel.h \
	$$PWD/yaeventnotifierframe.h \
	$$PWD/trimmablemultilinelabel.h \
	$$PWD/yaeventmessage.h \
	$$PWD/yachatcontactstatus.h \
	$$PWD/yachatedit.h \
	$$PWD/yapushbutton.h \
	$$PWD/yalinkbutton.h \
	$$PWD/yastatusbar.h \
	$$PWD/yastatusbarlayout.h \
	$$PWD/yainformer.h \
	$$PWD/yainformerbutton.h \
	$$PWD/yabusywidget.h \
	$$PWD/animatedstackedwidgettest/baseanimatedstackedwidget.h \
	$$PWD/animatedstackedwidgettest/animatedstackedwidget.h \
	$$PWD/yatoolbox.h \
	$$PWD/yatoolboxanimation.h \
	$$PWD/yatoolboxpage.h \
	$$PWD/yavisualutil.h \
	$$PWD/yaemptytextlineedit.h \
	$$PWD/yaexpandingoverlaylineedit.h \
	$$PWD/yawindow.h \
	$$PWD/yachatseparator.h \
	$$PWD/yawindowbackground.h \
	$$PWD/yatabwidget.h \
	$$PWD/yatabbar.h \
	$$PWD/yaclosebutton.h \
	$$PWD/yaokbutton.h \
	$$PWD/yachevronbutton.h \
	$$PWD/yachatview.h \
	$$PWD/yachatviewdelegate.h \
	$$PWD/yachatviewmodel.h \
	$$PWD/accountinformermodel.h \
	$$PWD/accountinformerdelegate.h \
	$$PWD/accountinformerview.h \
	$$PWD/yalastmailinformer.h \
	$$PWD/yalastmailinformermodel.h \
	$$PWD/yalastmailinformerdelegate.h \
	$$PWD/yalastmailinformerview.h \
	$$PWD/settingsbuttontest/yasettingsbutton.h \
	$$PWD/yarotation.h \
	$$PWD/yaofficebackgroundhelper.h \
	$$PWD/yamanageaccounts.h \
	$$PWD/yajidlabel.h \
	$$PWD/yapreferencestabbutton.h \
	$$PWD/yaeditorcontextmenu.h \
	$$PWD/yaboldmenu.h \
	$$PWD/fadingmultilinelabeltest/fadingmultilinelabel.h \
	$$PWD/yachatsendbutton.h

SOURCES += \
	$$PWD/expandingextrawidget.cpp \
	$$PWD/borderrenderer.cpp \
	$$PWD/yarostercombobox.cpp \
	$$PWD/yalabel.cpp \
	$$PWD/yaselflabel.cpp \
	$$PWD/yacontactlabel.cpp \
	$$PWD/yacontactlistviewdelegate.cpp \
	$$PWD/yacontactlistviewslimdelegate.cpp \
	$$PWD/yacontactlistviewlargedelegate.cpp \
	$$PWD/yacontactlistview.cpp \
	$$PWD/yafilteredcontactlistview.cpp \
	$$PWD/yaselfstatus.cpp \
	$$PWD/yaavatarlabel.cpp \
	$$PWD/yacontactavatarlabel.cpp \
	$$PWD/yaselfavatarlabel.cpp \
	$$PWD/yaeventnotifierframe.cpp \
	$$PWD/trimmablemultilinelabel.cpp \
	$$PWD/yaeventmessage.cpp \
	$$PWD/yachatcontactstatus.cpp \
	$$PWD/yachatedit.cpp \
	$$PWD/yapushbutton.cpp \
	$$PWD/yalinkbutton.cpp \
	$$PWD/yastatusbar.cpp \
	$$PWD/yastatusbarlayout.cpp \
	$$PWD/yainformer.cpp \
	$$PWD/yainformerbutton.cpp \
	$$PWD/yabusywidget.cpp \
	$$PWD/animatedstackedwidgettest/baseanimatedstackedwidget.cpp \
	$$PWD/animatedstackedwidgettest/animatedstackedwidget.cpp \
	$$PWD/yatoolbox.cpp \
	$$PWD/yatoolboxanimation.cpp \
	$$PWD/yatoolboxpage.cpp \
	$$PWD/yavisualutil.cpp \
	$$PWD/yaemptytextlineedit.cpp \
	$$PWD/yaexpandingoverlaylineedit.cpp \
	$$PWD/yawindow.cpp \
	$$PWD/yachatseparator.cpp \
	$$PWD/yawindowbackground.cpp \
	$$PWD/yatabwidget.cpp \
	$$PWD/yatabbar.cpp \
	$$PWD/yaclosebutton.cpp \
	$$PWD/yaokbutton.cpp \
	$$PWD/yachevronbutton.cpp \
	$$PWD/yachatview.cpp \
	$$PWD/yachatviewdelegate.cpp \
	$$PWD/yachatviewmodel.cpp \
	$$PWD/accountinformermodel.cpp \
	$$PWD/accountinformerdelegate.cpp \
	$$PWD/accountinformerview.cpp \
	$$PWD/yalastmailinformer.cpp \
	$$PWD/yalastmailinformermodel.cpp \
	$$PWD/yalastmailinformerdelegate.cpp \
	$$PWD/yalastmailinformerview.cpp \
	$$PWD/settingsbuttontest/yasettingsbutton.cpp \
	$$PWD/yarotation.cpp \
	$$PWD/yaofficebackgroundhelper.cpp \
	$$PWD/yamanageaccounts.cpp \
	$$PWD/yajidlabel.cpp \
	$$PWD/yapreferencestabbutton.cpp \
	$$PWD/yaeditorcontextmenu.cpp \
	$$PWD/yaboldmenu.cpp \
	$$PWD/fadingmultilinelabeltest/fadingmultilinelabel.cpp \
	$$PWD/yachatsendbutton.cpp

INTERFACES += \
	$$PWD/../yaeventnotifier.ui \
	$$PWD/yalastmailinformer.ui \
	$$PWD/yamanageaccounts.ui
