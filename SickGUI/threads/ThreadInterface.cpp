#include "ThreadInterface.h"

ThreadInterface& ThreadInterface::instance()
{
	static ThreadInterface _instance;
	return _instance;
}

void ThreadInterface::pushPlcFrame(const Frameset::frameset_t& fs)
{
	QMutexLocker locker(&plcMutex);
	plcBuffer.push_back(fs);
}

void ThreadInterface::pushGuiFrame(const Frameset::frameset_t& fs)
{
	QMutexLocker locker(&guiMutex);
	guiBuffer.push_back(fs);
}

void ThreadInterface::pushWebFrame(const Frameset::frameset_t& fs)
{
	QMutexLocker locker(&webMutex);
	webBuffer.push_back(fs);
}

const Frameset::frameset_t ThreadInterface::peekPlcFrame()
{
	Frameset::frameset_t fs;
	QMutexLocker locker(&plcMutex);
	if (!plcBuffer.empty())
	{
		fs = plcBuffer.front();
	}
	return fs;
}

const Frameset::frameset_t ThreadInterface::peekGuiFrame()
{
	Frameset::frameset_t fs;
	QMutexLocker locker(&guiMutex);
	if (!guiBuffer.empty())
	{
		fs = guiBuffer.front();
	}
	return fs;
}

const Frameset::frameset_t ThreadInterface::peekWebFrame()
{
	Frameset::frameset_t fs;
	QMutexLocker locker(&webMutex);
	if (!webBuffer.empty())
	{
		fs = webBuffer.front();
	}
	return fs;
}

const Frameset::frameset_t ThreadInterface::popPlcFrame()
{
	Frameset::frameset_t fs;
	QMutexLocker locker(&plcMutex);
	if (!plcBuffer.empty())
	{
		fs = plcBuffer.front();
		plcBuffer.pop_front();
	}
	return fs;
}

const Frameset::frameset_t ThreadInterface::popGuiFrame()
{
	Frameset::frameset_t fs;
	QMutexLocker locker(&guiMutex);
	if (!guiBuffer.empty())
	{
		fs = guiBuffer.front();
		guiBuffer.pop_front();
	}
	return fs;
}

const Frameset::frameset_t ThreadInterface::popWebFrame()
{
	Frameset::frameset_t fs;
	QMutexLocker locker(&webMutex);
	if (!webBuffer.empty())
	{
		fs = webBuffer.front();
		webBuffer.pop_front();
	}
	return fs;
}

ThreadInterface::ThreadInterface() :
	plcBuffer(bufferSize),
	guiBuffer(bufferSize),
	webBuffer(bufferSize)
{

}

ThreadInterface::~ThreadInterface()
{

}


