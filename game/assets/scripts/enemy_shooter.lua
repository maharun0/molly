-- Shooter: keeps its distance from the player and periodically pokes them
-- for damage directly (a simplified stand-in for a real flying projectile —
-- the engine doesn't yet have a sync path for scripting a moving physics
-- trigger, see PLAN.md Milestone 10 notes). This still gives it a real,
-- distinct threat from the chaser's contact damage.

local Shooter = {
    speed = 1.8,
    preferredRange = 5.0,
    shootRange = 6.0,
    shootDamage = 8,
    shootCooldown = 1.5,
    shootTimer = 0,
}

function Shooter.on_update(entity, dt)
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

    local mx, mz = 0.0, 0.0
    if dist > 0.01 then
        mx, mz = dx / dist, dz / dist
    end

    if dist < Shooter.preferredRange - 0.5 then
        molly.move_character(entity, -mx, -mz, Shooter.speed)
    elseif dist > Shooter.preferredRange + 0.5 then
        molly.move_character(entity, mx, mz, Shooter.speed)
    else
        molly.move_character(entity, 0.0, 0.0, 0.0)
    end

    Shooter.shootTimer = Shooter.shootTimer - dt
    if dist <= Shooter.shootRange and Shooter.shootTimer <= 0 then
        Shooter.shootTimer = Shooter.shootCooldown
        player.health.health = player.health.health - Shooter.shootDamage
        molly.play_sound(entity, "assets/audio/sfx_attack.wav")
    end
end

return Shooter
