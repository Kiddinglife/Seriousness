#ifndef PluginReceiveResult_H_
#define PluginReceiveResult_H_

namespace JACKIE_INET
{
	/// For each message that arrives on an instance of RakPeer, the plugins get an 
	/// opportunity to process them first. This enumeration represents what to do 
	/// with the message
	enum PluginActionType
	{
		/// The plugin used this message and it shouldn't be given to the user.
		PROCESSED_BY_ME_THEN_DEALLOC = 0,
		/// This message will be processed by other plugins, and at last by the user.
		PROCESSED_BY_OTHERS,
		/// The plugin is going to hold on to this message.  
		/// Do not deallocate it and do not pass it to other plugins either.
		HOLD_ON_BY_ME_NOT_DEALLOC
	};

	class IPlugin
	{
		public:
		IPlugin();
		virtual ~IPlugin();

		void OnRakPeerStartup();

		/// Update is called every time a packet is checked for .
		virtual void Update(void) { }
	};
}
#endif