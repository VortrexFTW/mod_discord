#include "pch.h"

#include <stdio.h>
#include <stdlib.h>

#include <SDKHelper.h>

#include <dpp.h>

// The modules internal name (Also used for the namespace name)
MODULE_MAIN("discord");

// -------------------------------------------------------------------------

// Needed because Module SDK does not have a way to trigger an event, so we need to grab the "triggerEvent" function from the scripting interface.
// Scripts will need to call module.discord.getTriggerEvent(triggerEvent) to hand the triggerEvent function over to this module so it can be used.
Galactic3D::Strong<Galactic3D::Interfaces::IFunction> g_TriggerEventFunction;

// -------------------------------------------------------------------------

SDK::Class g_ConnectionClass;
SDK::Class g_UserClass;
SDK::Class g_GuildMemberClass;
SDK::Class g_SlashCommandClass;
SDK::Class g_SlashCommandOptionClass;
SDK::Class g_ReceivedSlashCommandClass;

// -------------------------------------------------------------------------

class CDiscordConnection
{
public:
	const char* m_szToken;
	dpp::cluster m_pBot;

	// Add a constructor with a dpp::cluster object while dpp::cluster does not have an appropriate default constructor available
	CDiscordConnection((dpp::cluster)pBot) {
		m_pBot = pBot;
	}
};

// -------------------------------------------------------------------------

class CDiscordGuildMember
{
public:
	dpp::guild_member m_pGuildMember;

	CDiscordGuildMember(dpp::guild_member pGuildMember) {
		m_pGuildMember = pGuildMember;
	}
};

// -------------------------------------------------------------------------

class CDiscordReceivedSlashCommand
{
public:
	dpp::slashcommand_t m_pSlashCommand;

	CDiscordReceivedSlashCommand(dpp::slashcommand_t pSlashCommand) {
		m_pSlashCommand = pSlashCommand;
	}
};

// -------------------------------------------------------------------------

