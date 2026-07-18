/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_LOOTROLLACTION_H
#define PLAYERBOTS_LOOTROLLACTION_H

#include "QueryItemUsageAction.h"

class Group;
class PlayerbotAI;

struct ItemTemplate;

enum RollVote : uint8;

class LootRollAction : public QueryItemUsageAction
{
public:
    LootRollAction(PlayerbotAI* botAI, std::string const name = "loot roll") : QueryItemUsageAction(botAI, name) {}

    bool Execute(Event event) override;

protected:
    struct DisenchanterCacheEntry
    {
        uint32 groupId = 0;
        uint32 highestEnchantingSkill = 0;
        uint64 memberSignature = 0;
        time_t expiresAt = 0;
    };

    RollVote CalculateRollVote(ItemTemplate const* proto, ItemUsage usage = ITEM_USAGE_NONE);
    bool IsDisenchanterPresent(Group const* group, ItemTemplate const* proto) const;
    bool IsDisenchantEnabledForQuality(ItemTemplate const* proto) const;
    uint64 GetGroupMemberSignature(Group const* group) const;  // Helper for cache invalidation

    static constexpr time_t DISENCHANT_CACHE_TTL = 600;
    static constexpr size_t MAX_CACHED_GROUPS = 10;  // Limit to prevent memory growth
    mutable DisenchanterCacheEntry disenchanterCache[MAX_CACHED_GROUPS];  // Fixed-size array, no heap allocations
};

bool CanBotUseToken(ItemTemplate const* proto, Player* bot);
bool RollUniqueCheck(ItemTemplate const* proto, Player* bot);

class MasterLootRollAction : public LootRollAction
{
public:
    MasterLootRollAction(PlayerbotAI* botAI) : LootRollAction(botAI, "master loot roll") {}

    bool isUseful() override;
    bool Execute(Event event) override;
};

class RollAction : public Action
{
public:
    RollAction(PlayerbotAI* botAI) : Action(botAI, "roll") {}

    bool Execute(Event event) override;
};

#endif
