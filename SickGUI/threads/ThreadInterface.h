#pragma once
#include <Frameset.h>
#include <boost/circular_buffer.hpp>
#include <qmutex.h>

class ThreadInterface
{
public:
	static ThreadInterface& instance();

	void pushPlcFrame(const frameset::Frameset& fs);
	void pushGuiFrame(const frameset::Frameset& fs);
	void pushWebFrame(const frameset::Frameset& fs);

	const frameset::Frameset peekPlcFrame();
	const frameset::Frameset peekGuiFrame();
	const frameset::Frameset peekWebFrame();

	const frameset::Frameset popPlcFrame();
	const frameset::Frameset popGuiFrame();
	const frameset::Frameset popWebFrame();
	
private:
	explicit ThreadInterface();
	~ThreadInterface();

	const int bufferSize = 2;
	boost::circular_buffer<frameset::Frameset> plcBuffer;
	boost::circular_buffer<frameset::Frameset> guiBuffer;
	boost::circular_buffer<frameset::Frameset> webBuffer;

	QMutex plcMutex;
	QMutex guiMutex;
	QMutex webMutex;
};

