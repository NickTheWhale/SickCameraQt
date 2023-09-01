/*****************************************************************//**
 * @file   PlcStartModel.cpp
 * @brief  Special PLC start flag used to indicate start of filter flow.
 * 
 * @note See BilateralFilterModel method documentation since all models follow the same pattern.
 * 
 * @author Nicholas Loehrke
 * @date   September 2023
 *********************************************************************/

#include "PlcStartModel.h"

PlcStartModel::PlcStartModel()
{
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

QWidget* PlcStartModel::embeddedWidget()
{
	if (!_widget)
		createWidgets();
	return _widget;
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

void PlcStartModel::createWidgets()
{
	_widget = new QWidget();
	_widget->resize(0, 0);
}
