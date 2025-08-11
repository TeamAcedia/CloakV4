Cloak V4
==============
<img src="doc/showcase1.png" alt="showcase" width="80%" />

A minetest cheat client based off of Luanti 5.12.0.

It was recently updated to 5.12.0 and many new features were added, if you want to use the old 5.9.1 version, go to the legacy branch.

This client's name is based off of TeamAcedia's previous clients, CloakV3, CloakV2, and Cloak

### Important info:
This client includes almost all CSM api additions from dragonfire, although it hasn't been documented (yet). It also includes many built-in client side mods and a new minimalistic design for the cheat menu. If any bug or error is found, please open an issue about it. We'd like to hear your opinion.

### Discord servers

- [TeamAcedia](https://discord.com/invite/SKGuNxF4)
- [Minetest cheat clients (former dragonfire server)](https://discord.gg/kab99uu)


### Development team:
  - [ProunceDev](https://github.com/ProunceDev) [Core dev]
  - [Maintainer_ / FoxLoveFire](https://github.com/FoxLoveFire) [Core dev]
  - [plus22 / Plus-22](https://github.com/plus-22) [Core dev]
  - [Astra0081X](https://github.com/astra0081X) [Core dev]
  - Burrowing_Owl [Contributor]

# Features
### Menus
  - Quick Menu: Dragonfire cheat menu. Use arrow keys to navigate through it and enter key to toggle cheats. In order to activate it, toggle QuickMenu cheat.
  - Click Menu: Use F8 key to open it. It displays different windows for each category. Each window can be moved. It also lets you configure certain cheats. This menu was first implemented in [MineBoost Client](https://www.github.com/FoxLoveFire/MineBoost) by [Maintainer_ / FoxLoveFire](https://github.com/FoxLoveFire/), all credit goes to him.


### Cheats
<details>
  <summary>Combat</summary>

  - AntiKnockback: No knockback.
  - AttachmentFloat: float above parent.
  - AutoTotem: automatically puts a totem in your offhand.
  - AutoAim: aims at the nearest player.
  - CombatLog: Log off when certain HP is reached.
  - Criticals: Always do critical hits in mcl2/mcla servers.
  - CrystalSpam: automatically puts end crystals at another player's position and detonates it.
  - Killaura: auto-hit designated targets. Could be players, mobs or both.
</details>

<details>
  <summary>Interact</summary>

  - AutoDig: automatically dig pointed block
  - AutoHit: automatically hit pointed entity
  - AutoPlace: automatically place wielded item at pointed block
  - AutoTool: automatically select best tool for digging/hitting
  - Blink: Delay packet sending until this cheat is desabled.
  - FastDig: dig at 10x speed
  - FastHit: hit at 6x speed. Can be modified using the `.fasthit <multiplier>` chat command
  - FastPlace: instant block placing
  - InstantBreak: instant block breaking
</details>

<details>
  <summary>Inventory</summary>

  - AutoEject
  - AutoPlanks
  - AutoRefill: Automatically refill the item you're holding.
  - EnderChest: Lets you preview your ender chest.
  - Hand: Open hand formspec.
</details>


<details>
  <summary>Misc</summary>

  - AntiAFK: Always move to avoid getting kicked for being AFK.
  - AntiAdmin: Checks if there's any person with ban or kick privs.
  - AppleAura: dig any nearby apples.
  - AutoHeal: Eats food to regain health in ctf and similar servers.
  - AutoTeam: Set allied players to your team in CTF.
  - FlagAura: Pick up nearby flags in CTF.
  - Hints: Enable cheat descriptions.
  - Nametags: Set a player's nametag to a certain color based on their friend/enemy status.
  - Panic: Disable detectable cheats.
  - Spammer: Spams a preset message.
</details>


<details>
  <summary>Movement</summary>

  - AirJump: jump while in the air
  - AntiSlip: Walk on slippery blocks without slipping
  - AutoForward: Automatically move forward.
  - AutoJump: Jump automatically.
  - AutoSneak: Always sneak. This hides your nametag on some servers.
  - FastMove: Toggles fast.
  - Flight: Toggles fly.
  - Freecam: Spectator mode.
  - Jesus: Walk on liquids.
  - JetPack: AirJump but you fall after jumping.
  - Noclip: Toggles noclip.
  - NoSlow: Sneaking doesn't slow you down.
  - Overrides: Movement overrides.
  - PitchMove: While flying, move where you're pointing.
  - Spider: Climb walls.
  - Step: Climb the block you're facing.
  - Velocity: Various velocity overrides.
</details>

<details>
  <summary>Player</summary>

  - AutoRespawn: Respawn automatically after dying.
  - NoFallDamage: You suffer from no fall damage.
  - NoForceRotate: Prevent server from changing the player's view direction.
  - NoViewBob: Disable view bobbing.
  - PrivBypass: Lets the player toggle fly, fast, noclip and wireframe rendering without needing any privs.
  - QuickMenu: Enables the dragonfire cheat menu.
  - Reach: extend interact distance.
</details>

<details>
  <summary>Render</summary>

  - BrightNight: Always daytime.
  - CheatHUD: show currently toggled cheats.
  - Coords: Turns on a coords GUI.
  - EntityESP: show bounding boxes around entities.
  - EntityTracers: show lines to entities.
  - FOV: Lets you change your FOV.
  - Fullbright: No darkness.
  - HUDBypass: Allow player to toggle HUD elements disabled by the game.
  - HealthESP: Show player and entities' HP.
  - NoDrownCam: Disables drowning camera effect.
  - NoHurtCam: Disables hurt camera effect.
  - NodeESP: can be configured using the `.search add <(optional) node_name>` chat command.
  - NodeTracers: can be configured using the `.search add <(optional) node_name>` chat command.
  - NoRender: skip rendering particles to reduce client lag.
  - PlayerESP: show bounding boxes around players. Green = friendly, Red = enemy. Friends can be configured using the `.friend add <(optional) player_name>` chat command.
  - PlayerTracers: show lines to players. Green = friendly, Red = enemy. Friends can be configured using the `.friend add <(optional) player_name>` chat command.
  - TunnelESP: shows tunnels.
  - TunnelTracers: show lines to tunnels.
  - Xray: see blocks thru walls. Can be configured using the `.xray add <(optional) node_name>` chat command.
</details>

<details>
  <summary>World</summary>

  - AutoTNT: Puts TNT on the ground.
  - BlockLava: Replace lava with the block you're holding.
  - BlockWater: Replace water with the block you're holding.
  - Nuke
  - Replace: When you break a block, it gets replaced with the block you're holding.
  - Scaffold: Puts blocks below you.
  - ScaffoldPlus: Puts even more blocks below you.
  - Silence: Disables sound.
</details>

NOTE: The default menu bars, when expanded, will overlap. To fix this, move each option (check [#5](https://github.com/TeamAcedia/CloakV4/issues/5#issuecomment-2655191958) for visuals) or you can enable the "QuickMenu" feature (emulating dragonfireclient's menu. Keep in mind you can't configure cheats from there)

### Chat commands
- `.fasthit <multiplier>` set fasthit multiplier. FastHit needs to be turned on for this to be used.
- `.say <text>` send raw text
- `.tp <X> <Y> <Z>` teleport
- `.wielded` show itemstring of wielded item
- `.players` list online players
- `.kill` kill your own player
- `.sneak` toggle autosneak
- `.speed <speed>` set speed
- `.place <X> <Y> <Z>` place wielded item
- `.dig <X> <Y> <Z>` dig node
- `.break` toggle instant break
- `.setyaw <yaw>`
- `.setpitch <pitch>`
- `.respawn` respawn while in ghost mode
- `.xray`
    - `add <(optional) node_name>`: add a node to xray configuration (to see thru it). Can either add by node_name or by pointing at a node.
    - `del <(optional) node_name>`: remove a node from xray configuration
    - `list`: list xray nodes
    - `clear`: clear xray nodes
- `.search`
    - `add <(optional) node_name>`: add a node to nodeESP search. Can either add by node_name or by pointing at a node.
    - `del <(optional) node_name>`: remove a node from nodeESP search
    - `list`: list nodeESP nodes
    - `clear`: clear nodeESP nodes
- `.friend`
    - `add <(optional) player_name>`: add a friend. Can either add by player_name or by pointing at a player. Friends are server-specific.
    - `del <(optional) player_name>`: remove a friend
    - `list`: list all friends for the current server
    - `clear`: remove all friends for the current server
- `.ctf`
    - `add`: add current server to ctf servers list
    - `del`: delete current server from ctf servers list
    - `list`: view ctf servers list
    - `clear`: clear ctf servers list
 


# Compiling
- [Compiling on GNU/Linux](doc/compiling/linux.md)
- [Compiling on Windows](doc/compiling/windows.md)
- [Compiling on MacOS](doc/compiling/macos.md)
- [Compiling on Android](doc/android.md)
