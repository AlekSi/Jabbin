#ifndef GLOBALEVENTQUEUE_H
#define GLOBALEVENTQUEUE_H

class PsiEvent;

class GlobalEventQueue
{
public:
	GlobalEventQueue() {}

	QList<int> ids() const {
		return QList<int>();
	}

	PsiEvent* peek(int id) const {
		return 0;
	}

	static GlobalEventQueue* instance()
	{
		if (!instance_)
			instance_ = new GlobalEventQueue();
		return instance_;
	}

	static GlobalEventQueue* instance_;
};

GlobalEventQueue* GlobalEventQueue::instance_ = 0;

#endif
