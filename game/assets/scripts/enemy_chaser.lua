-- Chaser: closes distance on the player and relies on the player's own
-- contact-damage check (player.lua) to actually hurt them — this script
-- only needs to move and watch its own health.

local Chaser = { speed = 2.2 }

function Chaser.on_update(entity, dt)
    if GameState and (GameState.gameOver or GameState.victory) then
        return
    end

    if entity.health.health <= 0 then
        if GameState then GameState.enemiesRemaining = GameState.enemiesRemaining - 1 end
        molly.destroy_entity(entity)
        return
    end

    local player = molly.find_entity("Player")
    if not player then return end

    local pos = entity.transform.position
    local ppos = player.transform.position
    local dx = ppos.x - pos.x
    local dz = ppos.z - pos.z
    local dist = math.sqrt(dx * dx + dz * dz)

    if dist > 0.01 then
        molly.move_character(entity, dx / dist, dz / dist, Chaser.speed)
    end
end

return Chaser
