# Loot lifecycle hardening

Based independently on master (2f7d9f77). Module-only; no core handler changes.

## Changes

- Keep a submitted corpse/open-lock/gathering target until a valid loot response.
- Suppress competing loot selection, approach and opening while an AI request is pending or a loot window is open.
- Allow retry after 15 seconds without a response, but never expire the pending request during an active cast. This is a recovery timeout, not a required packet delay.
- Stop movement and remove mounted auras on an earlier AI update before opening.
- Reject error, truncated and stale loot responses. Never restore the authoritative session loot GUID from a deferred packet.
- Retain queued core money/item/release handlers and existing item, permission, skill, quest, range and bag filters.
- Release only the active loot window, not every nearby GUID.
- Inspect later spell effects when an earlier effect is not an opening/skinning effect.

This is not a transaction identifier added to the WoW protocol: item requests still contain only a slot. A real client replacing a window after AI response processing but before queued requests execute remains a race to test. Reopening the same GUID cannot be distinguished solely by GUID. This branch does not change gameobject command dispatch; see the independent gameobject-hardening branch.

## In-game regression checklist (not yet executed)

Run each applicable case as a random playerbot, an altbot with a real master, and a selfbot; compare an ordinary player with AI disabled.

1. Approach a lootable corpse while moving/mounted. Observe stop/dismount, one opening request, money/items, then release.
2. Interrupt mining/herbalism/skinning during its cast. Verify the bot can retry after the recovery timeout and does not replace the target while casting.
3. Loot a corpse and then skin it. Repeat with herbalism/mining corpses and insufficient skills/tools.
4. Open a chest and a quest chest permitted by #2579. Repeat without the quest, with NOT_SELECTABLE, out of configured loot distance, and with a disallowed entry. Restrictions must remain effective.
5. Deny ownership or group loot permission; fill bags; filter an item via loot strategy. No permission bypass, phantom GUID restoration, or repeated opening while a window is active.
6. On a selfbot, manually close window A before its queued AI response is handled; open B. A's response must not overwrite B's GUID or enqueue storage/release for A.
7. Test empty, money-only, mixed, owner-only and view-only loot windows. The normal response header is 14 bytes, each item entry 22 bytes; an error response is 10 bytes.
8. Force an opening failure without a response. Verify another eligible target can be selected after timeout.
9. Confirm ordinary-player behavior is unchanged.

Build validation is not in-game verification.
