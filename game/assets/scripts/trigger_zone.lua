-- Logs overlap events so the trigger volume's behavior stays observable
-- now that it's routed through Lua instead of a hardcoded C++ log line.

local TriggerZone = {}

function TriggerZone.on_trigger_enter(entity, other)
    molly.log("Trigger enter: " .. entity.tag .. " <-> " .. other.tag)
end

function TriggerZone.on_trigger_exit(entity, other)
    molly.log("Trigger exit: " .. entity.tag .. " <-> " .. other.tag)
end

return TriggerZone
