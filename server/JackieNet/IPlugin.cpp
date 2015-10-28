#include "IPlugin.h"


IPlugin::IPlugin()
{
}


IPlugin::~IPlugin()
{
}

void IPlugin::OnRakPeerStartup()
{
	throw std::logic_error("The method or operation is not implemented.");
}
