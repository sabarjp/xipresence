#define _CRT_SECURE_NO_WARNINGS

#include "discord_game_sdk.h"
#include "discord_wrapper.h"
#include <stdlib.h>
#include <windows.h>

// Global Core pointer
static struct IDiscordCore* core;

static void lua_log(lua_State* L, const char* message) {
    printf("%s", message);

    //lua_getglobal(L, "print");  // Get the `print` function from Lua
    //if (lua_isfunction(L, -1)) {
    //    lua_pushstring(L, message);  // Push the message onto the stack
    //    lua_call(L, 1, 0);           // Call the Lua print function
    //}
    //else {
    //    // If Lua's `print` is unavailable, fallback
    //    fprintf(stderr, "Lua print is not available: %s\n", message);
    //}
}

static void core_log_hook(void* hook_data, enum EDiscordLogLevel level, const char* message)
{
    lua_State* L = (lua_State*)hook_data; // Cast back to Lua state
    char log_message[512];
    snprintf(log_message, sizeof(log_message), "[Discord SDK] [%d] %s\n", (int)level, message);
    lua_log(L, log_message);
}


// Lua function to initialize Discord
int lua_discord_initialize(lua_State* L) {
    const char* app_id_str = luaL_checkstring(L, 1);  // aplication id from lua
    int64_t app_id = strtoll(app_id_str, NULL, 10);

    // Set up the create parameters
    struct DiscordCreateParams params;
    DiscordCreateParamsSetDefault(&params); // Set default values for the struct

    params.client_id = app_id;                 // Set the application ID (client ID)
    params.flags = DiscordCreateFlags_NoRequireDiscord; 

    lua_log(L, "Attempting to initialize Discord SDK...");

    // Call DiscordCreate with the correct version
    enum EDiscordResult result = DiscordCreate(DISCORD_VERSION, &params, &core);
    if (result != DiscordResult_Ok || !core) {
        lua_log(L, "Discord initialization failed.");
        lua_pushboolean(L, 0); // Initialization failed
        return 1;
    }

    //core->set_log_hook(core, DiscordLogLevel_Info, (void*)L, core_log_hook);

    lua_log(L, "Discord SDK initialized successfully.");
    lua_pushboolean(L, 1); // Initialization successful
    return 1;
}

static void activity_update_callback(void* data, enum EDiscordResult result) {
    lua_State* L = (lua_State*)data; // Cast back to Lua state
    if (result == DiscordResult_Ok) {
        lua_log(L, "Activity updated successfully.");
    }
    else {
        char error_message[256];
        snprintf(error_message, sizeof(error_message), "Failed to update activity: %d", result);
        lua_log(L, error_message);
    }
}

int lua_discord_update_activity(lua_State* L) {
    if (core == NULL) {
        lua_log(L, "Discord SDK is not initialized.");
        lua_pushboolean(L, 0);
        return 1;
    }

    struct IDiscordActivityManager* activity_manager = core->get_activity_manager(core);
    if (activity_manager == NULL) {
        lua_log(L, "Failed to get Activity Manager.");
        lua_pushboolean(L, 0);
        return 1;
    }

    const char* state = luaL_checkstring(L, 1);
    const char* details = luaL_checkstring(L, 2);
    int64_t startTime = strtoll(luaL_checkstring(L, 3), NULL, 10);
    int32_t partySize = luaL_checkinteger(L, 4);
    int32_t partyMax = luaL_checkinteger(L, 5);
    const char* largeImgKey = luaL_checkstring(L, 6);
    const char* largeImgText = luaL_checkstring(L, 7);
    const char* smallImgKey = luaL_checkstring(L, 8);
    const char* smallImgText = luaL_checkstring(L, 9);

    char test_message[512];
    snprintf(test_message, sizeof(test_message), "state='%s', details='%s, start_Time='%lld', pSize='%d', pmax='%d'", state, details, startTime, partySize, partyMax);
    lua_log(L, test_message);

    struct DiscordActivity activity;
    memset(&activity, 0, sizeof(activity));
    activity.type = DiscordActivityType_Playing;
    strncpy(activity.state, state, sizeof(activity.state) - 1);
    strncpy(activity.details, details, sizeof(activity.details) - 1);
    activity.timestamps.start = startTime;
    strncpy(activity.party.id, "test", sizeof(activity.party.id) - 1);
    activity.party.size.current_size = partySize;
    activity.party.size.max_size = partyMax;
    strncpy(activity.secrets.join, "lm54FF637Hna0w378gv5blY8O3", sizeof(activity.secrets.join) - 1);
    strncpy(activity.assets.large_image, largeImgKey, sizeof(activity.assets.large_image) - 1);
    strncpy(activity.assets.large_text, largeImgText, sizeof(activity.assets.large_text) - 1);
    strncpy(activity.assets.small_image, smallImgKey, sizeof(activity.assets.small_image) - 1);
    strncpy(activity.assets.small_text, smallImgText, sizeof(activity.assets.small_text) - 1);

    
    
    char log_message[512];
    snprintf(log_message, sizeof(log_message), "Setting activity: state='%s', details='%s, id='%lld', name='%s'", state, details, activity.application_id, activity.name);
    lua_log(L, log_message);

    activity_manager->update_activity(activity_manager, &activity, (void*)L, activity_update_callback);

    lua_pushboolean(L, 1);
    return 1;
}

int lua_discord_run_callbacks(lua_State* L) {
    if (core == NULL) {
        lua_log(L, "Discord SDK is not initialized.");
        lua_pushboolean(L, 0);  // Return false to Lua
        return 1;
    }

    enum EDiscordResult result = core->run_callbacks(core);
    if (result != DiscordResult_Ok) {
        char error_message[256];
        snprintf(error_message, sizeof(error_message), "run_callbacks failed: %d", result);
        lua_log(L, error_message);
        lua_pushboolean(L, 0);  // Return false to Lua
        return 1;
    }

    lua_pushboolean(L, 1);  // Return true to Lua
    return 1;
}

// Shutdown the Discord SDK
int lua_discord_shutdown(lua_State* L) {
    core->destroy(core);
    core = NULL;
    lua_log(L, "Discord SDK shut down.");
    lua_pushboolean(L, 1);
    return 1;
}

// Register the module with Lua
__declspec(dllexport) int luaopen_discord_wrapper(lua_State* L) {
    // Define the module functions
    struct luaL_reg discord_funcs[] = {
        {"initialize", lua_discord_initialize},
        {"update_activity", lua_discord_update_activity},
        {"run_callbacks", lua_discord_run_callbacks},
        {"shutdown", lua_discord_shutdown},
        {NULL, NULL}  // Sentinel to mark the end of the array
    };

    // Register the functions under the module name
    luaL_register(L, "discord_wrapper", discord_funcs);

    return 1;  // Return the table to Lua
}
 