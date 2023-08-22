#pragma once
#include <NodeData.hpp>
#include <opencv2/core.hpp>

class MatNodeData : public QtNodes::NodeData
{
public:
	MatNodeData() {}
	MatNodeData(const cv::Mat& mat) : _mat(mat) {}

	QtNodes::NodeDataType type() const override
	{
		return { "cv::Mat", "Mat" };
	}

	cv::Mat mat() const { return _mat; }

private:
	cv::Mat _mat;
};

