# Molly Engine — Gaps Audit

Snapshot of what the engine/editor currently has vs. what's missing,
based on a direct code audit (not a wishlist) taken after Milestones
1–10 (`PLAN.md`) and the editor UI overhaul (`EDITOR_UI_PLAN.md`) were
complete. Dated 2026-09-03.

## Summary table

| Area | What We Have | What's Missing |
|---|---|---|
| **Rendering** | Forward renderer, basic Blinn-Phong lit shader, directional + point lights, textures | Shadows, PBR materials, skybox/IBL, post-processing, no shadow/light culling |
| **Editor viewport** | Fly camera, scroll zoom, selection outline, light/camera gizmo markers, frame-selected (F) | No translate/rotate/scale manipulator (drag gizmo) — Transform is edit-fields only |
| **Scene workflow** | Hierarchy/Inspector/Content Browser, scene + prefab JSON serialization | No Undo/Redo (menu items are placeholders), no multi-select, no drag-drop (Content Browser → Hierarchy/material slots) |
| **Scripting** | Lua via sol2, `ScriptComponent`, hot-reload for shaders/scripts | — |
| **Physics** | Jolt: rigid bodies, colliders, character controller | No joints/constraints editor, no physics debug-draw |
| **Audio** | miniaudio, 3D positional sources, listener | No mixer/bus system |
| **Shipping the game** | `molly-demo` — hand-written C++ project linking the engine | No generic runtime/"Player", no Build/Export from editor |
| **Content types** | Static meshes, materials, textures | No skeletal animation/rigging (Assimp's vendored but unused for that), no particle system, no 2D/sprite support, no in-game UI/HUD (Canvas) system |
| **Content Browser** | File tree, double-click to open externally | No thumbnails/previews, no drag-and-drop |
| **Platform** | Windows only (GLFW/Win32 throughout — icon, ShellExecute, etc.) | No Linux/Mac |
| **Tooling** | Catch2 test suite (45 assertions), FPS counter | No profiler, no CI pipeline, no project-settings/multi-scene project system |
| **Input** | Raw `glfwGetKey`/`glfwGetMouseButton` calls scattered across `ViewportPanel.cpp` and `ScriptEngine.cpp`; Lua gets `molly.input.is_key_down/is_mouse_down` | No `Input` abstraction layer at all — no polling wrapper, no action/rebinding system. Every caller talks to GLFW directly |
| **Script editing** | `ScriptComponent` shows the file path + a "Running/Not started" label in Inspector | No in-editor code editor — zero syntax highlighting, autocomplete, or inline error display. Editing a `.lua` file means opening it externally (Content Browser double-click → system default app) |
| **Project/workspace** | Nothing | No `.mollyproject` file, no project settings (default scene, input maps, build config), no project picker/launcher. Each exe is hardcoded to whatever `assets/` sits next to it |
| **Asset import pipeline** | `ResourceManager` is a path-keyed load-on-demand cache with a 1s-poll file watcher for hot-reload | No GUID/metadata system, no `.meta` sidecar files, no import-settings (texture compression, mesh scale, etc.) — purely "load this path when first referenced" |
| **Materials** | `Material` struct (shader + albedo color/texture + shininess) editable per-entity in Inspector | Not a standalone asset — serializes **inline** into each `MeshRendererComponent` in the scene JSON. No `.material` file you can create once and reuse/share across objects via Content Browser |
| **Runtime scene management** | Exactly one `Scene` for the entire app lifetime | No `SceneManager`, no `LoadScene`/level-transition API in C++ or Lua — you cannot go from one level to another while running |
| **Lua API surface** | `find_entity`, `destroy_entity`, `spawn_entity` (prefab instantiate), `move_towards`, `move_character`, `find_closest_entity`, `log`, `play_sound`, plus `input.is_key_down/is_mouse_down` | That's the entire surface. No timers/coroutines helpers, no broader scene queries, no UI or audio-control API beyond one-shot playback |
| **Prefabs** | "Create Prefab From Selection" (entity → prefab file); `spawn_entity` from Lua | One-directional only — no drag-a-prefab-from-Content-Browser-into-Hierarchy/Viewport to instantiate it in the editor |
| **Editor camera** | Single fly-cam (RMB+WASD/QE, scroll zoom, frame-selected) | No orbit-around-target mode, no orthographic/2D view toggle, no grid or transform-snapping |
| **Error resilience** | Bad Lua scripts log via `MOLLY_ERROR` and don't crash (`sol::script_pass_on_error`); missing textures degrade to an unloaded/blank state instead of throwing | — |

## Reading the table

Error handling and the Lua binding layer are more mature than the
surrounding tooling suggests — scripts fail safely, missing assets
degrade gracefully. The weak points are all *workflow*: no way to reuse
a material across objects, no way to switch levels at runtime, no way to
edit a script without leaving the editor, and no project concept binding
any of it together.

Highest game-breaking impact if the goal is "build a real game inside
the editor, not by hand-writing a `game/`-style C++ project":

1. Materials as standalone, reusable assets (currently inline-only per
   entity).
2. Runtime scene/level switching (currently one `Scene` for the app's
   whole lifetime).
3. A generic runtime "Player" + Build/Export pipeline, so shipping a
   game doesn't require a bespoke C++ project like `game/`.
4. A viewport translate/rotate/scale gizmo — right now every transform
   edit is via number fields, not direct manipulation.
