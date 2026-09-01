# Molly

A from-scratch C++20 game engine (OpenGL renderer, entt ECS, Jolt physics,
Lua/sol2 scripting, an in-house editor) built as a learning project, with a
small wave-survival arena demo game on top to prove the whole stack works
end to end.

Full architecture, design rationale, and the milestone-by-milestone plan
this was built from live in [`plan/PLAN.md`](plan/PLAN.md).

## Requirements

- Windows 11, Visual Studio 2022 (MSVC) with the "Desktop development with
  C++" workload
- CMake 3.28+
- Git

Everything else (GLFW, GLM, entt, ImGui, Jolt, Lua, sol2, Assimp,
miniaudio, spdlog, nlohmann/json, Catch2) is vendored inside the repo under
`vendor/` — no package manager, no other install step.

## Getting started

```powershell
git clone <repo-url> molly
cd molly
git submodule update --init --recursive

cmake --preset windows-debug
cmake --build --preset windows-debug --parallel
```

First build compiles the whole vendor tree (Assimp and Jolt especially) and
can take several minutes; subsequent builds are incremental.

Run the editor or the demo game — both need their working directory set to
their own folder so relative `assets/...` paths resolve:

```powershell
cd editor
..\build\windows-debug\bin\editor\Debug\molly-editor.exe

cd ..\game
..\build\windows-debug\bin\game\Debug\molly-demo.exe
```

In VS Code with the CMake Tools extension, select the `windows-debug`
preset and use the checked-in `.vscode/launch.json` configurations
("Launch molly-editor" / "Launch molly-demo") to build and debug directly —
they already set the correct working directory.

## The demo game

WASD to move, left click for a melee attack, right click for a ranged
attack. Survive escalating waves of two enemy types (a red melee "chaser"
and a purple "shooter" that keeps its distance and pokes from range) for as
long as you can. All gameplay logic — movement, combat, wave spawning,
win/lose — lives in Lua under `game/assets/scripts/`, not in the C++ engine
code.

## Running the tests

```powershell
cmake --preset windows-debug -DMOLLY_BUILD_TESTS=ON
cmake --build --preset windows-debug --target molly-tests --parallel
ctest --test-dir build/windows-debug -C Debug --output-on-failure
```

## Project layout

- `engine/` — the engine itself, built as a static library (`molly-engine`)
- `editor/` — the in-house editor (`molly-editor`)
- `game/` — the demo game (`molly-demo`)
- `tests/` — Catch2 test suite (`molly-tests`), opt-in via
  `-DMOLLY_BUILD_TESTS=ON`
- `vendor/` — every third-party dependency, vendored
- `plan/PLAN.md` — the full design/architecture document

## License

MIT — see [`LICENSE`](LICENSE).
