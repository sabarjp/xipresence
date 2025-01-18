_addon.name = 'xipresence'
_addon.author = 'sabarjp'
_addon.version = '0.1'

res = require('resources') -- global from windower
zone_mapping = require('zone_mapping')

is_first_loaded = false
is_connected = false
is_zoning = false

-- how long the current activity has been running, since epoch
current_timer = 0

-- used to track if the party size has changed
current_party_size = 1

-- used to track if combat status has changed
is_in_combat = false


local addon_path = windower.addon_path:gsub('\\', '/')
package.cpath = package.cpath .. ';' .. addon_path .. 'libs/?.dll'

-- debugging
-- package.cpath = package.cpath .. ';E:\\windower\\addons\\xipresence\\libs\\?.dll'

package.loadlib(addon_path .. "libs/discord_game_sdk.dll", "*")
discord_wrapper = require("discord_wrapper")
local app_id = '1328470480323215499'

-- we don't want to spam the discord API, so debounce some functions to only run
-- at most one time in X seconds
local function debounce(fn, delay)
  local last_called = 0 -- Track the last call time

  return function(...)
    local now = os.time() -- Get the current time in seconds
    if now - last_called >= delay then
      last_called = now
      return fn(...) -- Call the original function
    end
  end
end

function connect()
  local initialize_success = discord_wrapper.initialize(app_id)
  if not initialize_success then
    --print("Failed to initialize Discord SDK.")
  else
    is_connected = true

    -- wait a moment before the first update...
    coroutine.schedule(function()
      current_timer = now()
      update_activity()
    end, 1.0)
  end
end

local db_connect = debounce(connect, 15)

function now()
  return os.time() * 1000
end

function update_activity()
  discord_wrapper.update_activity(get_status(), get_location(), current_timer, get_party_size(), get_party_max(),
    get_activity_key(), get_activity(), get_job_key(), get_job())
end

local db_update_activity = debounce(update_activity, 1)

function get_activity_key()
  local info = windower.ffxi.get_info()

  if info and info.mog_house then
    return 'key1'
  end

  if info and info.zone then
    local key = zone_mapping[info.zone]

    if key then
      return key:lower()
    end
  end

  return "key1"
end

function get_job_key()
  local player = windower.ffxi.get_player()

  if player then
    return player.main_job:lower()
  end

  return "key1"
end

function get_party_size()
  local party = windower.ffxi.get_party()
  if party and party.party1_count then
    return party.party1_count + party.party2_count + party.party3_count
  end

  return 1
end

function get_party_max()
  local party = windower.ffxi.get_party()
  if party then
    if party.alliance_leader then
      return 18
    else
      if party.party1_leader then
        return 6
      else
        return 6
      end
    end
  end

  return 1
end

function get_job()
  local player = windower.ffxi.get_player()

  if player then
    if not player.sub_job then
      return player.main_job .. player.main_job_level
    else
      return player.main_job .. player.main_job_level .. '/' .. player.sub_job .. player.sub_job_level
    end
  end

  return ''
end

function get_activity()
  local info = windower.ffxi.get_info()
  local player = windower.ffxi.get_player()



  -- Not logged in yet
  if not info or not info.logged_in then
    return "Logging in"
  end

  -- In combat
  if player and player.in_combat then
    return "Fighting in " .. get_location()
  end

  if info.mog_house then
    return 'Resting in a mog house'
  end

  -- Idle status
  return "Adventuring in " .. get_location()
end

function get_status()
  local info = windower.ffxi.get_info()
  local party = windower.ffxi.get_party()

  -- Not logged in yet
  if info and not info.logged_in then
    return "Logging in"
  end

  -- Idle status
  if party and party.alliance_leader then
    return "In alliance"
  else
    if party and party.party1_leader then
      return "In party"
    else
      return "Solo"
    end
  end
end

function get_location()
  local info = windower.ffxi.get_info()

  if info and info.mog_house then
    return 'Mog House'
  elseif info and info.zone then
    return res.zones[info.zone].en
  end

  return 'Unknown'
end

-- initial load
windower.register_event('load', function()
  if is_first_loaded == false then
    is_first_loaded = true

    connect()
  end
end)

windower.register_event('login', function()
  coroutine.sleep(0.5)
  if is_first_loaded == false then
    is_first_loaded = true

    connect()
  end
end)

windower.register_event('unload', function()
  if is_first_loaded and is_connected then
    discord_wrapper.shutdown()
  end
end)

windower.register_event('logout', function()
  if is_first_loaded and is_connected then
    discord_wrapper.shutdown()
  end
end)

windower.register_event('prerender', function()
  if is_connected then
    local callback_success = discord_wrapper.run_callbacks()
    if not callback_success then
      is_connected = false
    end
  else
    if is_first_loaded then
      -- retry
      db_connect()
    end
  end
end)

-------------------------------------------------------
------- TRIGGERS TO UPDATE STATUS
-------------------------------------------------------

-- World is loaded or zoning
windower.register_event('incoming chunk', function(id, original, modified, injected, blocked)
  if id == 0x00B then                   -- unload old zone
    is_zoning = true
  elseif id == 0x01D and is_zoning then -- complete load
    is_zoning = false
    current_timer = now()
    db_update_activity()
  end
end)

windower.register_event('job change', function()
  db_update_activity()
end)

windower.register_event('level up', function()
  db_update_activity()
end)

windower.register_event('level down', function()
  db_update_activity()
end)

windower.register_event('prerender', function()
  local party_size = get_party_size()
  local player = windower.ffxi.get_player()

  -- only update the activity if the party size has actually changed
  if party_size ~= current_party_size then
    current_party_size = party_size
    db_update_activity()
  end

  -- ...or if combat status has changed
  if player and player.in_combat ~= is_in_combat then
    is_in_combat = player.in_combat
    db_update_activity()
  end
end)
