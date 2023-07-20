#pragma once
#include <qobject.h>
class WebWorker : public QObject
{
	Q_OBJECT

public:
	explicit WebWorker();
	~WebWorker();

public slots:
	void process();
	
signals:
	void finished();
	
};

