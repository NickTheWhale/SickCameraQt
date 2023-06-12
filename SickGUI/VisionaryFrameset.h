#pragma once
#include "VisionaryFrame.h"


class Frameset
{
public:
	Frameset() {};
	Frameset(DepthFrame depth, ColorFrame color, StateFrame state);
	~Frameset();
	DepthFrame getDepth();
	ColorFrame getColor();
	StateFrame getState();

private:
	DepthFrame depth;
	ColorFrame color;
	StateFrame state;
};