void ModuleRegister()
{
	g_ConnectionClass = SDK::Class("DiscordConnection");
	g_UserClass = SDK::Class("DiscordUser");
	g_GuildMemberClass = SDK::Class("DiscordGuildMember");
	g_SlashCommandClass = SDK::Class("DiscordSlashCommand");
	g_SlashCommandOptionClass = SDK::Class("DiscordSlashCommandOption");
	g_ReceivedSlashCommandClass = SDK::Class("DiscordReceivedSlashCommand");

	SDK::RegisterFunction("init", [](Galactic3D::Interfaces::INativeState* pState, int32_t argc, void* pUser) {
		SDK_TRY;

		SDK::State State(pState);

		const char* szToken = State.CheckString(0);

		if(g_TriggerEventFunction == nullptr) {
			return pState->SetError("Can't initialize discord connection, please use 'module.discord.getTriggerEvent(triggerEvent)' first!");
		}

		dpp::cluster pBot(szToken);

		//command_handler(&pBot);
		//g_pBotCommandHandler = command_handler;

		pBot.on_log(dpp::utility::cout_logger());

		pBot.on_slashcommand([](const dpp::slashcommand_t& event) {
			SDK::ArrayValue Args;

			SDK::StringValue Name("OnDiscordCommand");
			Args.Insert(Name);

			dpp::guild_member pMember = event.command.member;
			SDK::ClassValue<CDiscordGuildMember, g_GuildMemberClass> User(new CDiscordGuildMember(pMember));
			Args.Insert(User);

			SDK::StringValue CommandName(event.command.get_command_name().c_str());
			Args.Insert(CommandName);

			//dpp::slashcommand_t pReceivedSlashCommand = event.member;
			//SDK::ClassValue<CDiscordReceivedSlashCommand, g_ReceivedSlashCommandClass> User(new CDiscordReceivedSlashCommand(pReceivedSlashCommand));
			//Args.Insert(pReceivedSlashCommand);

			GALACTIC_CALL(g_TriggerEventFunction->Call(Args.m_pArray));
	    });

		pBot.on_ready([&pBot](const dpp::ready_t& event) {
			if (dpp::run_once<struct register_bot_commands>()) {
				//bot.global_command_create(dpp::slashcommand("ping", "Ping pong!", bot.me.id));
			}
		});

		// Return the CConnection object
		SDK::ClassValue<CDiscordConnection, g_ConnectionClass> Connection(new CDiscordConnection(pBot));
		State.Return(Connection);
		return true;

		SDK_ENDTRY;
	});

	/*
	g_ConnectionClass.RegisterFunction("addCommandPrefix", [](Galactic3D::Interfaces::INativeState* pState, int32_t argc, void* pUser) {
		SDK_TRY;

		SDK::State State(pState);

		auto pThis = State.CheckThis<CConnection, g_ConnectionClass>();

		if (pThis->m_pBot == nullptr) {
			return pState->SetError("Discord connection is closed");
		}

		const char* szPrefix = State.CheckString(0);

		g_pBotCommandHandler.add_prefix(szPrefix);

		return true;

		SDK_ENDTRY;
	});
	*/

	/*
	g_ConnectionClass.RegisterFunction("addSlashCommandHandler", [](Galactic3D::Interfaces::INativeState* pState, int32_t argc, void* pUser) {
		SDK_TRY;

		SDK::State State(pState);

		auto pThis = State.CheckThis<CConnection, g_ConnectionClass>();

		if (pThis->m_pBot == nullptr) {
			return pState->SetError("Discord connection is closed");
		}

		const char* szCommandName = State.CheckString(0);
		const char* szDescription = State.CheckString(0);

		dpp::slashcommand newcommand(szCommandName, szDescription, pThis->m_pBot.me.id);

		//g_pBot.global_command_create(
		//	dpp::slashcommand(szCommandName, szDescription, bot.me.id)
		//);

		SDK::ClassValue<CSlashCommandClass, g_ResultClass> Value(new CSlashCommandClass(pResult));
		State.Return(Value);

		return true;

		SDK_ENDTRY;
	});
	*/

	/*
	SDK::g_ConnectionClass.RegisterFunction("addCommandHandler", [](Galactic3D::Interfaces::INativeState* pState, int32_t argc, void* pUser) {
		SDK_TRY;

		SDK::State State(pState);

		const char* szCommandName = State.CheckClass(0);
		const char* szDescription = State.CheckString(0);

		auto pThis = State.CheckThis<CConnection, g_ConnectionClass>();

		if (pThis->m_pBot == nullptr) {
			return pState->SetError("Discord connection is closed");
		}

		pThis->m_pBot.global_command_create(
			dpp::slashcommand(szCommandName, szDescription, pThis->m_pBot.me.id)
		);

		return true;

		SDK_ENDTRY;
	});
	*/

	SDK::RegisterFunction("getTriggerEvent", [](Galactic3D::Interfaces::INativeState* pState, int32_t argc, void* pUser) {
		SDK_TRY;

		SDK::State State(pState);

		Galactic3D::Strong<Galactic3D::Interfaces::IFunction> pFunction;
		GALACTIC_CALL(State.m_pNativeState->CheckFunction(0, &pFunction));

		g_TriggerEventFunction = pFunction;

		return true;

		SDK_ENDTRY;
	});

	g_GuildMemberClass.AddProperty("name", [](Galactic3D::Interfaces::INativeState* pState, int32_t argc, void* pUser) {
		SDK_TRY;

		SDK::State State(pState);

		auto pThis = State.CheckThis<CDiscordGuildMember, g_GuildMemberClass>();
		dpp::guild_member pGuildMember = pThis->m_pGuildMember;
		//if (pGuildMember == nullptr)
		//	return pState->SetError("Guild member is invalid!");

		SDK::StringValue Value(pGuildMember.get_nickname().c_str());
		State.Return(Value);
		return true;

		SDK_ENDTRY;
	});

	g_GuildMemberClass.AddProperty("id", [](Galactic3D::Interfaces::INativeState* pState, int32_t argc, void* pUser) {
		SDK_TRY;

		SDK::State State(pState);

		auto pThis = State.CheckThis<CDiscordGuildMember, g_GuildMemberClass>();
		dpp::guild_member pGuildMember = pThis->m_pGuildMember;
		//if (pGuildMember == nullptr)
		//	return pState->SetError("Guild member is invalid!");

		SDK::NumberValue Value((uint64_t)pGuildMember.user_id);
		State.Return(Value);

		return true;

		SDK_ENDTRY;
	});
}

/*
void OnSocketReceiveData(CSocket2 pSocket, const char* szData) {
	if(g_TriggerEventFunction() == nullptr)
		return;

	SDK::ArrayValue Args;

	SDK::StringValue Name("OnSocketReceive");
	Args.Insert(Name);

	SDK::ClassValue<CSocket2, g_SocketClass> Object(pSocket);
	Args.Insert(Object);

	SDK::ClassValue<CSocket2, g_SocketClass> Object(pSocket);
	Args.Insert(Object);

	SDK::StringValue Data(szData);
	Args.Insert(Data);

	GALACTIC_CALL(g_TriggerEventFunction->Call(Args.m_pArray));
}

void OnSocketClose(CSocket pSocket, const char* szData) {
	if(g_TriggerEventFunction() == nullptr)
		return;

	SDK::ArrayValue Args;

	SDK::StringValue Name("OnSocketClose");
	Args.Insert(Name);

	SDK::ClassValue<CSocket, g_SocketClass> Object(pSocket);
	Args.Insert(Object);

	SDK::ClassValue<CSocket, g_SocketClass> Object(pSocket);
	Args.Insert(Object);

	GALACTIC_CALL(g_TriggerEventFunction->Call(Args.m_pArray));
}
*/

void ModuleUnregister()
{
}