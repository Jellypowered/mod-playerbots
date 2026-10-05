/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include <sstream>
#include <vector>

#include "BattleGroundTactics.h"
#include "Chat.h"
#include "GuildTaskMgr.h"
#include "ObjectAccessor.h"
#include "PerfMonitor.h"
#include "PlayerbotMgr.h"
#include "RandomPlayerbotMgr.h"
#include "ScriptMgr.h"

using namespace Acore::ChatCommands;

class playerbots_commandscript : public CommandScript
{
public:
    playerbots_commandscript() : CommandScript("playerbots_commandscript") {}

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable playerbotsDebugCommandTable = {
            {"bg", HandleDebugBGCommand, SEC_GAMEMASTER, Console::Yes},
        };

        static ChatCommandTable playerbotsAccountCommandTable = {
            {"setKey", HandleSetSecurityKeyCommand, SEC_PLAYER, Console::No},
            {"link", HandleLinkAccountCommand, SEC_PLAYER, Console::No},
            {"linkedAccounts", HandleViewLinkedAccountsCommand, SEC_PLAYER, Console::No},
            {"unlink", HandleUnlinkAccountCommand, SEC_PLAYER, Console::No},
        };

        static ChatCommandTable playerbotsCommandTable = {
            {"bot", HandlePlayerbotCommand, SEC_PLAYER, Console::No},
            {"gtask", HandleGuildTaskCommand, SEC_GAMEMASTER, Console::Yes},
            {"pmon", HandlePerfMonCommand, SEC_GAMEMASTER, Console::Yes},
            {"rndbot", HandleRandomPlayerbotCommand, SEC_GAMEMASTER, Console::Yes},
            {"telemetry", HandleTelemetryCommand, SEC_ADMINISTRATOR, Console::Yes},
            {"debug", playerbotsDebugCommandTable},
            {"account", playerbotsAccountCommandTable},
        };

        static ChatCommandTable commandTable = {
            {"playerbots", playerbotsCommandTable},
        };

