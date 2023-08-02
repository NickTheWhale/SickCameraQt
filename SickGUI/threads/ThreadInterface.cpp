#include "ThreadInterface.h"

ThreadInterface& ThreadInterface::instance()
{
	static ThreadInterface _instance;
	return _instance;
}

void ThreadInterface::pushPlcFrame(const frameset::Frameset& fs)
{
	QMutexLocker locker(&plcMutex);
	plcBuffer.push_back(fs);
}

void ThreadInterface::pushGuiFrame(const frameset::Frameset& fs)
{
	QMutexLocker locker(&guiMutex);
	guiBuffer.push_back(fs);
}

void ThreadInterface::pushWebFrame(const frameset::Frameset& fs)
{
	QMutexLocker locker(&webMutex);
	webBuffer.push_back(fs);
}

const frameset::Frameset& ThreadInterface::peekPlcFrame()
{
	QMutexLocker locker(&plcMutex);
	frameset::Frameset fs;
	if (!plcBuffer.empty())
	{
		fs = plcBuffer.front();
	}
	return fs;
}

const frameset::Frameset& ThreadInterface::peekGuiFrame()
{
	QMutexLocker locker(&guiMutex);
	frameset::Frameset fs;
	if (!guiBuffer.empty())
	{
		fs = guiBuffer.front();
	}
	return fs;
}

const frameset::Frameset& ThreadInterface::peekWebFrame()
{
	QMutexLocker locker(&webMutex);
	frameset::Frameset fs;
	if (!webBuffer.empty())
	{
		fs = webBuffer.front();
	}
	return fs;
}

const frameset::Frameset& ThreadInterface::popPlcFrame()
{
	QMutexLocker locker(&plcMutex);
	frameset::Frameset fs;
	if (!plcBuffer.empty())
	{
		fs = plcBuffer.front();
		plcBuffer.pop_front();
	}
	return fs;
}

const frameset::Frameset& ThreadInterface::popGuiFrame()
{
	QMutexLocker locker(&guiMutex);
	frameset::Frameset fs;
	if (!guiBuffer.empty())
	{
		fs = guiBuffer.front();
		guiBuffer.pop_front();
	}
	return fs;
}

const frameset::Frameset& ThreadInterface::popWebFrame()
{
	QMutexLocker locker(&webMutex);
	frameset::Frameset fs;
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


