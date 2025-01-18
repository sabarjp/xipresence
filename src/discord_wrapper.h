#ifndef DISCORD_WRAPPER_H
#define DISCORD_WRAPPER_H

#include <lua.h>
#include <lauxlib.h>

// Functions exported by the DLL
int lua_discord_initialize(lua_State* L);
int lua_discord_update_activity(lua_State* L);
int lua_discord_run_callbacks(lua_State* L);
int lua_discord_shutdown(lua_State* L);
__declspec(dllexport) int luaopen_discord_wrapper(lua_State* L);

#endif // DISCORD_WRAPPER_H
