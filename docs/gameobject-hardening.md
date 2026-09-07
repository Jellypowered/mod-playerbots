# Gameobject interaction hardening

Based independently on master (2f7d9f77), not on looting-hardening. Module-only.

## Changes

`use [gameobject]` no longer calls `GameObject::Use()` directly for every object.

- Loot-bearing objects and chests are checked with `LootObject::IsLootPossible`, selected and added to the existing available-loot stack, then dispatched to `move to loot` or `open loot`.
- Configured loot distance (including its existing zero/unbounded convention), disallowed entries, NOT_SELECTABLE, #2579's INTERACT_COND/ActivateToQuest check, skills/tools/keys, object state and stay behavior are retained/enforced.
- Approaching requires the existing noncombat loot strategy so the engine can continue after movement. The command does not silently enable a disabled strategy. In-range attempts can execute immediately without enabling it.
- Non-loot objects go through the session use handler, with report-use handled separately for its script/achievement semantics. A locked non-loot object is rejected rather than unlocked via direct `Use`.
- Command messages distinguish rejected, queued and requested interactions. Void server handlers are not reported as confirmed success.
- Opening/gathering spells set their gameobject target and facing without first calling GAMEOBJ_USE. The validated spell effect owns the opening and its script hooks; this avoids running them before cast success or running them twice.

## Boundaries

This branch does not change loot-response storage, release sequencing or retry state. Those changes live independently on looting-hardening. Non-loot objects outside interaction range require moving closer; arbitrary locked non-loot interactions are not synthesized. Existing discovery and automatic-loot eligibility (including which non-loot quest objects are represented by LootObject) are not expanded. This is not a claim that every scripted gameobject is supported.

The normal player's core handlers are unchanged. The bot opening path intentionally invokes the spell implementation directly, as it did before, rather than manufacturing a second client cast packet for an already prepared server-side spell.

## In-game regression checklist (not yet executed)

Repeat applicable cases with playerbots, altbots and selfbots, plus a normal-player control:

1. Issue `use [chest]` in opening range: expect an opening request, a targeted opening spell, loot response, items/money and release.
2. Issue it outside interaction range but inside configured loot distance with loot enabled: expect queued approach, then the normal opening path. Test a blocked path and stay strategy.
3. Disable loot and issue the same out-of-range command: expect an explicit error, no strategy mutation. Test in-range use independently.
4. Gather mining/herb nodes with sufficient and insufficient skills/tools. Interrupt the cast; the object must not have received a premature Use hook.
5. Test Bundle of Wood with the required quest, without it and after completing its items. Verify #2579 is still present and ineligible objects remain blocked.
6. Test a disallowed entry, a NOT_SELECTABLE raid chest, a conditional dungeon chest, an unspawned object and an object beyond configured loot distance. None should be bypassed by an explicit command.
7. Test an unlocked door, questgiver or other supported non-loot interaction: verify session use/report behavior and truthful request messaging. Test locked non-loot objects and out-of-range objects for explicit refusal.
8. With script logging, compare opening-spell callbacks against a real player's interaction; verify removing the preliminary GAMEOBJ_USE avoids duplicate/premature use effects.
9. Verify item-use commands (`use [item]` and item-on-gameobject) are unaffected.

Build validation is not in-game verification.
