-- Logs overlap events and plays a chime so the trigger volume's behavior
-- stays observable now that it's routed through Lua instead of a
-- hardcoded C++ log line (Milestone 5), and demonstrates molly.play_sound
-- from a script (Milestone 9).

local TriggerZone = {}

function TriggerZone.on_trigger_enter(entity, other)
    molly.log("Trigger enter: " .. entity.tag .. " <-> " .. other.tag)
    molly.play_sound(entity, "assets/audio/sfx_trigger.wav")
end

function TriggerZone.on_trigger_exit(entity, other)
    molly.log("Trigger exit: " .. entity.tag .. " <-> " .. other.tag)
end

return TriggerZone
