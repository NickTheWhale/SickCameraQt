#pragma once
#include <qthread.h>
#include <qimage.h>

class WebThread : public QThread
{
	Q_OBJECT

public slots:
	void newImage(const QImage& image);

public:
	bool startWeb();
	void stopWeb();

protected:
	void run() override;

private:
	volatile bool _stop = false;
};

