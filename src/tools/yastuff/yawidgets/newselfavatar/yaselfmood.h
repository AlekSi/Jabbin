/*
 * yaselfmood.h - expanding mood widget
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

#ifndef YASELFMOOD_H
#define YASELFMOOD_H

#include <QFrame>

#include "expandingextrawidget.h"
#include "overlaywidget.h"
#include "customtimeline.h"

class YaSelfMood;
class YaSelfMoodButton;
class YaSelfMoodActionGroup;
class YaSelfMoodStatusButton;

class QAction;
class QActionGroup;
class QPixmap;
class QLineEdit;
class QTextEdit;
class QMenu;
class QToolButton;

#include "xmpp_status.h"

class YaSelfMoodExtra : public ExpandingExtraWidget
{
	Q_OBJECT
public:
	YaSelfMoodExtra(QWidget* parent, YaSelfMood* placeholderParent);

	QMenu* menu() const;

	QSize arrowSize() const;

	bool editingBackground() const;
	bool moodIsEditable() const;

	// reimplemented
	bool isExpanded() const;
	int normalHeight() const;
	int expandedHeight() const;
	int editingHeight() const;
	int normalWidth() const;
	int expandedWidth() const;
	int editingWidth() const;

	QString statusText() const;
	void setStatusText(const QString& statusText);

	XMPP::Status::Type statusType() const;
	void setStatusType(XMPP::Status::Type type);
	void setHaveConnectingAccounts(bool haveConnectingAccounts);

	void setMoods(QStringList moods);

	// reimplemented
	QSize sizeHint() const;
	bool eventFilter(QObject* obj, QEvent* event);

signals:
	void statusChanged(const QString&);
	void statusChanged(XMPP::Status::Type);

protected:
	// reimplemented
	void paintEvent(QPaintEvent*);
	void resizeEvent(QResizeEvent* event);
	void contextMenuEvent(QContextMenuEvent* e);
	void changeEvent(QEvent*);
	void focusOut();
	void cancelAction();

	void drawBackground(QPainter* painter, const QRect& rect, const QRect arrowRect);

	void setStatusTextUserAction(const QString& statusText);

private:
	QMenu* menu_;
	QTextEdit* lineEdit_;
	bool lineEditHovered_;
	QTimer* focusInTimer_;

	QAction* currentAction_;
	YaSelfMoodButton* currentActionButton_;
	YaSelfMoodStatusButton* statusButton_;
	QToolButton* okButton_;
	QAction* onlineAction_;
	QAction* changeStatusAction_;
	QAction* offlineAction_;
	QAction* setCustomMoodAction_;

	QList<QObject*> menuActions_;
	YaSelfMoodActionGroup* menuPastStatuses_;

	XMPP::Status::Type statusType_;

	QRect innerRect_;
	QRect arrowRect_;
	QRect downArrowRect_;

	mutable bool editingBackground_;
	mutable bool showingLineEditMenu_;

	enum State {
		Normal = 0,
		Editing,
		// Menu,
		// ExpandingToMenu,
		// MenuToNormal,
		// MenuToEditing,
		NormalToEditing,
		EditingToNormal
	};
	State state_;
	int animatingTo_;

	QSize sizeHint(State state) const;

	// reimplemented
	void setState(State state);
	void setNormalState();
	void stopAnimation();

	// QPixmap downArrowPixmap() const;
	void updateButtonsPositions();

	void createMenu();

	QString statusName(XMPP::Status::Type) const;

	QRect lineEditRect() const;
	bool lineEditHovered() const;

	void startAnimation(State state);
	void setActionMood(QAction* action, const QString& mood);

private slots:
	// reimplemented
	void animate();
	void animationFinished();

	void updateLineEditPosition();
	void invalidateButtons();
	void setCustomMood();
	void setMood();
	void setStatus();
	void lineEditTextChanged();
	void updateLineEdit();
	void updateSizes();
	void doFocusIn();
	void showYaruWarning();
};

class YaSelfMood : public OverlayWidget<QFrame, YaSelfMoodExtra>
{
	Q_OBJECT
public:
	YaSelfMood(QWidget* parent);
	~YaSelfMood();

	QMenu* menu() const;

	QString statusText() const;
	QList<QString> statuses() const;

	XMPP::Status::Type statusType() const;

public slots:
	void setStatusText(const QString&);
	void setStatusType(XMPP::Status::Type type);
	void setHaveConnectingAccounts(bool haveConnectingAccounts);
	void clearMoods();

signals:
	void statusChanged(const QString&);
	void statusChanged(XMPP::Status::Type);

private slots:
	void statusTextChanged();
	void statusTypeChanged();

private:
	// reimplemented
	QRect extraGeometry() const;
	void paintEvent(QPaintEvent*);

	void saveStatusMessages(QList<QString> statuses);
};

#endif
