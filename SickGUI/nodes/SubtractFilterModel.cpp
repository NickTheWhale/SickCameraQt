/*****************************************************************//**
 * @file   SubtractFilterModel.cpp
 * @brief  Implements QtNodes::NodeDelegateModel.
 *
 * @note See BilateralFilterModel method documentation since all models follow the same pattern.
 * 
 * @author Nicholas Loehrke
 * @date   September 2023
 *********************************************************************/

#include "SubtractFilterModel.h"

SubtractFilterModel::SubtractFilterModel()
{
}

unsigned int SubtractFilterModel::nPorts(QtNodes::PortType const portType) const
{
	unsigned int ports = 1;

	switch (portType)
	{
	case QtNodes::PortType::In:
		ports = 2;
		break;

	case QtNodes::PortType::Out:
		ports = 1;
		break;
	}

	return ports;
}

void SubtractFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
	switch (portIndex)
	{
	case QtNodes::PortIndex(0):
		_portNodeData0 = nodeData;
		break;

	case QtNodes::PortIndex(1):
		_portNodeData1 = nodeData;
		break;
	}

	applyFilter();
}

QWidget* SubtractFilterModel::embeddedWidget()
{
	if (!_widget)
		createWidgets();
	return _widget;
}

QJsonObject SubtractFilterModel::save() const
{
	QJsonObject root;
	root["model-name"] = modelName();
	return root;
}

void SubtractFilterModel::load(QJsonObject const& p)
{
}

void SubtractFilterModel::applyFilter()
{
	if (_portNodeData0 && _portNodeData1)
	{
		auto d0 = std::dynamic_pointer_cast<MatNodeData>(_portNodeData0);
		auto d1 = std::dynamic_pointer_cast<MatNodeData>(_portNodeData1);

		if (d0 && d1 && d0->mat().size() == d1->mat().size() && d0->mat().type() == d1->mat().type())
		{
			cv::Mat outputMat;

			cv::absdiff(d0->mat(), d1->mat(), outputMat);
			_currentNodeData = std::make_shared<MatNodeData>(outputMat);
		}
	}

	emit dataUpdated(0);
}

void SubtractFilterModel::createWidgets()
{
	_widget = new QWidget();
	_widget->resize(0, 0);
}
