#include "PlcEndModel.h"

PlcEndModel::PlcEndModel() :
	_widget(new QWidget())
{
	_widget->setMaximumSize(0, 0);
}

unsigned int PlcEndModel::nPorts(QtNodes::PortType const portType) const
{
	unsigned int result = 1;

	switch (portType)
	{
	case QtNodes::PortType::In:
		result = 1;
		break;

	case QtNodes::PortType::Out:
		result = 0;
		break;
	}

	return result;
}

QJsonObject PlcEndModel::save() const
{
	QJsonObject root;
	root["model-name"] = name();
	root["filterable"] = false;
	return root;
}

void PlcEndModel::load(QJsonObject const& p)
{
}
