/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PlayerbotAIConfig.h"
#include <iostream>
#include "Config.h"
#include "NewRpgInfo.h"
#include "PlayerbotDungeonSuggestionMgr.h"
#include "PlayerbotFactory.h"
#include "Playerbots.h"
#include "RandomItemMgr.h"
#include "RandomPlayerbotFactory.h"
#include "RandomPlayerbotMgr.h"
#include "Talentspec.h"

template <class T>
void LoadList(std::string const value, T& list)
{
    std::vector<std::string> ids = split(value, ',');
    for (std::vector<std::string>::iterator i = ids.begin(); i != ids.end(); i++)
    {
        uint32 id = atoi((*i).c_str());
        // if (!id)
        //     continue;
        list.push_back(id);
    }
}

template <class T>
void LoadSet(std::string const value, T& set)
{
    std::vector<std::string> ids = split(value, ',');
    for (std::vector<std::string>::iterator i = ids.begin(); i != ids.end(); i++)
    {
        uint32 id = atoi((*i).c_str());
        // if (!id)
        //     continue;
        set.insert(id);
    }
}

template <class T>
void LoadListString(std::string const value, T& list)
{
    std::vector<std::string> strings = split(value, ',');
    for (std::vector<std::string>::iterator i = strings.begin(); i != strings.end(); i++)
    {
        std::string const string = *i;
        if (string.empty())
            continue;

        list.push_back(string);
    }
}

