/*
 * yaremoveconfirmationmessagebox.cpp - generic confirmation of destructive action
 * Copyright (C) 2008  Yandex LLC (Michail Pishchagin)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "yaremoveconfirmationmessagebox.h"

#include <QApplication>
#include <QPushButton>

#include "yastyle.h"

#ifdef Q_WS_WIN
// #define USE_WINDOWS_NATIVE_MSGBOX
#endif

#ifdef USE_WINDOWS_NATIVE_MSGBOX
#include <windows.h>

static QString okButtonCaption;
static QString cancelButtonCaption;
static bool activated;

// hook technique from http://www.catch22.net/tuts/msgbox.asp
static HHOOK hMsgBoxHook;

static int textWidth(HWND button, const QString& text)
{
	RECT textRect;
	textRect.left = textRect.top = textRect.right = textRect.bottom = 0;
	DrawText(GetDC(button), text.utf16(), text.length(), &textRect, DT_CALCRECT);
	return textRect.right - textRect.left;
}

LRESULT CALLBACK CBTProc(INT nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HCBT_ACTIVATE) {
		if (activated)
			return 0;
		activated = true;
		HWND hChildWnd = (HWND)wParam;

		HWND okButton = GetDlgItem(hChildWnd, IDOK);
		HWND cancelButton = GetDlgItem(hChildWnd, IDCANCEL);
		Q_ASSERT(okButton);
		Q_ASSERT(cancelButton);

		SetDlgItemText(hChildWnd, IDOK, okButtonCaption.utf16());
		SetDlgItemText(hChildWnd, IDCANCEL, cancelButtonCaption.utf16());

		RECT parentRect, okRect, cancelRect;
		GetWindowRect(GetParent(okButton), &parentRect);
		GetWindowRect(okButton, &okRect);
		GetWindowRect(cancelButton, &cancelRect);

		int parentWidth = parentRect.right - parentRect.left;

		int minMargin = 14;
		int spacing = cancelRect.left - okRect.right;

		int buttonWidth = qMax(::textWidth(okButton, okButtonCaption), ::textWidth(cancelButton, cancelButtonCaption));
		int maxButtonWidth = (parentWidth - minMargin*2 - spacing) / 2;
		buttonWidth = qMin(buttonWidth, maxButtonWidth);

		WINDOWPLACEMENT wp;
		GetWindowPlacement(okButton, &wp);
		wp.rcNormalPosition.left  = (parentWidth / 2) - (spacing / 2) - buttonWidth - 3;
		wp.rcNormalPosition.right = wp.rcNormalPosition.left + buttonWidth;
		SetWindowPlacement(okButton, &wp);

		GetWindowPlacement(cancelButton, &wp);
		wp.rcNormalPosition.left  = (parentWidth / 2) + (spacing / 2) - 3;
		wp.rcNormalPosition.right = wp.rcNormalPosition.left + buttonWidth;
		SetWindowPlacement(cancelButton, &wp);
	}
	else {
		CallNextHookEx(hMsgBoxHook, nCode, wParam, lParam);
	}
	return 0;
}

int MsgBoxEx(HWND hwnd, LPCTSTR szText, LPCTSTR szCaption, UINT uType)
{
	int retval;

	activated = false;
	hMsgBoxHook = SetWindowsHookEx(
	                  WH_CBT,
	                  CBTProc,
	                  NULL,
	                  GetCurrentThreadId()
	              );

	retval = MessageBox(hwnd, szText, szCaption, uType);

	UnhookWindowsHookEx(hMsgBoxHook);

	return retval;
}
#endif

YaRemoveConfirmationMessageBox::YaRemoveConfirmationMessageBox(const QString& title, const QString& informativeText, QWidget* parent)
	: QMessageBox()
	, removeButton_(0)
	, cancelButton_(0)
{
	setStyle(YaStyle::defaultStyle());

	setWindowTitle(tr("Joim"));
	setText(title);
	setInformativeText(informativeText);

	setIcon(QMessageBox::Warning);
	int iconSize = style()->pixelMetric(QStyle::PM_MessageBoxIconSize);
	QIcon tmpIcon= style()->standardIcon(QStyle::SP_MessageBoxWarning);
	if (!tmpIcon.isNull())
		setIconPixmap(tmpIcon.pixmap(iconSize, iconSize));

	// doesn't work with borderless top-level windows on Mac OS X
	// QWidget* window = parent->window();
	// msgBox.setParent(window);
	// msgBox.setWindowFlags(Qt::Sheet);
	Q_UNUSED(parent);
}

void YaRemoveConfirmationMessageBox::setDestructiveActionName(const QString& destructiveAction)
{
	Q_ASSERT(!removeButton_);
	Q_ASSERT(!cancelButton_);
	removeButton_ = addButton(destructiveAction, QMessageBox::AcceptRole /*QMessageBox::DestructiveRole*/);
	cancelButton_ = addButton(QMessageBox::Cancel);
	setDefaultButton(removeButton_);
}

bool YaRemoveConfirmationMessageBox::confirmRemoval()
{
	if (!removeButton_) {
		setDestructiveActionName(tr("Delete"));
	}

	QString text = informativeText();
	text.replace("<br>", "\n");
	QRegExp rx("<.+>");
	rx.setMinimal(true);
	text.replace(rx, "");

	setInformativeText(QString());
	setText(text);

#ifdef USE_WINDOWS_NATIVE_MSGBOX
	okButtonCaption = removeButton_->text();
	cancelButtonCaption = cancelButton_->text();

	int msgboxID = MsgBoxEx(
	                   NULL,
	                   text.utf16(),
	                   windowTitle().utf16(),
	                   MB_ICONWARNING | MB_OKCANCEL | MB_APPLMODAL
	               );

	return msgboxID == IDOK;
#else
	Q_ASSERT(removeButton_);
	Q_ASSERT(cancelButton_);
	YaStyle::makeMeNativeLooking(this);
	exec();
	return clickedButton() == removeButton_;
#endif
}
