#include "PlcStartModel.h"

PlcStartModel::PlcStartModel() :
	_widget(new QWidget())
{
	_widget->setMaximumSize(0, 0);
}

unsigned int PlcStartModel::nPorts(QtNodes::PortType const portType) const
{
	unsigned int result = 1;
	
	switch (portType)
	{
	case QtNodes::PortType::In:
		result = 0;
		break;

	case QtNodes::PortType::Out:
		result = 1;
		break;
	}

	return result;
}

QJsonObject PlcStartModel::save() const
{
	QJsonObject root;
	root["model-name"] = name();
	return root;
}

void PlcStartModel::load(QJsonObject const& p)
{
}
