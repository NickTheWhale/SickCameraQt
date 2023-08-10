#include "SubtractFilterModel.h"

SubtractFilterModel::SubtractFilterModel() :
	_widget(new QWidget())
{
	_widget->resize(QSize(0, 0));
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

void SubtractFilterModel::applyFilter()
{
	if (_portNodeData0 && _portNodeData1)
	{
		frameset::Frame frame0 = std::dynamic_pointer_cast<FrameNodeData>(_portNodeData0)->frame();
		frameset::Frame frame1 = std::dynamic_pointer_cast<FrameNodeData>(_portNodeData1)->frame();

		if (frameset::size(frame0) == frameset::size(frame1))
		{
			_currentNodeData = std::make_shared<FrameNodeData>(frameset::difference(frame0, frame1));
		}
	}

	emit dataUpdated(0);
}
