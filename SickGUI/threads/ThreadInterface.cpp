#include "ThreadInterface.h"
#include <qdebug.h>

ThreadInterface& ThreadInterface::instance()
{
	static ThreadInterface _instance;
	return _instance;
}

void ThreadInterface::pushFilteredFrame(const frameset::Frameset& fs)
{
	QMutexLocker locker(&filteredMutex);
	filteredBuffer.push_back(fs);
}

void ThreadInterface::pushRawFrame(const frameset::Frameset& fs)
{
	QMutexLocker locker(&rawMutex);
	rawBuffer.push_back(fs);
}

const frameset::Frameset ThreadInterface::peekFilteredFrame()
{
	QMutexLocker locker(&filteredMutex);
	frameset::Frameset fs;
	if (!filteredBuffer.empty())
	{
		fs = filteredBuffer.front();
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
	QMutexLocker locker(&filteredMutex);
	frameset::Frameset fs;
	if (!filteredBuffer.empty())
	{
		fs = filteredBuffer.front();
		filteredBuffer.pop_front();
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
	filteredBuffer(bufferSize),
	rawBuffer(bufferSize)
{

}

ThreadInterface::~ThreadInterface()
{

}


