-- Player movement, combat, and health — entirely in Lua. Melee/ranged are
-- both implemented as instant spatial-query "hits" within a range rather
-- than a literal flying projectile (see enemy_shooter.lua for why); they
-- differ in range/damage/cooldown, which is enough to feel distinct.

local Player = {
    speed = 3.5,

    meleeRange = 2.0,
    meleeDamage = 34,
    meleeCooldown = 0.5,
    meleeCooldownTimer = 0.0,

    rangedRange = 8.0,
    rangedDamage = 20,
    rangedCooldown = 0.8,
    rangedCooldownTimer = 0.0,

    contactRange = 1.1,
    contactDamage = 10,
    invulnDuration = 0.6,
    invulnTimer = 0.0,
}

local function tryHit(entity, range, damage)
    local target = molly.find_closest_entity(entity.transform.position, "Enemy", range)
    if not target then return false end

    target.health.health = target.health.health - damage
    molly.play_sound(entity, "assets/audio/sfx_attack.wav")

    if target.health.health <= 0 then
        if GameState then GameState.enemiesRemaining = GameState.enemiesRemaining - 1 end
        molly.destroy_entity(target)
    end
    return true
end

function Player.on_start(entity)
    molly.log("Player script started for " .. entity.tag)
end

function Player.on_update(entity, dt)
    if GameState and (GameState.gameOver or GameState.victory) then
        return
    end

    -- Movement
    local dx, dz = 0.0, 0.0
    if molly.input.is_key_down(molly.Key.W) then dz = dz - 1.0 end
    if molly.input.is_key_down(molly.Key.S) then dz = dz + 1.0 end
    if molly.input.is_key_down(molly.Key.A) then dx = dx - 1.0 end
    if molly.input.is_key_down(molly.Key.D) then dx = dx + 1.0 end
    molly.move_character(entity, dx, dz, Player.speed)

    -- Cooldowns
    Player.meleeCooldownTimer = math.max(0.0, Player.meleeCooldownTimer - dt)
    Player.rangedCooldownTimer = math.max(0.0, Player.rangedCooldownTimer - dt)
    Player.invulnTimer = math.max(0.0, Player.invulnTimer - dt)

    -- Melee (left click)
    if molly.input.is_mouse_down(molly.MouseButton.Left) and Player.meleeCooldownTimer <= 0 then
        Player.meleeCooldownTimer = Player.meleeCooldown
        tryHit(entity, Player.meleeRange, Player.meleeDamage)
    end

    -- Ranged (right click)
    if molly.input.is_mouse_down(molly.MouseButton.Right) and Player.rangedCooldownTimer <= 0 then
        Player.rangedCooldownTimer = Player.rangedCooldown
        tryHit(entity, Player.rangedRange, Player.rangedDamage)
    end

    -- Contact damage from anything currently touching range, with i-frames
    if Player.invulnTimer <= 0 then
        local nearby = molly.find_closest_entity(entity.transform.position, "Enemy", Player.contactRange)
        if nearby then
            entity.health.health = entity.health.health - Player.contactDamage
            Player.invulnTimer = Player.invulnDuration
        end
    end

    -- Publish health/state for the HUD (GameLayer reads this each frame)
    if GameState then
        GameState.playerHealth = entity.health.health
        GameState.playerMaxHealth = entity.health.maxHealth
        if entity.health.health <= 0 and not GameState.gameOver then
            GameState.gameOver = true
            molly.log("Player died. Game Over.")
        end
    end
end

return Player
