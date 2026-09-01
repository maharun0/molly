-- Drives the core loop (PLAN.md §31.3): spawn a wave, wait for it to clear,
-- spawn the next one, and declare victory after the final wave. State other
-- scripts/C++ need (wave number, enemies remaining, game over/victory) lives
-- on the global GameState table this script creates in on_start — see
-- player.lua/enemy_*.lua for how they read/write it, and GameLayer.cpp for
-- how the HUD reads it.

local WaveManager = {}

local waveTable = {
    { chasers = 2, shooters = 0 },
    { chasers = 3, shooters = 1 },
    { chasers = 3, shooters = 2 },
    { chasers = 4, shooters = 2 },
    { chasers = 5, shooters = 3 },
}

local spawnPoints = {
    vec3.new(8.5, 0.0, 8.5),
    vec3.new(-8.5, 0.0, 8.5),
    vec3.new(8.5, 0.0, -8.5),
    vec3.new(-8.5, 0.0, -8.5),
    vec3.new(0.0, 0.0, 9.5),
    vec3.new(9.5, 0.0, 0.0),
    vec3.new(-9.5, 0.0, 0.0),
    vec3.new(0.0, 0.0, -9.5),
}

local currentWave = 0
local waveActive = false
local nextWaveTimer = 0.0
local WAVE_DELAY = 3.0

local function spawnWave()
    currentWave = currentWave + 1
    if currentWave > #waveTable then
        GameState.victory = true
        molly.log("All waves cleared — victory!")
        return
    end

    local cfg = waveTable[currentWave]
    local spawned = 0

    for i = 1, cfg.chasers do
        local sp = spawnPoints[((spawned) % #spawnPoints) + 1]
        molly.spawn_entity("assets/prefabs/enemy_chaser.prefab.json", sp)
        spawned = spawned + 1
    end
    for i = 1, cfg.shooters do
        local sp = spawnPoints[((spawned) % #spawnPoints) + 1]
        molly.spawn_entity("assets/prefabs/enemy_shooter.prefab.json", sp)
        spawned = spawned + 1
    end

    GameState.wave = currentWave
    GameState.enemiesRemaining = spawned
    waveActive = true
    molly.log("Wave " .. currentWave .. " started (" .. spawned .. " enemies)")
end

function WaveManager.on_start(entity)
    GameState = {
        wave = 0,
        enemiesRemaining = 0,
        playerHealth = 100,
        playerMaxHealth = 100,
        gameOver = false,
        victory = false,
    }
    spawnWave()
end

function WaveManager.on_update(entity, dt)
    if GameState.gameOver or GameState.victory then
        return
    end

    if waveActive and GameState.enemiesRemaining <= 0 then
        waveActive = false
        nextWaveTimer = WAVE_DELAY
    end

    if not waveActive then
        nextWaveTimer = nextWaveTimer - dt
        if nextWaveTimer <= 0 then
            spawnWave()
        end
    end
end

return WaveManager
