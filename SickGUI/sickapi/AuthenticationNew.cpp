#include "AuthenticationNew.h"
#include "CoLaParameterWriter.h"
#include "CoLaParameterReader.h"

namespace visionary
{
	AuthenticationNew::AuthenticationNew(VisionaryControl& vctrl) : m_VisionaryControl(vctrl)
	{
	}

	AuthenticationNew::~AuthenticationNew()
	{
	}

	bool AuthenticationNew::login(UserLevel userLevel, const std::string& password)
	{
		return true;
		CoLaCommand loginCommand = CoLaParameterWriter(CoLaCommandType::METHOD_INVOCATION, "GetChallenge").parameterUSInt(3).build();
		CoLaCommand loginResponse = m_VisionaryControl.sendCommand(loginCommand);

		if (loginResponse.getError() == CoLaError::OK)
		{
			
			return true;
		}
		return false;
	}

	bool AuthenticationNew::logout()
	{
		return false;
	}

}


//bool AuthenticationLegacy::login(UserLevel userLevel, const std::string& password)
//{
//	CoLaCommand loginCommand = CoLaParameterWriter(CoLaCommandType::METHOD_INVOCATION, "SetAccessMode").parameterSInt(static_cast<int8_t>(userLevel)).parameterPasswordMD5(password).build();
//	CoLaCommand loginResponse = m_VisionaryControl.sendCommand(loginCommand);
//
//
//	if (loginResponse.getError() == CoLaError::OK)
//	{
//		return CoLaParameterReader(loginResponse).readBool();
//	}
//	return false;
//}
//
//bool AuthenticationLegacy::logout()
//{
//	CoLaCommand runCommand = CoLaParameterWriter(CoLaCommandType::METHOD_INVOCATION, "Run").build();
//	CoLaCommand runResponse = m_VisionaryControl.sendCommand(runCommand);
//
//	if (runResponse.getError() == CoLaError::OK)
//	{
//		return CoLaParameterReader(runResponse).readBool();
//	}
//	return false;
//}