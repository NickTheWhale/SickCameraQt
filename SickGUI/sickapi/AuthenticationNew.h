#pragma once
#include <IAuthentication.h>
#include <VisionaryControl.h>

namespace visionary
{
	class AuthenticationNew : public IAuthentication
	{
	public:
		explicit AuthenticationNew(VisionaryControl& vctrl);
		virtual ~AuthenticationNew();

		virtual bool login(UserLevel userLevel, const std::string& password);
		virtual bool logout();

	private:
		VisionaryControl& m_VisionaryControl;
	};
};