        return commandTable;
    }

    static bool HandlePlayerbotCommand(ChatHandler* handler, char const* args)
    {
        return PlayerbotMgr::HandlePlayerbotMgrCommand(handler, args);
    }

    static bool HandleRandomPlayerbotCommand(ChatHandler* handler, char const* args)
    {
        return RandomPlayerbotMgr::HandlePlayerbotConsoleCommand(handler, args);
    }

    static bool HandleGuildTaskCommand(ChatHandler* handler, char const* args)
    {
        return GuildTaskMgr::HandleConsoleCommand(handler, args);
    }

    static bool HandleTelemetryCommand(ChatHandler* handler, char const* args)
    {
        PerfMonitorScope totalScope(sPerfMonitor.acquire(PERF_MON_TOTAL, "Telemetry.Total"));

        struct TelemetryPlayer
        {
            std::string name;
            uint32 mapId;
            uint32 instanceId;
            float positionX;
            float positionY;
            float positionZ;
            float orientation;
            uint32 level;
            uint32 race;
            uint32 playerClass;
            uint32 accountId;
            bool isBot;
            bool alive;
            bool inCombat;
        };

        std::istringstream arguments(args ? args : "");
        uint32 mapId = 0;
        uint32 instanceId = 0;
        bool filterMap = bool(arguments >> mapId);
        bool filterInstance = bool(arguments >> instanceId);
        std::vector<TelemetryPlayer> players;

        {
            std::shared_lock<std::shared_mutex> playerLock(*HashMapHolder<Player>::GetLock(), std::defer_lock);
            {
                PerfMonitorScope lockWaitScope(sPerfMonitor.acquire(PERF_MON_RNDBOT, "Telemetry.LockWait"));
                playerLock.lock();
            }

            PerfMonitorScope snapshotScope(sPerfMonitor.acquire(PERF_MON_RNDBOT, "Telemetry.Snapshot"));
            for (auto const& playerEntry : ObjectAccessor::GetPlayers())
            {
                Player* player = playerEntry.second;
                if (!player || !player->IsInWorld())
                    continue;
                if (filterMap && player->GetMapId() != mapId)
                    continue;
                if (filterInstance && player->GetInstanceId() != instanceId)
                    continue;

                WorldSession* session = player->GetSession();
                if (!session)
                    continue;

                players.push_back({player->GetName(), player->GetMapId(), player->GetInstanceId(),
                                   player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(),
                                   player->GetOrientation(), player->GetLevel(), player->getRace(), player->getClass(),
                                   session->GetAccountId(), PlayerbotsMgr::instance().GetPlayerbotAI(player) != nullptr,
                                   player->IsAlive(), player->IsInCombat()});
            }
        }

        {
            PerfMonitorScope serializeScope(sPerfMonitor.acquire(PERF_MON_RNDBOT, "Telemetry.Serialize"));
            for (TelemetryPlayer const& player : players)
            {
                handler->PSendSysMessage("WMAP|{}|{}|{}|{:.3f}|{:.3f}|{:.3f}|{:.3f}|{}|{}|{}|{}|{}|{}|{}", player.name,
                                         player.mapId, player.instanceId, player.positionX, player.positionY,
                                         player.positionZ, player.orientation, player.level, player.race,
                                         player.playerClass, player.accountId, player.isBot ? 1 : 0,
                                         player.alive ? 1 : 0, player.inCombat ? 1 : 0);
            }
            handler->PSendSysMessage("WMAP_END|{}", players.size());
        }

        return true;
    }

    static bool HandlePerfMonCommand(ChatHandler* /*handler*/, char const* args)
    {
        if (!strcmp(args, "reset"))
        {
            sPerfMonitor.Reset();
            return true;
        }

        if (!strcmp(args, "tick"))
        {
            sPerfMonitor.PrintStats(true, false);
            sPerfMonitor.DumpJson(true);
            return true;
        }

        if (!strcmp(args, "stack"))
        {
            sPerfMonitor.PrintStats(false, true);
            sPerfMonitor.DumpJson(false);
            return true;
        }

        if (!strcmp(args, "toggle"))
        {
            sPlayerbotAIConfig.perfMonEnabled = !sPlayerbotAIConfig.perfMonEnabled;
            if (sPlayerbotAIConfig.perfMonEnabled)
                LOG_INFO("playerbots", "Performance monitor enabled");
            else
                LOG_INFO("playerbots", "Performance monitor disabled");
            return true;
        }

        sPerfMonitor.PrintStats();
        sPerfMonitor.DumpJson(false);
        return true;
    }

    static bool HandleDebugBGCommand(ChatHandler* handler, char const* args)
    {
        return BGTactics::HandleConsoleCommand(handler, args);
    }

    static bool HandleSetSecurityKeyCommand(ChatHandler* handler, char const* args)
    {
        if (!args || !*args)
        {
            handler->PSendSysMessage("Usage: .playerbots account setKey <securityKey>");
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();
        std::string key = args;

        PlayerbotMgr* mgr = PlayerbotsMgr::instance().GetPlayerbotMgr(player);
        if (mgr)
        {
            mgr->HandleSetSecurityKeyCommand(player, key);
            return true;
        }
        else
        {
            handler->PSendSysMessage("PlayerbotMgr instance not found.");
            return false;
        }
    }

    static bool HandleLinkAccountCommand(ChatHandler* handler, char const* args)
    {
        if (!args || !*args)
            return false;

        char* accountName = strtok((char*)args, " ");
        char* key = strtok(nullptr, " ");

        if (!accountName || !key)
        {
            handler->PSendSysMessage("Usage: .playerbots account link <accountName> <securityKey>");
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();

        PlayerbotMgr* mgr = PlayerbotsMgr::instance().GetPlayerbotMgr(player);
        if (mgr)
        {
            mgr->HandleLinkAccountCommand(player, accountName, key);
            return true;
        }
        else
        {
            handler->PSendSysMessage("PlayerbotMgr instance not found.");
            return false;
        }
    }

    static bool HandleViewLinkedAccountsCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();

        PlayerbotMgr* mgr = PlayerbotsMgr::instance().GetPlayerbotMgr(player);
        if (mgr)
        {
            mgr->HandleViewLinkedAccountsCommand(player);
            return true;
        }
        else
        {
            handler->PSendSysMessage("PlayerbotMgr instance not found.");
            return false;
        }
    }

    static bool HandleUnlinkAccountCommand(ChatHandler* handler, char const* args)
    {
        if (!args || !*args)
            return false;

        char* accountName = strtok((char*)args, " ");
        if (!accountName)
        {
            handler->PSendSysMessage("Usage: .playerbots account unlink <accountName>");
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();

        PlayerbotMgr* mgr = PlayerbotsMgr::instance().GetPlayerbotMgr(player);
        if (mgr)
        {
            mgr->HandleUnlinkAccountCommand(player, accountName);
            return true;
        }
        else
        {
            handler->PSendSysMessage("PlayerbotMgr instance not found.");
            return false;
        }
    }
};

void AddPlayerbotsCommandscripts() { new playerbots_commandscript(); }