#include "BufferManager.h"

BufferManager& BufferManager::instance()
{
	static BufferManager _instance;
	return _instance;
}

void BufferManager::pushPlcFrame(const Frameset::frameset_t& fs)
{
	QMutexLocker locker(&plcMutex);
	plcBuffer.push_back(fs);
}

void BufferManager::pushGuiFrame(const Frameset::frameset_t& fs)
{
	QMutexLocker locker(&guiMutex);
	guiBuffer.push_back(fs);
}

void BufferManager::pushWebFrame(const Frameset::frameset_t& fs)
{
	QMutexLocker locker(&webMutex);
	webBuffer.push_back(fs);
}

const Frameset::frameset_t BufferManager::peekPlcFrame()
{
	Frameset::frameset_t fs;
	QMutexLocker locker(&plcMutex);
	if (!plcBuffer.empty())
	{
		fs = plcBuffer.front();
	}
	return fs;
}

const Frameset::frameset_t BufferManager::peekGuiFrame()
{
	Frameset::frameset_t fs;
	QMutexLocker locker(&guiMutex);
	if (!guiBuffer.empty())
	{
		fs = guiBuffer.front();
	}
	return fs;
}

const Frameset::frameset_t BufferManager::peekWebFrame()
{
	Frameset::frameset_t fs;
	QMutexLocker locker(&webMutex);
	if (!webBuffer.empty())
	{
		fs = webBuffer.front();
	}
	return fs;
}

const Frameset::frameset_t BufferManager::popPlcFrame()
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

const Frameset::frameset_t BufferManager::popGuiFrame()
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

const Frameset::frameset_t BufferManager::popWebFrame()
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

BufferManager::BufferManager() :
	plcBuffer(bufferSize),
	guiBuffer(bufferSize),
	webBuffer(bufferSize)
{

}

BufferManager::~BufferManager()
{

}