bool PlayerbotAIConfig::Initialize()
{
    LOG_INFO("server.loading", "Initializing AI Playerbots by ike3, based on the original Playerbots by blueboy");

    enabled = sConfigMgr->GetOption<bool>("AiPlayerbot.Enabled", true);
    if (!enabled)
    {
        LOG_INFO("server.loading", "AI Playerbots is Disabled in aiplayerbot.conf");
        return false;
    }

    globalCoolDown = sConfigMgr->GetOption<int32>("AiPlayerbot.GlobalCooldown", 1500);
    maxWaitForMove = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxWaitForMove", 5000);
    disableMoveSplinePath = sConfigMgr->GetOption<int32>("AiPlayerbot.DisableMoveSplinePath", 0);
    maxMovementSearchTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxMovementSearchTime", 3);
    expireActionTime = sConfigMgr->GetOption<int32>("AiPlayerbot.ExpireActionTime", 5000);
    dispelAuraDuration = sConfigMgr->GetOption<int32>("AiPlayerbot.DispelAuraDuration", 7000);
    reactDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.ReactDelay", 100);
    dynamicReactDelay = sConfigMgr->GetOption<bool>("AiPlayerbot.DynamicReactDelay", true);
    passiveDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.PassiveDelay", 10000);
    repeatDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.RepeatDelay", 2000);
    errorDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.ErrorDelay", 5000);
    rpgDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.RpgDelay", 10000);
    sitDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.SitDelay", 30000);
    returnDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.ReturnDelay", 7000);
    lootDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.LootDelay", 1000);
    disabledWithoutRealPlayerLoginDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.DisabledWithoutRealPlayerLoginDelay", 30);
    disabledWithoutRealPlayerLogoutDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.DisabledWithoutRealPlayerLogoutDelay", 300);

    farDistance = sConfigMgr->GetOption<float>("AiPlayerbot.FarDistance", 20.0f);
    sightDistance = sConfigMgr->GetOption<float>("AiPlayerbot.SightDistance", 75.0f);
    spellDistance = sConfigMgr->GetOption<float>("AiPlayerbot.SpellDistance", 25.0f);
    shootDistance = sConfigMgr->GetOption<float>("AiPlayerbot.ShootDistance", 25.0f);
    healDistance = sConfigMgr->GetOption<float>("AiPlayerbot.HealDistance", 25.0f);
    lootDistance = sConfigMgr->GetOption<float>("AiPlayerbot.LootDistance", 15.0f);
    fleeDistance = sConfigMgr->GetOption<float>("AiPlayerbot.FleeDistance", 7.5f);
    aggroDistance = sConfigMgr->GetOption<float>("AiPlayerbot.AggroDistance", 22.0f);
    tooCloseDistance = sConfigMgr->GetOption<float>("AiPlayerbot.TooCloseDistance", 5.0f);
    meleeDistance = sConfigMgr->GetOption<float>("AiPlayerbot.MeleeDistance", 0.75f);
    followDistance = sConfigMgr->GetOption<float>("AiPlayerbot.FollowDistance", 1.5f);
    whisperDistance = sConfigMgr->GetOption<float>("AiPlayerbot.WhisperDistance", 6000.0f);
    contactDistance = sConfigMgr->GetOption<float>("AiPlayerbot.ContactDistance", 0.5f);
    aoeRadius = sConfigMgr->GetOption<float>("AiPlayerbot.AoeRadius", 5.0f);
    rpgDistance = sConfigMgr->GetOption<float>("AiPlayerbot.RpgDistance", 200.0f);
    grindDistance = sConfigMgr->GetOption<float>("AiPlayerbot.GrindDistance", 75.0f);
    reactDistance = sConfigMgr->GetOption<float>("AiPlayerbot.ReactDistance", 150.0f);

    criticalHealth = sConfigMgr->GetOption<int32>("AiPlayerbot.CriticalHealth", 20);
    lowHealth = sConfigMgr->GetOption<int32>("AiPlayerbot.LowHealth", 45);
    mediumHealth = sConfigMgr->GetOption<int32>("AiPlayerbot.MediumHealth", 65);
    almostFullHealth = sConfigMgr->GetOption<int32>("AiPlayerbot.AlmostFullHealth", 85);
    lowMana = sConfigMgr->GetOption<int32>("AiPlayerbot.LowMana", 15);
    mediumMana = sConfigMgr->GetOption<int32>("AiPlayerbot.MediumMana", 40);
    highMana = sConfigMgr->GetOption<int32>("AiPlayerbot.HighMana", 65);
    autoSaveMana = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoSaveMana", true);
    saveManaThreshold = sConfigMgr->GetOption<int32>("AiPlayerbot.SaveManaThreshold", 60);
    autoAvoidAoe = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoAvoidAoe", true);
    maxAoeAvoidRadius = sConfigMgr->GetOption<float>("AiPlayerbot.MaxAoeAvoidRadius", 15.0f);
    LoadSet<std::set<uint32>>(sConfigMgr->GetOption<std::string>("AiPlayerbot.AoeAvoidSpellWhitelist", "50759,57491,13810"),
                              aoeAvoidSpellWhitelist);
    tellWhenAvoidAoe = sConfigMgr->GetOption<bool>("AiPlayerbot.TellWhenAvoidAoe", false);

    randomGearLoweringChance = sConfigMgr->GetOption<float>("AiPlayerbot.RandomGearLoweringChance", 0.0f);
    incrementalGearInit = sConfigMgr->GetOption<bool>("AiPlayerbot.IncrementalGearInit", true);
    randomGearQualityLimit = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomGearQualityLimit", 3);
    randomGearScoreLimit = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomGearScoreLimit", 0);

    randomBotMinLevelChance = sConfigMgr->GetOption<float>("AiPlayerbot.RandomBotMinLevelChance", 0.1f);
    randomBotMaxLevelChance = sConfigMgr->GetOption<float>("AiPlayerbot.RandomBotMaxLevelChance", 0.1f);
    randomBotRpgChance = sConfigMgr->GetOption<float>("AiPlayerbot.RandomBotRpgChance", 0.20f);

    iterationsPerTick = sConfigMgr->GetOption<int32>("AiPlayerbot.IterationsPerTick", 100);

    allowAccountBots = sConfigMgr->GetOption<bool>("AiPlayerbot.AllowAccountBots", true);
    allowGuildBots = sConfigMgr->GetOption<bool>("AiPlayerbot.AllowGuildBots", true);
    allowTrustedAccountBots = sConfigMgr->GetOption<bool>("AiPlayerbot.AllowTrustedAccountBots", true);
    disabledWithoutRealPlayer = sConfigMgr->GetOption<bool>("AiPlayerbot.DisabledWithoutRealPlayer", false);
    randomBotGuildNearby = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotGuildNearby", false);
    randomBotInvitePlayer = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotInvitePlayer", false);
    inviteChat = sConfigMgr->GetOption<bool>("AiPlayerbot.InviteChat", false);

    randomBotMapsAsString = sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotMaps", "0,1,530,571");
    LoadList<std::vector<uint32>>(randomBotMapsAsString, randomBotMaps);
    probTeleToBankers = sConfigMgr->GetOption<float>("AiPlayerbot.ProbTeleToBankers", 0.25f);
    LoadList<std::vector<uint32>>(
        sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotQuestItems",
                                           "6948,5175,5176,5177,5178,16309,12382,13704,11000"),
        randomBotQuestItems);
    LoadList<std::vector<uint32>>(sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotSpellIds", "54197"),
                                  randomBotSpellIds);
    LoadList<std::vector<uint32>>(
        sConfigMgr->GetOption<std::string>("AiPlayerbot.PvpProhibitedZoneIds",
                                           "2255,656,2361,2362,2363,976,35,2268,3425,392,541,1446,3828,3712,3738,3565,"
                                           "3539,3623,4152,3988,4658,4284,4418,4436,4275,4323,4395,3703,4298,139,3951"),
        pvpProhibitedZoneIds);
    LoadList<std::vector<uint32>>(sConfigMgr->GetOption<std::string>("AiPlayerbot.PvpProhibitedAreaIds", "976,35"),
                                  pvpProhibitedAreaIds);
    fastReactInBG = sConfigMgr->GetOption<bool>("AiPlayerbot.FastReactInBG", true);
    LoadList<std::vector<uint32>>(
        sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotQuestIds", "7848,3802,5505,6502,7761"),
        randomBotQuestIds);

    LoadSet<std::set<uint32>>(sConfigMgr->GetOption<std::string>("AiPlayerbot.DisallowedGameObjects", "176213,17155"),
                              disallowedGameObjects);
    botAutologin = sConfigMgr->GetOption<bool>("AiPlayerbot.BotAutologin", false);
    randomBotAutologin = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotAutologin", true);
    minRandomBots = sConfigMgr->GetOption<int32>("AiPlayerbot.MinRandomBots", 50);
    maxRandomBots = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxRandomBots", 50);
    randomBotUpdateInterval = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotUpdateInterval", 20);
    randomBotCountChangeMinInterval =
        sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotCountChangeMinInterval", 30 * MINUTE);
    randomBotCountChangeMaxInterval =
        sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotCountChangeMaxInterval", 2 * HOUR);
    minRandomBotInWorldTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MinRandomBotInWorldTime", 2 * HOUR);
    maxRandomBotInWorldTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxRandomBotInWorldTime", 14 * 24 * HOUR);
    minRandomBotRandomizeTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MinRandomBotRandomizeTime", 2 * HOUR);
    maxRandomBotRandomizeTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxRandomBotRandomizeTime", 14 * 24 * HOUR);
    minRandomBotChangeStrategyTime =
        sConfigMgr->GetOption<int32>("AiPlayerbot.MinRandomBotChangeStrategyTime", 30 * MINUTE);
    maxRandomBotChangeStrategyTime =
        sConfigMgr->GetOption<int32>("AiPlayerbot.MaxRandomBotChangeStrategyTime", 2 * HOUR);
    minRandomBotReviveTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MinRandomBotReviveTime", MINUTE);
    maxRandomBotReviveTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxRandomBotReviveTime", 5 * MINUTE);
    minRandomBotTeleportInterval = sConfigMgr->GetOption<int32>("AiPlayerbot.MinRandomBotTeleportInterval", 1 * HOUR);
    maxRandomBotTeleportInterval = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxRandomBotTeleportInterval", 5 * HOUR);
    permanantlyInWorldTime =
        sConfigMgr->GetOption<int32>("AiPlayerbot.PermanantlyInWorldTime", 1 * YEAR);
    randomBotTeleportDistance = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotTeleportDistance", 100);
    randomBotsPerInterval = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotsPerInterval", 60);
    minRandomBotsPriceChangeInterval =
        sConfigMgr->GetOption<int32>("AiPlayerbot.MinRandomBotsPriceChangeInterval", 2 * HOUR);
    maxRandomBotsPriceChangeInterval =
        sConfigMgr->GetOption<int32>("AiPlayerbot.MaxRandomBotsPriceChangeInterval", 48 * HOUR);
    randomBotJoinLfg = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotJoinLfg", true);

    restrictHealerDPS = sConfigMgr->GetOption<bool>("AiPlayerbot.HealerDPSMapRestriction", false);
    LoadList<std::vector<uint32>>(
        sConfigMgr->GetOption<std::string>("AiPlayerbot.RestrictedHealerDPSMaps",
                                             "33,34,36,43,47,48,70,90,109,129,209,229,230,329,349,389,429,1001,1004,"
                                             "1007,269,540,542,543,545,546,547,552,553,554,555,556,557,558,560,585,574,"
                                             "575,576,578,595,599,600,601,602,604,608,619,632,650,658,668,409,469,509,"
                                             "531,532,534,544,548,550,564,565,580,249,533,603,615,616,624,631,649,724"),
        restrictedHealerDPSMaps);
	
	//////////////////////////// ICC

	EnableICCBuffs = sConfigMgr->GetOption<bool>("AiPlayerbot.EnableICCBuffs", true);

    //////////////////////////// CHAT
    enableBroadcasts = sConfigMgr->GetOption<bool>("AiPlayerbot.EnableBroadcasts", true);
    randomBotTalk = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotTalk", false);
    randomBotEmote = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotEmote", false);
    randomBotSuggestDungeons = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotSuggestDungeons", true);
    randomBotSayWithoutMaster = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotSayWithoutMaster", false);

    // broadcastChanceMaxValue is used in urand(1, broadcastChanceMaxValue) for broadcasts,
    // lowering it will increase the chance, setting it to 0 will disable broadcasts
    // for internal use, not intended to be change by the user
    broadcastChanceMaxValue = enableBroadcasts ? 30000 : 0;

    // all broadcast chances should be in range 1-broadcastChanceMaxValue, value of 0 will disable this particular
    // broadcast setting value to max does not guarantee the broadcast, as there are some internal randoms as well
    broadcastToGuildGlobalChance = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastToGuildGlobalChance", 30000);
    broadcastToWorldGlobalChance = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastToWorldGlobalChance", 30000);
    broadcastToGeneralGlobalChance = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastToGeneralGlobalChance", 30000);
    broadcastToTradeGlobalChance = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastToTradeGlobalChance", 30000);
    broadcastToLFGGlobalChance = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastToLFGGlobalChance", 30000);
    broadcastToLocalDefenseGlobalChance =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastToLocalDefenseGlobalChance", 30000);
    broadcastToWorldDefenseGlobalChance =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastToWorldDefenseGlobalChance", 30000);
    broadcastToGuildRecruitmentGlobalChance =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastToGuildRecruitmentGlobalChance", 30000);

    broadcastChanceLootingItemPoor = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLootingItemPoor", 30);
    broadcastChanceLootingItemNormal =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLootingItemNormal", 300);
    broadcastChanceLootingItemUncommon =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLootingItemUncommon", 10000);
    broadcastChanceLootingItemRare = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLootingItemRare", 20000);
    broadcastChanceLootingItemEpic = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLootingItemEpic", 30000);
    broadcastChanceLootingItemLegendary =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLootingItemLegendary", 30000);
    broadcastChanceLootingItemArtifact =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLootingItemArtifact", 30000);

    broadcastChanceQuestAccepted = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceQuestAccepted", 6000);
    broadcastChanceQuestUpdateObjectiveCompleted =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceQuestUpdateObjectiveCompleted", 300);
    broadcastChanceQuestUpdateObjectiveProgress =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceQuestUpdateObjectiveProgress", 300);
    broadcastChanceQuestUpdateFailedTimer =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceQuestUpdateFailedTimer", 300);
    broadcastChanceQuestUpdateComplete =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceQuestUpdateComplete", 1000);
    broadcastChanceQuestTurnedIn = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceQuestTurnedIn", 10000);

    broadcastChanceKillNormal = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceKillNormal", 30);
    broadcastChanceKillElite = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceKillElite", 300);
    broadcastChanceKillRareelite = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceKillRareelite", 3000);
    broadcastChanceKillWorldboss = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceKillWorldboss", 20000);
    broadcastChanceKillRare = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceKillRare", 10000);
    broadcastChanceKillUnknown = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceKillUnknown", 100);
    broadcastChanceKillPet = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceKillPet", 10);
    broadcastChanceKillPlayer = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceKillPlayer", 30);

    broadcastChanceLevelupGeneric = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLevelupGeneric", 20000);
    broadcastChanceLevelupTenX = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLevelupTenX", 30000);
    broadcastChanceLevelupMaxLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLevelupMaxLevel", 30000);

    broadcastChanceSuggestInstance = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceSuggestInstance", 5000);
    broadcastChanceSuggestQuest = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceSuggestQuest", 10000);
    broadcastChanceSuggestGrindMaterials =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceSuggestGrindMaterials", 5000);
    broadcastChanceSuggestGrindReputation =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceSuggestGrindReputation", 5000);
    broadcastChanceSuggestSell = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceSuggestSell", 300);
    broadcastChanceSuggestSomething =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceSuggestSomething", 30000);

    broadcastChanceSuggestSomethingToxic =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceSuggestSomethingToxic", 0);

    broadcastChanceSuggestToxicLinks = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceSuggestToxicLinks", 0);
    toxicLinksPrefix = sConfigMgr->GetOption<std::string>("AiPlayerbot.ToxicLinksPrefix", "gnomes");

    broadcastChanceSuggestThunderfury =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceSuggestThunderfury", 1);

    // does not depend on global chance
    broadcastChanceGuildManagement = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceGuildManagement", 30000);
    ////////////////////////////

    toxicLinksRepliesChance = sConfigMgr->GetOption<int32>("AiPlayerbot.ToxicLinksRepliesChance", 30);    // 0-100
    thunderfuryRepliesChance = sConfigMgr->GetOption<int32>("AiPlayerbot.ThunderfuryRepliesChance", 40);  // 0-100
    guildRepliesRate = sConfigMgr->GetOption<int32>("AiPlayerbot.GuildRepliesRate", 100);                 // 0-100
    suggestDungeonsInLowerCaseRandomly =
        sConfigMgr->GetOption<bool>("AiPlayerbot.SuggestDungeonsInLowerCaseRandomly", false);

    ////////////////////////// !CHAT

    randomBotJoinBG = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotJoinBG", true);
    randomBotAutoJoinBG = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotAutoJoinBG", false);

    randomBotAutoJoinArenaBracket = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAutoJoinArenaBracket", 7);

    randomBotAutoJoinICBrackets = sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotAutoJoinICBrackets", "0,1");
    randomBotAutoJoinEYBrackets = sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotAutoJoinEYBrackets", "0,1,2");
    randomBotAutoJoinAVBrackets = sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotAutoJoinAVBrackets", "0,1,2,3");
    randomBotAutoJoinABBrackets = sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotAutoJoinABBrackets", "0,1,2,3,4,5,6");
    randomBotAutoJoinWSBrackets = sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotAutoJoinWSBrackets", "0,1,2,3,4,5,6,7");

    randomBotAutoJoinBGICCount = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAutoJoinBGICCount", 0);
    randomBotAutoJoinBGEYCount = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAutoJoinBGEYCount", 0);
    randomBotAutoJoinBGAVCount = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAutoJoinBGAVCount", 0);
    randomBotAutoJoinBGABCount = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAutoJoinBGABCount", 0);
    randomBotAutoJoinBGWSCount = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAutoJoinBGWSCount", 0);

    randomBotAutoJoinBGRatedArena2v2Count =
        sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAutoJoinBGRatedArena2v2Count", 0);
    randomBotAutoJoinBGRatedArena3v3Count =
        sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAutoJoinBGRatedArena3v3Count", 0);
    randomBotAutoJoinBGRatedArena5v5Count =
        sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAutoJoinBGRatedArena5v5Count", 0);
    logInGroupOnly = sConfigMgr->GetOption<bool>("AiPlayerbot.LogInGroupOnly", true);
    logValuesPerTick = sConfigMgr->GetOption<bool>("AiPlayerbot.LogValuesPerTick", false);
    fleeingEnabled = sConfigMgr->GetOption<bool>("AiPlayerbot.FleeingEnabled", true);
    summonAtInnkeepersEnabled = sConfigMgr->GetOption<bool>("AiPlayerbot.SummonAtInnkeepersEnabled", true);
    randomBotMinLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotMinLevel", 1);
    randomBotMaxLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotMaxLevel", 80);
    if (randomBotMaxLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
        randomBotMaxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);
    randomBotLoginAtStartup = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotLoginAtStartup", true);
    randomBotTeleLowerLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotTeleLowerLevel", 1);
    randomBotTeleHigherLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotTeleHigherLevel", 3);
    openGoSpell = sConfigMgr->GetOption<int32>("AiPlayerbot.OpenGoSpell", 6477);

    // Zones for NewRpgStrategy teleportation brackets
    std::vector<uint32> zoneIds = {
        // Classic WoW - Low-level zones
        1, 12, 14, 85, 141, 215, 3430, 3524,
        // Classic WoW - Mid-level zones
        17, 38, 40, 130, 148, 3433, 3525,
        // Classic WoW - High-level zones
        10, 11, 44, 267, 331, 400, 406,
        // Classic WoW - Higher-level zones
        3, 8, 15, 16, 33, 45, 47, 51, 357, 405, 440,
        // Classic WoW - Top-level zones
        4, 28, 46, 139, 361, 490, 618, 1377,
        // The Burning Crusade - Zones
        3483, 3518, 3519, 3520, 3521, 3522, 3523, 4080,
        // Wrath of the Lich King - Zones
        65, 66, 67, 210, 394, 495, 2817, 3537, 3711, 4197
    };

    for (uint32 zoneId : zoneIds)
    {
        std::string setting = "AiPlayerbot.ZoneBracket." + std::to_string(zoneId);
        std::string value = sConfigMgr->GetOption<std::string>(setting, "");

        if (!value.empty())
        {
            size_t commaPos = value.find(',');
            if (commaPos != std::string::npos)
            {
                uint32 minLevel = atoi(value.substr(0, commaPos).c_str());
                uint32 maxLevel = atoi(value.substr(commaPos + 1).c_str());
                zoneBrackets[zoneId] = std::make_pair(minLevel, maxLevel);
            }
        }
    }

    randomChangeMultiplier = sConfigMgr->GetOption<float>("AiPlayerbot.RandomChangeMultiplier", 1.0);

    randomBotCombatStrategies = sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotCombatStrategies", "-threat");
    randomBotNonCombatStrategies = sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotNonCombatStrategies", "");
    combatStrategies = sConfigMgr->GetOption<std::string>("AiPlayerbot.CombatStrategies", "+custom::say");
    nonCombatStrategies = sConfigMgr->GetOption<std::string>("AiPlayerbot.NonCombatStrategies", "+custom::say,+return");
    applyInstanceStrategies = sConfigMgr->GetOption<bool>("AiPlayerbot.ApplyInstanceStrategies", true);

    commandPrefix = sConfigMgr->GetOption<std::string>("AiPlayerbot.CommandPrefix", "");
    commandSeparator = sConfigMgr->GetOption<std::string>("AiPlayerbot.CommandSeparator", "\\\\");

    commandServerPort = sConfigMgr->GetOption<int32>("AiPlayerbot.CommandServerPort", 8888);
    perfMonEnabled = sConfigMgr->GetOption<bool>("AiPlayerbot.PerfMonEnabled", false);

    useGroundMountAtMinLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.UseGroundMountAtMinLevel", 20);
    useFastGroundMountAtMinLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.UseFastGroundMountAtMinLevel", 40);
    useFlyMountAtMinLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.UseFlyMountAtMinLevel", 60);
    useFastFlyMountAtMinLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.UseFastFlyMountAtMinLevel", 70);

    // stagger bot flightpath takeoff
    delayMin = sConfigMgr->GetOption<uint32>("AiPlayerbot.BotTaxiDelayMinMs", 350u);
    delayMax = sConfigMgr->GetOption<uint32>("AiPlayerbot.BotTaxiDelayMaxMs", 5000u);
    gapMs = sConfigMgr->GetOption<uint32>("AiPlayerbot.BotTaxiGapMs", 200u);
    gapJitterMs = sConfigMgr->GetOption<uint32>("AiPlayerbot.BotTaxiGapJitterMs", 100u);

    LOG_INFO("server.loading", "Loading TalentSpecs...");

    for (uint32 cls = 1; cls < MAX_CLASSES; ++cls)
    {
        if (cls == 10)
        {
            continue;
        }
        for (uint32 spec = 0; spec < MAX_SPECNO; ++spec)
        {
            std::ostringstream os;
            os << "AiPlayerbot.PremadeSpecName." << cls << "." << spec;
            premadeSpecName[cls][spec] = sConfigMgr->GetOption<std::string>(os.str().c_str(), "", false);
            os.str("");
            os.clear();
            os << "AiPlayerbot.PremadeSpecGlyph." << cls << "." << spec;
            premadeSpecGlyph[cls][spec] = sConfigMgr->GetOption<std::string>(os.str().c_str(), "", false);
            std::vector<std::string> splitSpecGlyph = split(premadeSpecGlyph[cls][spec], ',');
            for (std::string& split : splitSpecGlyph)
            {
                if (split.size() != 0)
                {
                    parsedSpecGlyph[cls][spec].push_back(atoi(split.c_str()));
                }
            }
            for (uint32 level = 0; level < MAX_LEVEL; ++level)
            {
                std::ostringstream os;
                os << "AiPlayerbot.PremadeSpecLink." << cls << "." << spec << "." << level;
                premadeSpecLink[cls][spec][level] = sConfigMgr->GetOption<std::string>(os.str().c_str(), "", false);
                parsedSpecLinkOrder[cls][spec][level] = ParseTempTalentsOrder(cls, premadeSpecLink[cls][spec][level]);
            }
        }
        for (uint32 spec = 0; spec < 3; ++spec)
        {
            for (uint32 points = 0; points < 21; ++points)
            {
                std::ostringstream os;
                os << "AiPlayerbot.PremadeHunterPetLink." << spec << "." << points;
                premadeHunterPetLink[spec][points] = sConfigMgr->GetOption<std::string>(os.str().c_str(), "", false);
                parsedHunterPetLinkOrder[spec][points] =
                    ParseTempPetTalentsOrder(spec, premadeHunterPetLink[spec][points]);
            }
        }
        for (uint32 spec = 0; spec < MAX_SPECNO; ++spec)
        {
            std::ostringstream os;
            os << "AiPlayerbot.RandomClassSpecProb." << cls << "." << spec;
            uint32 def;
            if (spec <= 1)
                def = 33;
            else if (spec == 2)
                def = 34;
            else
                def = 0;
            randomClassSpecProb[cls][spec] = sConfigMgr->GetOption<uint32>(os.str().c_str(), def, false);
            os.str("");
            os.clear();
            os << "AiPlayerbot.RandomClassSpecIndex." << cls << "." << spec;
            randomClassSpecIndex[cls][spec] = sConfigMgr->GetOption<uint32>(os.str().c_str(), spec, false);
        }
    }

    botCheats.clear();
    LoadListString<std::vector<std::string>>(sConfigMgr->GetOption<std::string>("AiPlayerbot.BotCheats", "taxi,raid"),
                                             botCheats);

    botCheatMask = 0;

    if (std::find(botCheats.begin(), botCheats.end(), "taxi") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::taxi;
    if (std::find(botCheats.begin(), botCheats.end(), "gold") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::gold;
    if (std::find(botCheats.begin(), botCheats.end(), "health") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::health;
    if (std::find(botCheats.begin(), botCheats.end(), "mana") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::mana;
    if (std::find(botCheats.begin(), botCheats.end(), "power") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::power;
    if (std::find(botCheats.begin(), botCheats.end(), "raid") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::raid;

    LoadListString<std::vector<std::string>>(sConfigMgr->GetOption<std::string>("AiPlayerbot.AllowedLogFiles", ""),
                                             allowedLogFiles);
    LoadListString<std::vector<std::string>>(sConfigMgr->GetOption<std::string>("AiPlayerbot.TradeActionExcludedPrefixes", ""),
                                             tradeActionExcludedPrefixes);

    worldBuffs.clear();
    loadWorldBuff();
    LOG_INFO("playerbots", "Loading World Buff Feature...");

    randomBotAccountPrefix = sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotAccountPrefix", "rndbot");
    randomBotAccountCount = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAccountCount", 0);
    deleteRandomBotAccounts = sConfigMgr->GetOption<bool>("AiPlayerbot.DeleteRandomBotAccounts", false);
    randomBotGuildCount = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotGuildCount", 20);
    deleteRandomBotGuilds = sConfigMgr->GetOption<bool>("AiPlayerbot.DeleteRandomBotGuilds", false);

    guildTaskEnabled = sConfigMgr->GetOption<bool>("AiPlayerbot.EnableGuildTasks", true);
    minGuildTaskChangeTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MinGuildTaskChangeTime", 3 * 24 * 3600);
    maxGuildTaskChangeTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxGuildTaskChangeTime", 4 * 24 * 3600);
    minGuildTaskAdvertisementTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MinGuildTaskAdvertisementTime", 300);
    maxGuildTaskAdvertisementTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxGuildTaskAdvertisementTime", 12 * 3600);
    minGuildTaskRewardTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MinGuildTaskRewardTime", 300);
    maxGuildTaskRewardTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxGuildTaskRewardTime", 3600);
    guildTaskAdvertCleanupTime = sConfigMgr->GetOption<int32>("AiPlayerbot.GuildTaskAdvertCleanupTime", 300);
    guildTaskKillTaskDistance = sConfigMgr->GetOption<int32>("AiPlayerbot.GuildTaskKillTaskDistance", 2000);
    targetPosRecalcDistance = sConfigMgr->GetOption<float>("AiPlayerbot.TargetPosRecalcDistance", 0.1f);

    // cosmetics (by lidocain)
    randomBotShowCloak = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotShowCloak", true);
    randomBotShowHelmet = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotShowHelmet", true);

    // SPP switches
    enableGreet = sConfigMgr->GetOption<bool>("AiPlayerbot.EnableGreet", true);
    summonWhenGroup = sConfigMgr->GetOption<bool>("AiPlayerbot.SummonWhenGroup", true);
    randomBotFixedLevel = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotFixedLevel", false);
    disableRandomLevels = sConfigMgr->GetOption<bool>("AiPlayerbot.DisableRandomLevels", false);
    randomBotRandomPassword = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotRandomPassword", true);
    downgradeMaxLevelBot = sConfigMgr->GetOption<bool>("AiPlayerbot.DowngradeMaxLevelBot", true);
    equipmentPersistence = sConfigMgr->GetOption<bool>("AiPlayerbot.EquipmentPersistence", false);
    equipmentPersistenceLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.EquipmentPersistenceLevel", 80);
    groupInvitationPermission = sConfigMgr->GetOption<int32>("AiPlayerbot.GroupInvitationPermission", 1);
    keepAltsInGroup = sConfigMgr->GetOption<bool>("AiPlayerbot.KeepAltsInGroup", false);
    allowSummonInCombat = sConfigMgr->GetOption<bool>("AiPlayerbot.AllowSummonInCombat", true);
    allowSummonWhenMasterIsDead = sConfigMgr->GetOption<bool>("AiPlayerbot.AllowSummonWhenMasterIsDead", true);
    allowSummonWhenBotIsDead = sConfigMgr->GetOption<bool>("AiPlayerbot.AllowSummonWhenBotIsDead", true);
    reviveBotWhenSummoned = sConfigMgr->GetOption<int32>("AiPlayerbot.ReviveBotWhenSummoned", 1);
    botRepairWhenSummon = sConfigMgr->GetOption<bool>("AiPlayerbot.BotRepairWhenSummon", true);
    autoInitOnly = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoInitOnly", false);
    autoInitEquipLevelLimitRatio = sConfigMgr->GetOption<float>("AiPlayerbot.AutoInitEquipLevelLimitRatio", 1.0);

    maxAddedBots = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxAddedBots", 40);
    addClassCommand = sConfigMgr->GetOption<int32>("AiPlayerbot.AddClassCommand", 1);
    addClassAccountPoolSize = sConfigMgr->GetOption<int32>("AiPlayerbot.AddClassAccountPoolSize", 50);
    maintenanceCommand = sConfigMgr->GetOption<int32>("AiPlayerbot.MaintenanceCommand", 1);
    autoGearCommand = sConfigMgr->GetOption<int32>("AiPlayerbot.AutoGearCommand", 1);
    autoGearCommandAltBots = sConfigMgr->GetOption<int32>("AiPlayerbot.AutoGearCommandAltBots", 1);
    autoGearQualityLimit = sConfigMgr->GetOption<int32>("AiPlayerbot.AutoGearQualityLimit", 3);
    autoGearScoreLimit = sConfigMgr->GetOption<int32>("AiPlayerbot.AutoGearScoreLimit", 0);

    randomBotXPRate = sConfigMgr->GetOption<float>("AiPlayerbot.RandomBotXPRate", 1.0);
    randomBotAllianceRatio = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAllianceRatio", 50);
    randomBotHordeRatio = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotHordeRatio", 50);
    disableDeathKnightLogin = sConfigMgr->GetOption<bool>("AiPlayerbot.DisableDeathKnightLogin", 0);
    limitTalentsExpansion = sConfigMgr->GetOption<bool>("AiPlayerbot.LimitTalentsExpansion", 0);
    botActiveAlone = sConfigMgr->GetOption<int32>("AiPlayerbot.BotActiveAlone", 100);
    BotActiveAloneForceWhenInRadius = sConfigMgr->GetOption<uint32>("AiPlayerbot.BotActiveAloneForceWhenInRadius", 150);
    BotActiveAloneForceWhenInZone = sConfigMgr->GetOption<bool>("AiPlayerbot.BotActiveAloneForceWhenInZone", 1);
    BotActiveAloneForceWhenInMap = sConfigMgr->GetOption<bool>("AiPlayerbot.BotActiveAloneForceWhenInMap", 0);
    BotActiveAloneForceWhenIsFriend = sConfigMgr->GetOption<bool>("AiPlayerbot.BotActiveAloneForceWhenIsFriend", 1);
    BotActiveAloneForceWhenInGuild = sConfigMgr->GetOption<bool>("AiPlayerbot.BotActiveAloneForceWhenInGuild", 1);
    botActiveAloneSmartScale = sConfigMgr->GetOption<bool>("AiPlayerbot.botActiveAloneSmartScale", 1);
    botActiveAloneSmartScaleDiffLimitfloor = sConfigMgr->GetOption<uint32>("AiPlayerbot.botActiveAloneSmartScaleDiffLimitfloor", 50);
    botActiveAloneSmartScaleDiffLimitCeiling = sConfigMgr->GetOption<uint32>("AiPlayerbot.botActiveAloneSmartScaleDiffLimitCeiling", 200);
    botActiveAloneSmartScaleWhenMinLevel = sConfigMgr->GetOption<uint32>("AiPlayerbot.botActiveAloneSmartScaleWhenMinLevel", 1);
    botActiveAloneSmartScaleWhenMaxLevel = sConfigMgr->GetOption<uint32>("AiPlayerbot.botActiveAloneSmartScaleWhenMaxLevel", 80);

    randombotsWalkingRPG = sConfigMgr->GetOption<bool>("AiPlayerbot.RandombotsWalkingRPG", false);
    randombotsWalkingRPGInDoors = sConfigMgr->GetOption<bool>("AiPlayerbot.RandombotsWalkingRPG.InDoors", false);
    minEnchantingBotLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.MinEnchantingBotLevel", 60);
    limitEnchantExpansion = sConfigMgr->GetOption<int32>("AiPlayerbot.LimitEnchantExpansion", 1);
    limitGearExpansion = sConfigMgr->GetOption<int32>("AiPlayerbot.LimitGearExpansion", 1);
    randombotStartingLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.RandombotStartingLevel", 5);
    enablePeriodicOnlineOffline = sConfigMgr->GetOption<bool>("AiPlayerbot.EnablePeriodicOnlineOffline", false);
    enableRandomBotTrading = sConfigMgr->GetOption<int32>("AiPlayerbot.EnableRandomBotTrading", 1);
    periodicOnlineOfflineRatio = sConfigMgr->GetOption<float>("AiPlayerbot.PeriodicOnlineOfflineRatio", 2.0);
    gearscorecheck = sConfigMgr->GetOption<bool>("AiPlayerbot.GearScoreCheck", false);
    randomBotPreQuests = sConfigMgr->GetOption<bool>("AiPlayerbot.PreQuests", true);

    // SPP automation
    freeMethodLoot = sConfigMgr->GetOption<bool>("AiPlayerbot.FreeMethodLoot", false);
    lootRollLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.LootRollLevel", 1);
    autoPickReward = sConfigMgr->GetOption<std::string>("AiPlayerbot.AutoPickReward", "yes");
    autoEquipUpgradeLoot = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoEquipUpgradeLoot", true);
    equipUpgradeThreshold = sConfigMgr->GetOption<float>("AiPlayerbot.EquipUpgradeThreshold", 1.1f);
    twoRoundsGearInit = sConfigMgr->GetOption<bool>("AiPlayerbot.TwoRoundsGearInit", false);
    syncQuestWithPlayer = sConfigMgr->GetOption<bool>("AiPlayerbot.SyncQuestWithPlayer", true);
    syncQuestForPlayer = sConfigMgr->GetOption<bool>("AiPlayerbot.SyncQuestForPlayer", false);
    dropObsoleteQuests = sConfigMgr->GetOption<bool>("AiPlayerbot.DropObsoleteQuests", true);
    autoTrainSpells = sConfigMgr->GetOption<std::string>("AiPlayerbot.AutoTrainSpells", "yes");
    autoPickTalents = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoPickTalents", true);
    autoUpgradeEquip = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoUpgradeEquip", false);
    hunterWolfPet = sConfigMgr->GetOption<int32>("AiPlayerbot.HunterWolfPet", 0);
    autoLearnTrainerSpells = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoLearnTrainerSpells", true);
    autoLearnQuestSpells = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoLearnQuestSpells", false);
    autoTeleportForLevel = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoTeleportForLevel", false);
    autoDoQuests = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoDoQuests", true);
    enableNewRpgStrategy = sConfigMgr->GetOption<bool>("AiPlayerbot.EnableNewRpgStrategy", true);

    RpgStatusProbWeight[RPG_WANDER_RANDOM] = sConfigMgr->GetOption<int32>("AiPlayerbot.RpgStatusProbWeight.WanderRandom", 15);
    RpgStatusProbWeight[RPG_WANDER_NPC] = sConfigMgr->GetOption<int32>("AiPlayerbot.RpgStatusProbWeight.WanderNpc", 20);
    RpgStatusProbWeight[RPG_GO_GRIND] = sConfigMgr->GetOption<int32>("AiPlayerbot.RpgStatusProbWeight.GoGrind", 15);
    RpgStatusProbWeight[RPG_GO_CAMP] = sConfigMgr->GetOption<int32>("AiPlayerbot.RpgStatusProbWeight.GoCamp", 10);
    RpgStatusProbWeight[RPG_DO_QUEST] = sConfigMgr->GetOption<int32>("AiPlayerbot.RpgStatusProbWeight.DoQuest", 60);
    RpgStatusProbWeight[RPG_TRAVEL_FLIGHT] = sConfigMgr->GetOption<int32>("AiPlayerbot.RpgStatusProbWeight.TravelFlight", 15);
    RpgStatusProbWeight[RPG_REST] = sConfigMgr->GetOption<int32>("AiPlayerbot.RpgStatusProbWeight.Rest", 5);

    syncLevelWithPlayers = sConfigMgr->GetOption<bool>("AiPlayerbot.SyncLevelWithPlayers", false);
    freeFood = sConfigMgr->GetOption<bool>("AiPlayerbot.FreeFood", true);
    randomBotGroupNearby = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotGroupNearby", true);

    // arena
    randomBotArenaTeam2v2Count = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotArenaTeam2v2Count", 10);
    randomBotArenaTeam3v3Count = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotArenaTeam3v3Count", 10);
    randomBotArenaTeam5v5Count = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotArenaTeam5v5Count", 5);
    deleteRandomBotArenaTeams = sConfigMgr->GetOption<bool>("AiPlayerbot.DeleteRandomBotArenaTeams", false);
    randomBotArenaTeamMaxRating = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotArenaTeamMaxRating", 2000);
    randomBotArenaTeamMinRating = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotArenaTeamMinRating", 1000);

    selfBotLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.SelfBotLevel", 1);

    RandomPlayerbotFactory::CreateRandomBots();
    if (World::IsStopped())
    {
        return true;
    }

    // Assign account types after accounts are created
    sRandomPlayerbotMgr->AssignAccountTypes();

    if (sPlayerbotAIConfig->enabled)
    {
        sRandomPlayerbotMgr->Init();
    }

    sRandomItemMgr->Init();
    sRandomItemMgr->InitAfterAhBot();
    sPlayerbotTextMgr->LoadBotTexts();
    sPlayerbotTextMgr->LoadBotTextChance();
    PlayerbotFactory::Init();

    AiObjectContext::BuildAllSharedContexts();

    if (sPlayerbotAIConfig->randomBotSuggestDungeons)
    {
        sPlayerbotDungeonSuggestionMgr->LoadDungeonSuggestions();
    }

    excludedHunterPetFamilies.clear();
    LoadList<std::vector<uint32>>(sConfigMgr->GetOption<std::string>("AiPlayerbot.ExcludedHunterPetFamilies", ""), excludedHunterPetFamilies);

    LOG_INFO("server.loading", "---------------------------------------");
    LOG_INFO("server.loading", "        AI Playerbots initialized       ");
    LOG_INFO("server.loading", "---------------------------------------");

    return true;
}

bool PlayerbotAIConfig::IsInRandomAccountList(uint32 id)
{
    return find(randomBotAccounts.begin(), randomBotAccounts.end(), id) != randomBotAccounts.end();
}

bool PlayerbotAIConfig::IsInRandomQuestItemList(uint32 id)
{
    return find(randomBotQuestItems.begin(), randomBotQuestItems.end(), id) != randomBotQuestItems.end();
}

bool PlayerbotAIConfig::IsPvpProhibited(uint32 zoneId, uint32 areaId)
{
    return IsInPvpProhibitedZone(zoneId) || IsInPvpProhibitedArea(areaId) || IsInPvpProhibitedZone(areaId);
}

bool PlayerbotAIConfig::IsInPvpProhibitedZone(uint32 id)
{
    return find(pvpProhibitedZoneIds.begin(), pvpProhibitedZoneIds.end(), id) != pvpProhibitedZoneIds.end();
}

bool PlayerbotAIConfig::IsInPvpProhibitedArea(uint32 id)
{
    return find(pvpProhibitedAreaIds.begin(), pvpProhibitedAreaIds.end(), id) != pvpProhibitedAreaIds.end();
}

bool PlayerbotAIConfig::IsRestrictedHealerDPSMap(uint32 mapId) const
{
    return restrictHealerDPS &&
            std::find(restrictedHealerDPSMaps.begin(), restrictedHealerDPSMaps.end(), mapId) != restrictedHealerDPSMaps.end();
}

std::string const PlayerbotAIConfig::GetTimestampStr()
{
    time_t t = time(nullptr);
    tm* aTm = localtime(&t);
    //       YYYY   year
    //       MM     month (2 digits 01-12)
    //       DD     day (2 digits 01-31)
    //       HH     hour (2 digits 00-23)
    //       MM     minutes (2 digits 00-59)
    //       SS     seconds (2 digits 00-59)
    char buf[20];
    snprintf(buf, 20, "%04d-%02d-%02d %02d-%02d-%02d", aTm->tm_year + 1900, aTm->tm_mon + 1, aTm->tm_mday, aTm->tm_hour,
             aTm->tm_min, aTm->tm_sec);
    return std::string(buf);
}

bool PlayerbotAIConfig::openLog(std::string const fileName, char const* mode)
{
    if (!hasLog(fileName))
        return false;

    auto logFileIt = logFiles.find(fileName);
    if (logFileIt == logFiles.end())
    {
        logFiles.insert(std::make_pair(fileName, std::make_pair(nullptr, false)));
        logFileIt = logFiles.find(fileName);
    }

    FILE* file = logFileIt->second.first;
    bool fileOpen = logFileIt->second.second;

    if (fileOpen)  // close log file
        fclose(file);

    std::string m_logsDir = sConfigMgr->GetOption<std::string>("LogsDir", "", false);
    if (!m_logsDir.empty())
    {
        if ((m_logsDir.at(m_logsDir.length() - 1) != '/') && (m_logsDir.at(m_logsDir.length() - 1) != '\\'))
            m_logsDir.append("/");
    }

    file = fopen((m_logsDir + fileName).c_str(), mode);
    fileOpen = true;

    logFileIt->second.first = file;
    logFileIt->second.second = fileOpen;

    return true;
}

void PlayerbotAIConfig::log(std::string const fileName, char const* str, ...)
{
    if (!str)
        return;

    std::lock_guard<std::mutex> guard(m_logMtx);

    if (!isLogOpen(fileName) && !openLog(fileName, "a"))
        return;

    FILE* file = logFiles.find(fileName)->second.first;

    va_list ap;
    va_start(ap, str);
    vfprintf(file, str, ap);
    fprintf(file, "\n");
    va_end(ap);
    fflush(file);

    fflush(stdout);
}

void PlayerbotAIConfig::loadWorldBuff()
{
    std::string matrix = sConfigMgr->GetOption<std::string>("AiPlayerbot.WorldBuffMatrix", "", true);
    if (matrix.empty())
        return;

    std::istringstream entryStream(matrix);
    std::string entry;

    while (std::getline(entryStream, entry, ';'))
    {

        entry.erase(0, entry.find_first_not_of(" \t\r\n"));
        entry.erase(entry.find_last_not_of(" \t\r\n") + 1);

        size_t firstColon = entry.find(':');
        size_t secondColon = entry.find(':', firstColon + 1);

        if (firstColon == std::string::npos || secondColon == std::string::npos)
        {
            LOG_ERROR("playerbots", "Malformed entry: [{}]", entry);
            continue;
        }

        std::string metaPart = entry.substr(firstColon + 1, secondColon - firstColon - 1);
        std::string spellPart = entry.substr(secondColon + 1);

        std::vector<uint32> ids;
        std::istringstream metaStream(metaPart);
        std::string token;
        while (std::getline(metaStream, token, ','))
        {
            try {
                ids.push_back(static_cast<uint32>(std::stoi(token)));
            } catch (...) {
                LOG_ERROR("playerbots", "Invalid meta token in [{}]", entry);
                break;
            }
        }

        if (ids.size() != 5)
        {
            LOG_ERROR("playerbots", "Entry [{}] has incomplete meta block", entry);
            continue;
        }

        std::istringstream spellStream(spellPart);
        while (std::getline(spellStream, token, ','))
        {
            try {
                uint32 spellId = static_cast<uint32>(std::stoi(token));
                worldBuff wb = { spellId, ids[0], ids[1], ids[2], ids[3], ids[4] };
                worldBuffs.push_back(wb);
            } catch (...) {
                LOG_ERROR("playerbots", "Invalid spell ID in [{}]", entry);
            }
        }
    }
}

static std::vector<std::string> split(const std::string& str, const std::string& pattern)
{
    std::vector<std::string> res;
    if (str == "")
        return res;
    // Also add separators to string connections to facilitate intercepting the last paragraph.
    std::string strs = str + pattern;
    size_t pos = strs.find(pattern);

    while (pos != strs.npos)
    {
        std::string temp = strs.substr(0, pos);
        res.push_back(temp);
        // Remove the split string and split the remaining string
        strs = strs.substr(pos + 1, strs.size());
        pos = strs.find(pattern);
    }

    return res;
}

std::vector<std::vector<uint32>> PlayerbotAIConfig::ParseTempTalentsOrder(uint32 cls, std::string tab_link)
{
    // check bad link
    uint32 classMask = 1 << (cls - 1);
    std::vector<std::vector<uint32>> res;
    std::vector<std::string> tab_links = split(tab_link, "-");
    std::map<uint32, std::vector<TalentEntry const*>> spells;
    std::vector<std::vector<std::vector<uint32>>> orders(3);
    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const* talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo)
            continue;

        TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);
        if (!talentTabInfo)
            continue;

        if ((classMask & talentTabInfo->ClassMask) == 0)
            continue;

        spells[talentTabInfo->tabpage].push_back(talentInfo);
    }
    for (int tab = 0; tab < 3; tab++)
    {
        if (tab_links.size() <= tab)
        {
            break;
        }
        std::sort(spells[tab].begin(), spells[tab].end(),
                  [&](TalentEntry const* lhs, TalentEntry const* rhs)
                  { return lhs->Row != rhs->Row ? lhs->Row < rhs->Row : lhs->Col < rhs->Col; });
        for (int i = 0; i < tab_links[tab].size(); i++)
        {
            if (i >= spells[tab].size())
            {
                break;
            }
            int lvl = tab_links[tab][i] - '0';
            if (lvl == 0)
                continue;
            orders[tab].push_back({(uint32)tab, spells[tab][i]->Row, spells[tab][i]->Col, (uint32)lvl});
        }
    }
    // sort by talent tab size
    std::sort(orders.begin(), orders.end(), [&](auto& lhs, auto& rhs) { return lhs.size() > rhs.size(); });
    for (auto& order : orders)
    {
        res.insert(res.end(), order.begin(), order.end());
    }
    return res;
}

std::vector<std::vector<uint32>> PlayerbotAIConfig::ParseTempPetTalentsOrder(uint32 spec, std::string tab_link)
{
    // check bad link
    // uint32 classMask = 1 << (cls - 1);
    std::vector<TalentEntry const*> spells;
    std::vector<std::vector<uint32>> orders;
    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const* talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo)
            continue;

        TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);
        if (!talentTabInfo)
            continue;

        if (!((1 << spec) & talentTabInfo->petTalentMask))
            continue;
        // skip some duplicate spells like dash/dive
        if (talentInfo->TalentID == 2201 || talentInfo->TalentID == 2208 || talentInfo->TalentID == 2219 ||
            talentInfo->TalentID == 2203)
            continue;

        spells.push_back(talentInfo);
    }
    std::sort(spells.begin(), spells.end(),
              [&](TalentEntry const* lhs, TalentEntry const* rhs)
              { return lhs->Row != rhs->Row ? lhs->Row < rhs->Row : lhs->Col < rhs->Col; });
    for (int i = 0; i < tab_link.size(); i++)
    {
        if (i >= spells.size())
        {
            break;
        }
        int lvl = tab_link[i] - '0';
        if (lvl == 0)
            continue;
        orders.push_back({spells[i]->Row, spells[i]->Col, (uint32)lvl});
    }
    // sort by talent tab size
    std::sort(orders.begin(), orders.end(), [&](auto& lhs, auto& rhs) { return lhs.size() > rhs.size(); });

    return orders;
}
