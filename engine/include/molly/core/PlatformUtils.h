#pragma once

namespace molly {

// Changes the process's current working directory to the directory
// containing the running executable. Every asset/log path in the engine
// is a relative path resolved against the cwd (e.g. "assets/shaders/..."),
// which only works if the app happens to be launched from the right
// directory — true from a terminal `cd`'d into editor/ or game/, false for
// a double-click from Explorer or most debugger default working
// directories. Called once at the very start of main() to make launch
// location irrelevant, matching how real engines ship (assets copied next
// to the exe by the build, exe finds them regardless of launch method).
// Windows-only, matching the engine's Windows-only scope (PLAN.md §1.2).
void setWorkingDirectoryToExecutable();

}  // namespace molly
