#pragma once
#include <NodeData.hpp>
#include <Frameset.h>

class FrameNodeData : public QtNodes::NodeData
{
public:
	FrameNodeData() {}
	FrameNodeData(const frameset::Frame& frame) : _frame(frame) {}

	QtNodes::NodeDataType type() const override
	{
		return { "frame", "F" };
	}

	frameset::Frame frame() const { return _frame; }

private:
	frameset::Frame _frame;
};

