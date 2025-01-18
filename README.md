# Discord Rich Presence for FFXI (Final Fantasy 11)
Simply adds Discord rich presence support to Final Fantasy 11 (retail). Windower 4 required.

## Whats in it?
- Current zone, with limited art
- Current job/sub combo, with limited art
- Play time in current zone
- In-game activity (limited)
- Size of party or solo play status

## Requirements
- Windows
- Windower 4
- Discord installed and running

## Installation
1. Download
2. Extract to `windower/addons/`
3. Make sure the `xipresence.lua` file is in an `xipresence` folder in that addons folder.
4. Edit `windower/scripts/init.txt` and add `lua load xipresence` to the end.

## I don't trust this random DLL!
The DLL can be easily generated from the included source. 

1. I used Visual Studio 2022 targeting x86 (release)
2. Ensure the two transitive DLLs are linked correctly: 1) the LuaCore implementation in Windower, and 2) the Discord game sdk 2.5.4 (x86)
3. Build solution
4. Copy the built DLL into the `addon/libs` folder
5. You still need the discord game sdk, so go download it (https://discord.com/developers/docs/developer-tools/game-sdk)
6. From the download, find the discord DLL for x86 and copy it into the `addon/libs` folder

In theory, it should be possible to just import the included source and hit "build".