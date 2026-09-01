-- Player movement, entirely in Lua — no C++ input handling for this entity
-- (Milestone 5 proves the full engine <-> script boundary end-to-end).
--
-- Per-instance state (speed) lives on this module table, not on `entity`:
-- the Entity usertype only exposes engine-registered members (tag,
-- transform, ...), and each ScriptComponent gets a freshly executed copy
-- of this chunk, so a plain module-level table is already instance-local.

local Player = { speed = 3.5 }

function Player.on_start(entity)
    molly.log("Player script started for " .. entity.tag)
end

function Player.on_update(entity, dt)
    local dx = 0.0
    local dz = 0.0

    if molly.input.is_key_down(molly.Key.W) then dz = dz - 1.0 end
    if molly.input.is_key_down(molly.Key.S) then dz = dz + 1.0 end
    if molly.input.is_key_down(molly.Key.A) then dx = dx - 1.0 end
    if molly.input.is_key_down(molly.Key.D) then dx = dx + 1.0 end

    molly.move_character(entity, dx, dz, Player.speed)
end

return Player
