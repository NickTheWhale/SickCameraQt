/*****************************************************************//**
 * @file   PlcEndModel.cpp
 * @brief  Special node used to indicate the end of a filter flow.
 * 
 * @author Nicholas Loehrke
 * @date   September 2023
 *********************************************************************/

#include "PlcEndModel.h"

PlcEndModel::PlcEndModel()
{
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

QWidget* PlcEndModel::embeddedWidget()
{
	if (_widget)
		createWidgets();
	return _widget;
}

QJsonObject PlcEndModel::save() const
{
	QJsonObject root;
	root["model-name"] = name();
	return root;
}

void PlcEndModel::load(QJsonObject const& p)
{
}

void PlcEndModel::createWidgets()
{
	_widget = new QWidget();
	_widget->resize(0, 0);
}
