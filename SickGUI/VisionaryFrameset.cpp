#include "VisionaryFrameset.h"

Frameset::Frameset(DepthFrame depth, ColorFrame color, StateFrame state)
	: depth(depth), color(color), state(state)
{
}

Frameset::~Frameset()
{
}

DepthFrame Frameset::getDepth()
{
	return depth;
}

ColorFrame Frameset::getColor()
{
	return color;
}

StateFrame Frameset::getState()
{
	return state;
}
