# CSE499A Presentation Notes — Molly Game Engine

Quick-reference talking points for presenting the CSE499A report. Organized to match the paper's section order — walk through top to bottom.

## 1. The one-line pitch

- **Molly** is a from-scratch C++20 game engine with its own visual editor, built to understand how engines actually work internally instead of using a black-box commercial one.
- Not trying to compete with Unity/Godot/Unreal — trying to be small enough to fully understand while still being structured the way a real engine is structured.

## 2. Why build an engine at all (Introduction)

- Commercial engines are powerful but too large/closed to learn *from* — you use them, you don't see how they're built.
- Two engine philosophies exist:
  - **Data-driven** (Unity/Godot/Unreal) — author everything in the editor, generic runtime executes it.
  - **Code-first** (id Tech/Source) — gameplay is compiled C++ linked to the engine; editor just authors levels.
  - Molly sits **in between**: data-driven scenes/prefabs (JSON) + Lua scripts, but no generic "player" runtime yet (that's a CSE499B goal).
- 9 stated objectives — rendering, ECS, physics, scripting, resources, editor, serialization, audio, playable demo. All delivered.

## 3. Related Work — don't just list libraries, explain the *idea*

- ECS (entity-component-system) is the core design pattern: composition over inheritance.
  - Classic problem it solves: an inheritance tree breaks down once an object needs behavior that crosses branches (e.g. something that's damageable *and* renderable *and* physically simulated) — cite West's "Evolve Your Hierarchy."
  - `entt` is the ECS library used — header-only, sparse-set based, industry-standard choice.
- Rendering follows standard real-time graphics practice (Akenine-Möller et al., *Real-Time Rendering*) — forward renderer, Blinn-Phong lighting, OpenGL 4.6 core profile (Khronos spec).
- Physics (Jolt) uses discrete/iterative constraint solving — the standard approach for *real-time* game physics, as opposed to continuous offline-tolerant simulation methods used in engineering software.

## 4. System Architecture — know the diagram cold

- Three build targets, one repo, **no external package manager** (vcpkg/Conan) — everything vendored as git submodules under `vendor/`. This was a deliberate constraint, not an accident.
  - `molly-engine` — static library, all reusable subsystems.
  - `molly-editor` — the visual editor, links the engine.
  - `molly-demo` — a hand-written sample game, links the engine the same way any third-party game would.
- **Be ready to explain the diagram** (Figure 1): editor and demo are independent executables → both link `molly-engine` → engine sits on top of vendored libraries (GLFW, GLM, entt, Jolt, sol2, Dear ImGui, Assimp, miniaudio, spdlog, nlohmann/json, Catch2).
- Frame loop order matters: `onUpdate()` for all layers → ImGui frame begins → `onImGuiRender()` for all layers → present → poll events. This exact ordering is what caused one of the bugs discussed later (mouse-wheel timing) — good bridge into the next section if asked.
- **Design requirements** (explicit non-functional constraints, not vibes):
  1. Self-contained (no package manager)
  2. Windows-only, OpenGL 4.6 core profile
  3. Human-readable (JSON) scene/prefab persistence
  4. Real-time responsiveness (60 FPS target)
  5. Fault containment — bad script or missing asset must degrade gracefully, never crash

## 5. The Editor — the part people will actually want to see

- Docking layout: Hierarchy (left) / Inspector (right) / Content Browser + Console (tabbed, bottom) / Viewport (center) — Unity/Godot-style.
- **The original bug**: ImGui's dockspace helper only creates the dock *host*, it doesn't assign panels to it — so panels fell back to a cascading, overlapping mess. Fixed with ImGui's `DockBuilder` API to explicitly construct the default layout on first run.
- Toolbar (Play/Pause/Stop + play-state badge), status bar, menu bar, consistent accent color theme.
- Viewport tooling: free-fly camera, scroll-zoom, "Frame Selected" (`F` key, same as Unity/Unreal/Blender), selection outline, and gizmo markers for lights/cameras (since those have no mesh and would otherwise be invisible/unselectable).

## 6. Engineering Challenges — this is your strongest storytelling material

Four real bugs, each with a clear cause-and-effect story. Practice telling these as *stories*, not just facts:

1. **Missing depth testing** — only the floor was rendering, a box prop was invisible. Turned out `GL_DEPTH_TEST` was never enabled anywhere, so draw order (not distance to camera) decided what was visible on top. `entt` iterates in *reverse creation order*, so the floor (made after the prop) was drawn over it.
2. **Mouse-wheel timing bug** — scroll-to-zoom did nothing. The zoom code read input in the wrong part of the frame loop (before `ImGui::NewFrame()` instead of after), so it was always reading a stale/unmerged value.
3. **Working-directory dependence** — assets loaded fine from a terminal but not from double-clicking the exe, because every asset path was relative to the *current working directory*, not the executable's location. Fixed by chdir-ing to the exe's own folder at startup + copying assets next to the build output.
4. **Invisible selection for lights/cameras** — "Frame Selected" *was* working, but lights/cameras have no mesh, so centering the camera on them showed nothing. Fixed with gizmo markers.
- **The unifying insight** (this is the one line a faculty member will remember): three of these four bugs share the same root cause shape — an *implicit assumption that held in the first simple test case but broke under an equally valid different one*. Lesson: test against scenes deliberately designed to violate assumptions (multiple overlapping objects, non-default timing, entities missing the "expected" components), not just whatever's already open.

## 7. Testing — be upfront about what's *not* covered

- Catch2 suite: 45 assertions / 7 test cases, all passing, runs in under 0.3s.
- Covers: serialization round-trips (scene/prefab), core component behavior.
- **Does NOT cover**: rendering output correctness, physics accuracy, editor UI behavior — say this proactively, it shows self-awareness rather than being caught out by a question.
- Fault-containment verified directly: bad Lua script → logged error, no crash. Missing texture → logged error, blank/unloaded texture, no crash.

## 8. Results — the numbers to have memorized

| Metric | Value |
|---|---|
| Engine source | 74 files / 4,577 lines |
| Editor source | 15 files / 1,481 lines |
| Demo game source | 3 files / 259 lines |
| Test suite | 45 assertions / 7 cases, <0.3s |
| `molly-editor.exe` (Debug) | ~18.5 MB |
| `molly-demo.exe` (Debug) | ~18.3 MB |
| Observed frame rate | ~75 FPS (informal, not a rigorous benchmark) |

- Be honest that the FPS number is a single informal observation, not a benchmark suite — a proper frame-time-vs-entity-count benchmark harness is explicitly proposed for CSE499B.

## 9. Limitations and Future Work — this doubles as your "roadmap" slide

Top 4 priorities for CSE499B, in order of impact:

1. **Materials as reusable assets** — right now a material is baked into each entity individually; can't create one and share it.
2. **Runtime scene/level switching** — currently exactly one `Scene` exists for the whole app's life; no way to load a second level.
3. **Generic runtime "Player" + Build/Export** — right now shipping a game means hand-writing a C++ project (`game/`) like `molly-demo`; a real workflow needs a generic runtime that just loads data.
4. **Viewport transform gizmo** — transforms are currently edited via number fields only, no drag-to-move/rotate/scale in the 3D view.

Plus a 5th cross-cutting gap: automated test coverage itself needs to grow (rendering/physics/UI tests), not just be "a milestone."

## 10. Ethical/Societal Considerations (short, don't over-invest time here)

- Offline tool, no data collection/telemetry → no privacy concerns.
- All vendored dependencies are permissively licensed (MIT/zlib/Apache-2.0-style).
- As a *tool* (not a game), content-ethics questions belong to whoever builds a game with it — same posture as Unity/Unreal/Godot.

## 11. Conclusion — the closing line

> "CSE499A delivered a functioning, intentionally minimal engine — validated end-to-end by a real demo game and an automated test suite — and just as importantly, an honest gap analysis that becomes the concrete scope for CSE499B."

---

# Anticipated Q&A

**Q: Why build your own engine instead of just using Unity or Godot?**
A: The goal wasn't to out-compete them — it was to understand how an engine's subsystems (renderer, ECS, physics, scripting, editor) are actually wired together end-to-end, which you can't learn from using a closed/massive commercial tool. Molly is deliberately small enough to read in full.

**Q: What's actually novel here — isn't this just gluing together existing libraries?**
A: The individual libraries (entt, Jolt, Dear ImGui, etc.) are vendored, yes — but the architecture decisions (layer-stack app loop, ECS-driven scene model, the editor's dock-builder layout, the edit/play-mode scene-cloning system, the fault-containment design) are original integration work, and several non-trivial bugs (Section 6) came directly from getting that integration wrong the first time and having to reason about *why*.

**Q: How do you know the engine actually performs well? You only have one FPS number.**
A: Honestly — we don't have rigorous performance data yet. That's explicitly called out as a limitation, not hidden. A proper benchmarking harness (frame time vs. entity/light count) is scoped as CSE499B work. The one number we do report (~75 FPS) is disclosed as informal.

**Q: What happens if a script crashes or an asset is missing at runtime?**
A: This was a deliberate design requirement (fault containment). Lua errors are caught and logged, not propagated — a broken script disables that one entity's behavior, not the whole app. Missing textures log an error and fall back to an unloaded/blank state instead of throwing. We verified both cases directly.

**Q: Why is the editor important if the engine already works via code (like `molly-demo`)?**
A: `molly-demo` shows the engine *works*, but it's hand-written C++ — that's not how you'd actually want to build a game day-to-day. The editor is what makes it a *tool* rather than just a library: visual scene editing, live Play-mode testing, drag-free-camera navigation, all without recompiling C++.

**Q: What's the single most significant bug you found, and why?**
A: The missing depth-test bug — a whole rendering object was invisible because draw order (not distance to camera) determined what showed on screen. It's significant not because it was hard to fix (one line: `glEnable(GL_DEPTH_TEST)`) but because of what it reveals: a very basic, "should always be on" GPU state was silently never enabled, and it stayed hidden until the scene had more than one overlapping object to expose it.

**Q: Why C++ and not something like Rust or C#?**
A: C++ is still the industry-standard systems language for real-time engines (Unreal, Unity's native core, id Tech, Source) — largely because of direct control over memory layout and zero-overhead abstractions, both of which matter for a real-time frame budget. It also let us integrate mature C++ libraries (Jolt, Assimp, Dear ImGui) directly.

**Q: What's the biggest thing you'd do differently if you started over?**
A: Build a minimal automated test harness for rendering/physics *earlier* — right now those are only verified manually/structurally (log inspection, `imgui.ini` structure checks), which is exactly the category of gap that let bugs like the depth-test issue go unnoticed until they were visually obvious.

**Q: Is this project just you, or a team? Who did what?**
A: [Answer live — the report intentionally does not include a contribution breakdown per author, so be ready to state this yourself rather than reading it off a slide.]

**Q: What's the plan for CSE499B specifically?**
A: Four prioritized items: materials as standalone reusable assets, runtime scene/level switching, a generic runtime "Player" plus a Build/Export pipeline from the editor, and a viewport translate/rotate/scale gizmo — ranked by how much each currently blocks "building an actual game" rather than just "having a working engine."
