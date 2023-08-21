#include "ThreadInterface.h"
#include <qdebug.h>

ThreadInterface& ThreadInterface::instance()
{
	static ThreadInterface _instance;
	return _instance;
}

void ThreadInterface::pushFilteredFrame(const frameset::Frameset& fs)
{
	QMutexLocker locker(&plcMutex);
	plcBuffer.push_back(fs);
}

void ThreadInterface::pushRawFrame(const frameset::Frameset& fs)
{
	QMutexLocker locker(&rawMutex);
	rawBuffer.push_back(fs);
}

const frameset::Frameset ThreadInterface::peekFilteredFrame()
{
	QMutexLocker locker(&plcMutex);
	frameset::Frameset fs;
	if (!plcBuffer.empty())
	{
		fs = plcBuffer.front();
	}
	return fs;
}

const frameset::Frameset ThreadInterface::peekRawFrame()
{
	QMutexLocker locker(&rawMutex);
	frameset::Frameset fs;
	if (!rawBuffer.empty())
	{
		fs = rawBuffer.front();
	}
	return fs;
}

const frameset::Frameset ThreadInterface::popFilteredFrame()
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

const frameset::Frameset ThreadInterface::popRawFrame()
{
	QMutexLocker locker(&rawMutex);
	frameset::Frameset fs;
	if (!rawBuffer.empty())
	{
		fs = rawBuffer.front();
		rawBuffer.pop_front();
	}
	return fs;
}

ThreadInterface::ThreadInterface() :
	plcBuffer(bufferSize),
	rawBuffer(bufferSize)
{

}

ThreadInterface::~ThreadInterface()
{

}


