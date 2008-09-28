#ifndef MSGMLE_H
#define MSGMLE_H

#include <QTextEdit>

class ChatView : public QTextEdit
{
public:
	ChatView(QWidget *parent)
		: QTextEdit(parent)
	{}
};

class ChatEdit : public QTextEdit
{
public:
	ChatEdit(QWidget *parent)
		: QTextEdit(parent)
	{}
};

#endif
