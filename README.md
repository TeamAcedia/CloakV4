Cloak V4
==============
<img src="doc/showcase1.png" alt="showcase" width="80%" />

A minetest cheat client based off of [Chimeraclient](https://github.com/plus-22/chimeraclient/), which draws its roots from [OtterClient](https://github.com/otterminetest/OtterClient/).

This client's name is based off of TeamAcedia's previous clients, CloakV3, CloakV2, and Cloak

### Important info:
This client includes almost all CSM api additions from dragonfire, although it hasn't been documented (yet). It also includes many built-in client side mods and a new minimalistic design for the cheat menu.
It has not been tested extensively, so it may not be stable. If any bug or error is found, please open an issue about it.

### Core devs:
  - [ProunceDev](https://github.com/ProunceDev)
  - [FoxLoveFire](https://github.com/FoxLoveFire)
  - [Plus-22](https://github.com/plus-22)
  - [Pl608](https://github.com/PL608wastaken)

# Features
### Menus
  - Quick Menu: Dragonfire cheat menu. Use arrow keys to navigate through it and enter key to toggle cheats.
  - Click Menu: Use F8 key to open it. It displays different windows for each category. Each window can be moved. This menu was first implemented in [MineBoost Client](https://www.github.com/FoxLoveFire/MineBoost) by [Maintainer_ / FoxLoveFire](https://github.com/FoxLoveFire/), all credit goes to him.


Menus can be toggled with the "QuickMenu" cheat in Player category.

### Cheats
<details>
  <summary>Combat</summary>

  - AntiKnockback
  - AttachmentFloat: float above parent.
  - AutoAssist: don't completely kill your enemies. Turning this on automatically turns on PlayerAura.
  - AutoTotem: automatically puts a totem in your offhand. WARNING: Don't use on non-mcl servers.
  - AutoAim: aims at the nearest player.
  - CrystalSpam: automatically puts end crystals at another player's position and detonates it.
  - EntityAura: auto-hit entities.
  - PlayerAura: auto-hit enemy players.
</details>

<details>
  <summary>Interact</summary>

  - AutoDig: automatically dig pointed block
  - AutoHit: automatically hit pointed entity
  - AutoPlace: automatically place wielded item at pointed block
  - AutoTool: automatically select best tool for digging/hitting
  - FastDig: dig at 10x speed
  - FastHit: hit at 6x speed. Can be modified using the `.fasthit <multiplier>` chat command
  - FastPlace: instant block placing
  - InstantBreak: instant block breaking
</details>

<details>
  <summary>Inventory</summary>

  - AutoEject
  - AutoPlanks
  - AutoRefill
  - EnderChest
  - Hand
</details>

<details>
  <summary>Movement</summary>

  - AirJump: jump while in the air
  - AutoForward
  - AutoJump
  - AutoSneak: autosneak. This hides your nametag on some servers.
  - FastMove
  - Flight
  - Freecam
  - GravityOverride
  - Jesus
  - JetPack
  - JumpOverride
  - Noclip
  - NoSlow
  - PitchMove
  - SpeedOverride
  - Spider: climb walls
</details>

<details>
  <summary>Player</summary>

  - AutoRespawn
  - NoFallDamage
  - Reach: extend interact distance
  - PrivBypass
  - QuickMenu: switch from the click menu (new) to the quick menu (old)
</details>

<details>
  <summary>Render</summary>

  - CheatHUD: show currently toggled cheats
  - Coords: show coords in bottom left of screen
  - EntityESP: show bounding boxes around entities
  - EntityTracers: show lines to entities
  - Fullbright
  - HUDBypass
  - NoDrownCam
  - NoHurtCam
  - NodeESP: can be configured using the `.search add <(optional) node_name>` chat command
  - NodeTracers: can be configured using the `.search add <(optional) node_name>` chat command
  - NoRender: skip rendering particles to reduce client lag
  - PlayerESP: show bounding boxes around players. Green = friendly, Red = enemy. Friends can be configured using the `.friend add <(optional) player_name>` chat command.
  - PlayerTracers: show lines to players. Green = friendly, Red = enemy. Friends can be configured using the `.friend add <(optional) player_name>` chat command.
  - TunnelESP: shows tunnels.
  - TunnelTracers: show lines to tunnels.
  - Xray: see blocks thru walls. Can be configured using the `.xray add <(optional) node_name>` chat command.
</details>

<details>
  <summary>World</summary>
  - AutoTNT: switches to TNT and places it
  - BlockLava: puts blocks where lava is
  - BlockWater: puts blocks where water is
  - DigCustom: Digs custom nodes. Can be configured using the `.digcustom del | <item> | add <item> | list | clear` chat command.
  - Nuke
  - Replace
  - Scaffold
  - ScaffoldPlus
  - Silence
</details>

NOTE: The default menu bars, when expanded, will overlap. To fix this, you can enable the "QuickMenu" feature (emulating dragonfireclient's menu) or move each option. (check #5 for visuals)

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
