/*****************************************************************//**
 * @file   MatNodeData.h
 * @brief  Class to encapsulate cv::Mat for node data flow model.
 * 
 * @author Nicholas Loehrke
 * @date   August 2023
 *********************************************************************/

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

