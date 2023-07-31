#pragma once
#include "..\VisionaryFrameset.h"
#include <boost/circular_buffer.hpp>
#include <qmutex.h>

class ThreadInterface
{
public:
	static ThreadInterface& instance();

	void pushPlcFrame(const Frameset::frameset_t& fs);
	void pushGuiFrame(const Frameset::frameset_t& fs);
	void pushWebFrame(const Frameset::frameset_t& fs);

	const Frameset::frameset_t peekPlcFrame();
	const Frameset::frameset_t peekGuiFrame();
	const Frameset::frameset_t peekWebFrame();

	const Frameset::frameset_t popPlcFrame();
	const Frameset::frameset_t popGuiFrame();
	const Frameset::frameset_t popWebFrame();
	
private:
	explicit ThreadInterface();
	~ThreadInterface();

	const int bufferSize = 2;
	boost::circular_buffer<Frameset::frameset_t> plcBuffer;
	boost::circular_buffer<Frameset::frameset_t> guiBuffer;
	boost::circular_buffer<Frameset::frameset_t> webBuffer;

	QMutex plcMutex;
	QMutex guiMutex;
	QMutex webMutex;
};
