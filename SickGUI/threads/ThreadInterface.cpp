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
	QMutexLocker locker(&plcMutex);
	Frameset::frameset_t fs;
	if (!plcBuffer.empty())
	{
		fs = plcBuffer.front();
	}
	return fs;
}

const Frameset::frameset_t ThreadInterface::peekGuiFrame()
{
	QMutexLocker locker(&guiMutex);
	Frameset::frameset_t fs;
	if (!guiBuffer.empty())
	{
		fs = guiBuffer.front();
	}
	return fs;
}

const Frameset::frameset_t ThreadInterface::peekWebFrame()
{
	QMutexLocker locker(&webMutex);
	Frameset::frameset_t fs;
	if (!webBuffer.empty())
	{
		fs = webBuffer.front();
	}
	return fs;
}

const Frameset::frameset_t ThreadInterface::popPlcFrame()
{
	QMutexLocker locker(&plcMutex);
	Frameset::frameset_t fs;
	if (!plcBuffer.empty())
	{
		fs = plcBuffer.front();
		plcBuffer.pop_front();
	}
	return fs;
}

const Frameset::frameset_t ThreadInterface::popGuiFrame()
{
	QMutexLocker locker(&guiMutex);
	Frameset::frameset_t fs;
	if (!guiBuffer.empty())
	{
		fs = guiBuffer.front();
		guiBuffer.pop_front();
	}
	return fs;
}

const Frameset::frameset_t ThreadInterface::popWebFrame()
{
	QMutexLocker locker(&webMutex);
	Frameset::frameset_t fs;
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


