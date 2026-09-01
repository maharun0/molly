# Molly — 2.5D C++ Game Engine

**Comprehensive Engineering & Design Plan**

---

## Table of Contents

1. [Vision & Overview](#1-vision--overview)
2. [Goals & Non-Goals](#2-goals--non-goals)
3. [Tech Stack](#3-tech-stack)
4. [Repository & Directory Structure](#4-repository--directory-structure)
5. [Build System](#5-build-system)
6. [Dependency Vendoring](#6-dependency-vendoring)
7. [Coding Conventions](#7-coding-conventions)
8. [Core Architecture](#8-core-architecture)
9. [Platform & Windowing Layer](#9-platform--windowing-layer)
10. [Renderer Subsystem](#10-renderer-subsystem)
11. [ECS Subsystem](#11-ecs-subsystem)
12. [Physics Subsystem](#12-physics-subsystem)
13. [Scripting Subsystem](#13-scripting-subsystem)
14. [Resource / Asset Management](#14-resource--asset-management)
15. [Scene System](#15-scene-system)
16. [Audio Subsystem](#16-audio-subsystem)
17. [UI System](#17-ui-system)
18. [Editor Application](#18-editor-application)
19. [Input System](#19-input-system)
20. [Camera System](#20-camera-system)
21. [Math & Coordinate Conventions](#21-math--coordinate-conventions)
22. [Testing Strategy](#22-testing-strategy)
23. [CI/CD Pipeline](#23-cicd-pipeline)
24. [Performance Budgets & Profiling](#24-performance-budgets--profiling)
25. [Error Handling & Logging Philosophy](#25-error-handling--logging-philosophy)
26. [Memory Management Strategy](#26-memory-management-strategy)
27. [Threading Model](#27-threading-model)
28. [Versioning & Release Process](#28-versioning--release-process)
29. [Git Workflow & Branching Strategy](#29-git-workflow--branching-strategy)
30. [Documentation Strategy](#30-documentation-strategy)
31. [Demo Game Design Document](#31-demo-game-design-document)
32. [Milestones — Detailed Task Breakdown](#32-milestones--detailed-task-breakdown)
33. [Risk Register](#33-risk-register)
34. [Glossary](#34-glossary)
35. [Appendix A — Component Reference](#35-appendix-a--component-reference)
36. [Appendix B — Lua API Sketch](#36-appendix-b--lua-api-sketch)
37. [Appendix C — Scene JSON Format](#37-appendix-c--scene-json-format)
38. [Appendix D — Engine Class Sketches](#38-appendix-d--engine-class-sketches)
39. [Appendix E — Editor Keybindings](#39-appendix-e--editor-keybindings)
40. [Appendix F — CMake Skeleton](#40-appendix-f--cmake-skeleton)
41. [Appendix G — Sample Shaders](#41-appendix-g--sample-shaders)
42. [Appendix H — Sample Catch2 Test](#42-appendix-h--sample-catch2-test)
43. [Appendix I — CI Workflow](#43-appendix-i--ci-workflow)
44. [Appendix J — `.clang-format`](#44-appendix-j--clang-format)
45. [Decision Log](#45-decision-log)
46. [Post-v1 Backlog](#46-post-v1-backlog-ideas-deliberately-not-scheduled)
47. [Accessibility Notes](#47-accessibility-notes-v1-scope)
48. [Project Configuration File](#48-project-configuration-file)
49. [Frequently Revisited Questions](#49-frequently-revisited-questions)

---

## 1. Vision & Overview

Molly is a hobby-scale, from-scratch C++ game engine built to run on Windows
11, developed in Visual Studio Code, targeting **2.5D top-down action games**
— games rendered with a real 3D pipeline (lighting, depth, perspective or
orthographic-angled cameras) but played on an effectively 2D plane, in the
spirit of *Hades*, *Enter the Gungeon*, *Death's Door*, and *Diablo*-likes.

The project has two intertwined purposes:

1. **Learning vehicle** — a from-scratch engine forces first-hand
   understanding of rendering pipelines, ECS architecture, physics
   integration, scripting embedding, and editor tooling, instead of treating
   these as black boxes provided by Unity/Unreal/Godot.
2. **Working tool** — the engine should reach a point where it can actually
   ship a small, complete game, not remain a permanent tech demo. Every
   milestone is scoped so there is always a runnable, visible result.

The 2.5D top-down genre was deliberately chosen as the "proving ground"
because it forces the engine to get rendering, physics, scripting, and
editor tooling right, while deliberately avoiding the single biggest time
sink in hobby engines: skeletal animation and rigging. Movement and gameplay
logic stay on a 2D plane (X/Z), while the renderer still does full 3D work
(lighting, meshes, depth, shadows-later), which is a deliberate stepping
stone toward a more general-purpose 3D engine if the project continues past
v1.

### 1.1 Guiding Principles

- **Own the whole stack, vendor the dependencies.** Every third-party
  library lives inside the repository. There is no reliance on system-wide
  installs, global package caches, or "works on my machine" toolchains.
  Clone the repo, run one submodule command, build.
- **Data-oriented where it matters, pragmatic everywhere else.** entt is
  used for gameplay objects because cache-friendly iteration matters for a
  real-time engine loop, but the engine is not on a mission to be
  dogmatically data-oriented in every subsystem (e.g. the editor's UI code
  is plain object-oriented ImGui glue, because that's what ImGui wants).
- **C++ owns systems, Lua owns gameplay.** The boundary between engine code
  and game code is the scripting layer. Engine programmers write C++
  systems; game logic (enemy behavior, abilities, triggers) is written in
  Lua so iteration doesn't require a recompile.
- **Editor and game share one engine library.** There is no separate
  "runtime-only" build vs "editor" build fork at this stage — the editor is
  just another application linked against the engine static library, with
  ImGui panels layered on top.
- **Ship small, ship often.** Each milestone below produces something that
  can be run and looked at. No milestone should take so long that motivation
  dies before there's something to show.

### 1.2 Target Platform

- **Primary and only supported platform for v1**: Windows 11, x64, Visual
  Studio 2022 (MSVC) toolchain, built via CMake from VS Code.
- OpenGL 4.6 core profile is required (verify GPU/driver support up front —
  virtually all Windows 11 hardware from the last decade supports this).
- Cross-platform support (Linux, macOS) is explicitly deferred; the code
  should *not* go out of its way to use Windows-only APIs where a portable
  alternative is equally convenient (e.g. prefer `std::filesystem` over
  `Win32` file APIs), but no engineering time is spent validating or testing
  other platforms until after v1.

---

## 2. Goals & Non-Goals

### 2.1 Goals

- A working rendering pipeline: window + OpenGL context + shader pipeline +
  mesh/texture loading + basic lighting + a top-down camera.
- A working ECS-driven scene model using entt.
- Physics via Jolt: static/dynamic bodies, a top-down character controller,
  collision detection/response, trigger volumes.
- Embedded Lua scripting via sol2, bound to transform/input/physics, driving
  gameplay logic without recompilation.
- A functional in-house editor: hierarchy, inspector, viewport,
  content browser, play/pause/stop.
- Scene and prefab serialization to a human-readable JSON format.
- Basic audio playback (SFX + music, 2D and simple positional).
- A small, complete demo game built on top of all of the above, proving the
  engine works end-to-end for a real (if small) game.
- A repository that is **fully self-contained**: clone + submodule init +
  CMake build, with zero external package manager dependency.

### 2.2 Non-Goals (for v1)

- Skeletal animation / rigging / animation blending.
- Networking or multiplayer of any kind.
- Advanced rendering: PBR material pipelines, shadow mapping, global
  illumination, post-processing stacks. Basic Phong/Blinn-Phong lighting is
  sufficient until the demo game's needs outgrow it.
- A general-purpose, data-driven in-game UI system beyond what ImGui can do
  directly. Building a real HUD/menu system is deferred until a real game
  needs one badly enough to justify the design work.
- Cross-platform builds (Linux/macOS).
- Console/mobile ports.
- A visual scripting system (node graphs, Blueprints-style) — Lua text
  scripting only.
- Asset marketplace/plugin ecosystem, hot-reloadable C++ modules, or any
  other "engine as a product" concerns. This is a single-project engine
  first; generality is not a goal in itself.

---

## 3. Tech Stack

| Layer                | Choice            | Version pin (initial) | Rationale |
|-----------------------|-------------------|------------------------|-----------|
| Language              | C++20             | MSVC v143 toolset      | RAII, concepts, ranges (partial MSVC support), `std::span`, structured bindings |
| Graphics API          | OpenGL             | 4.6 core profile       | Mature, extremely well documented, huge tutorial/reference base for a learning project |
| GL function loader    | GLAD                | generated per-build    | Simple, single generated source file, no runtime dependency |
| Windowing / Input     | GLFW               | 3.4                     | De facto standard OpenGL windowing library; clean C API |
| Math                  | GLM                 | 1.0.x                   | GLSL-style vector/matrix types, header-only |
| ECS                   | entt                | 3.13.x                  | Fast, header-only, sparse-set ECS, huge adoption in hobby/indie engines |
| Scripting language    | Lua                 | 5.4.x                    | Small, fast, the standard embeddable scripting language |
| Lua binding           | sol2                | 3.3.x                    | Ergonomic, modern C++ binding layer over the Lua C API |
| Physics               | Jolt Physics        | latest stable            | Modern, actively maintained (used in Horizon Forbidden West), fast, MIT-licensed |
| Immediate-mode UI     | Dear ImGui          | docking branch           | Editor panels + early in-game UI; docking branch needed for dockable editor layout |
| Model loading         | Assimp              | 5.4.x                    | Broad format support: glTF, FBX, OBJ |
| Texture loading       | stb_image           | single header             | Trivial to vendor, covers PNG/JPG/TGA |
| Audio                 | miniaudio            | single header             | No external deps, covers playback + basic mixing/3D positioning |
| Logging               | spdlog               | 1.14.x                    | Fast, header-friendly, printf- and fmt-style API |
| Serialization         | nlohmann/json         | 3.11.x                    | De facto standard C++ JSON library, single header available |
| Testing               | Catch2                | 3.x                        | BDD-style option, single-header-friendly, first-class CMake integration (`catch_discover_tests`) |
| Profiling (post-v1)   | Tracy                 | latest                     | Frame-time / GPU profiling once performance work starts |
| Build system          | CMake                 | 3.28+                      | Standard for C++ cross-toolchain builds |
| Dependency strategy   | Vendored git submodules | n/a                    | Fully self-contained repository (see §6) |
| Version control       | Git + Git LFS          | n/a                        | LFS for binary assets (textures, models, audio) once the demo game has real content |
| License               | MIT                    | n/a                        | Permissive, standard default for a hobby/open engine |
| CI                    | GitHub Actions          | n/a                        | Windows/MSVC build + test verification on push/PR |
| Coding style enforcement | clang-format         | LLVM-based config          | Consistent formatting, checked in as `.clang-format` |

### 3.1 Alternatives Considered

Documenting what was *not* chosen, and why, so future-me doesn't re-litigate
these decisions from scratch:

- **Vulkan instead of OpenGL** — rejected for v1. Vulkan's verbosity is a
  poor fit for a learning-focused solo project's early milestones; OpenGL
  gets a triangle on screen in an afternoon instead of a week. Could be
  revisited as a v2 renderer backend behind an abstraction layer.
- **SDL2 instead of GLFW** — SDL2 does more (audio, input, more platform
  coverage) but GLFW's API is smaller and more focused, which matters more
  for a learning project than SDL's broader feature set which mostly goes
  unused here (audio is already covered by miniaudio).
- **Box2D instead of Jolt** — Box2D is 2D-only. Since the renderer is fully
  3D even though gameplay is 2D-plane-constrained, a 3D physics engine
  (Jolt) is the correct fit — it also leaves the door open to fully 3D
  gameplay later without a physics engine swap.
- **Bullet Physics instead of Jolt** — Bullet is older, more fragmented API,
  less actively maintained. Jolt is the more modern choice with cleaner
  ergonomics and better documentation for a newer project.
- **flecs instead of entt** — both are excellent; entt was chosen for its
  larger hobby-engine community/tutorial base, which matters for a solo
  learning project working things out mostly from documentation and
  examples.
- **GameObject/Component OOP model instead of ECS** — rejected; the
  cache-friendly, data-oriented iteration model of an ECS is one of the
  explicit things this project exists to learn, and Unity-style OOP
  component models are already familiar territory.
- **vcpkg/Conan instead of vendored submodules** — rejected per explicit
  project requirement: everything needed to build must live inside the
  repository, with no dependency on a global package manager or cache
  living outside the project tree.
- **Dear ImGui non-docking branch** — rejected; the docking branch's
  dockable/floating panel model is essentially required for a usable
  editor layout (viewport + hierarchy + inspector + content browser all
  need to dock/resize/float).

---

## 4. Repository & Directory Structure

```
molly/
  .clang-format
  .editorconfig
  .gitattributes            # LFS tracking rules
  .gitignore
  .gitmodules                # all vendor/ submodules
  CMakeLists.txt              # root build script
  CMakePresets.json            # named configure/build presets (Debug/Release, MSVC)
  LICENSE                       # MIT
  PLAN.md                        # this document
  README.md                       # quickstart: clone, submodule init, build, run

  .github/
    workflows/
      build.yml                  # CI: configure, build, run tests on Windows/MSVC

  cmake/
    CompilerWarnings.cmake        # shared warning flags across targets
    Sanitizers.cmake               # optional ASan/UBSan toggle for Debug builds
    Vendor.cmake                    # helper functions for wiring up vendored libs

  vendor/                            # ALL third-party dependencies, vendored
    glfw/                              # submodule
    glad/                               # generated loader source, checked in directly
      include/glad/gl.h
      src/gl.c
    glm/                                 # submodule
    entt/                                 # submodule
    imgui/                                 # submodule (docking branch)
      backends/                              # imgui_impl_glfw.*, imgui_impl_opengl3.*
    lua/                                     # submodule (or vendored source drop)
    sol2/                                     # submodule (single header variant)
    JoltPhysics/                               # submodule
    assimp/                                     # submodule
    stb/                                          # vendored headers (stb_image.h)
    miniaudio/                                     # vendored header (miniaudio.h)
    spdlog/                                         # submodule
    json/                                             # submodule (nlohmann/json, single include)
    catch2/                                            # submodule

  engine/                             # Molly engine — static library `molly-engine`
    CMakeLists.txt
    include/molly/
      core/
        Application.h
        Window.h
        Log.h
        Time.h
        Events/
          Event.h
          KeyEvent.h
          MouseEvent.h
          WindowEvent.h
        Layer.h
        LayerStack.h
      renderer/
        RendererAPI.h
        Renderer.h
        Renderer2D.h                    # optional: batched sprite/quad renderer
        Shader.h
        Buffer.h                         # VBO/EBO/VAO wrappers
        VertexArray.h
        Texture.h
        Framebuffer.h
        Mesh.h
        Material.h
        Camera.h
        OrthographicCamera.h
        PerspectiveCamera.h
        Light.h
      ecs/
        Scene.h
        Entity.h
        Components.h
        Systems/
          RenderSystem.h
          PhysicsSyncSystem.h
          ScriptSystem.h
      physics/
        PhysicsWorld.h
        RigidBody.h
        CharacterController.h
        CollisionLayers.h
      scripting/
        LuaVM.h
        ScriptEngine.h
        LuaBindings.h
      audio/
        AudioEngine.h
        AudioSource.h
      resource/
        ResourceManager.h
        AssetHandle.h
        AssetLoaders/
          TextureLoader.h
          ModelLoader.h
          ShaderLoader.h
          ScriptLoader.h
      scene/
        SceneSerializer.h
        Prefab.h
      ui/
        ImGuiLayer.h
        UiWidgets.h
      Molly.h                           # single umbrella include for consumers
    src/
      (mirrors include/ structure with .cpp files)

  editor/                              # `molly-editor` executable
    CMakeLists.txt
    src/
      EditorApp.cpp
      EditorLayer.h / .cpp
      Panels/
        SceneHierarchyPanel.h / .cpp
        InspectorPanel.h / .cpp
        ViewportPanel.h / .cpp
        ContentBrowserPanel.h / .cpp
        ConsolePanel.h / .cpp
      main.cpp
    assets/
      editor_icons/
      default_layout.ini              # ImGui docking layout default

  game/                                 # `molly-demo` executable — the 2.5D demo game
    CMakeLists.txt
    src/
      GameApp.cpp
      Scripts/                           # (native fallback scripts / bootstrapping only)
      main.cpp
    assets/
      scenes/
      scripts/                             # Lua game logic
      models/
      textures/
      audio/
      shaders/

  tests/                                  # `molly-tests` executable (Catch2)
    CMakeLists.txt
    core/
    renderer/
    ecs/
    physics/
    scripting/
    resource/
    scene/

  tools/                                    # standalone helper scripts (non-shipping)
    scripts/
      bootstrap.ps1                            # git submodule update --init --recursive wrapper
      gen_glad.md                                # notes on regenerating the GLAD loader
```

### 4.1 Directory Ownership Rules

- `engine/` never depends on `editor/` or `game/`. Dependency direction is
  strictly one-way: `game`/`editor` → `engine` → `vendor`.
- `editor/` may depend on `engine/` only. It must not reach into `game/`.
- `tests/` links against `engine/` (and, where useful, individual internal
  headers) but never against `editor/` or `game/`.
- Anything under `vendor/` is never modified in place. If a vendored library
  needs a patch, it's applied via a small local patch file tracked in
  `cmake/patches/`, not by hand-editing submodule contents (which would be
  lost on update and is invisible to reviewers).

---

## 5. Build System

### 5.1 Why CMake

CMake is the only realistic choice for a C++ project that needs to: build
multiple targets (engine lib, editor exe, game exe, tests exe), pull in a
dozen vendored libraries with varied build systems (some CMake-native, some
header-only, some needing custom target definitions), and stay usable from
VS Code via the CMake Tools extension without depending on a full Visual
Studio solution file being checked in.

### 5.2 Top-Level Structure

The root `CMakeLists.txt` declares the project, sets the C++ standard,
includes the shared `cmake/` helper modules, then adds subdirectories in
dependency order:

```
project(Molly LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(vendor)   # defines vendored library targets
add_subdirectory(engine)   # defines molly-engine
add_subdirectory(editor)   # defines molly-editor, links molly-engine
add_subdirectory(game)     # defines molly-demo, links molly-engine
add_subdirectory(tests)    # defines molly-tests, links molly-engine + catch2
```

### 5.3 CMake Presets

`CMakePresets.json` defines named presets so building from VS Code or the
command line is a single command, with no need to remember generator/flag
combinations:

- `windows-debug` — MSVC, `Debug`, sanitizers optionally on, tests enabled.
- `windows-release` — MSVC, `RelWithDebInfo` (kept debuggable — pure
  `Release` is only for final builds), tests enabled.
- `windows-release-final` — MSVC, `Release`, tests disabled, used only when
  producing a build to hand off/share.

Each preset pins the generator to `"Visual Studio 17 2022"` with `x64`
architecture, and sets `CMAKE_EXPORT_COMPILE_COMMANDS=ON` so clangd/VS Code
IntelliSense (if used) gets accurate compile flags.

### 5.4 Warning & Sanitizer Policy

- `cmake/CompilerWarnings.cmake` defines an interface target `molly_warnings`
  applying `/W4` on MSVC (treat as close to "all reasonable warnings on" as
  practical without drowning in vendored-header noise — vendored includes
  are pulled in via `-external:I` / `SYSTEM` treatment so their warnings
  don't count against the engine's own code).
- Warnings are **not** treated as errors globally in early milestones (that
  would slow down exploratory work); this is revisited once the core
  systems (Milestones 1–5) stabilize, at which point `/WX` is turned on for
  `engine/` specifically.
- `cmake/Sanitizers.cmake` provides an opt-in `MOLLY_ENABLE_ASAN` CMake
  option for Debug builds, useful when chasing memory bugs in the physics/
  scripting boundary code.

### 5.5 Building From VS Code

- Install the **CMake Tools** and **C/C++** extensions.
- `Ctrl+Shift+P` → "CMake: Select Configure Preset" → pick `windows-debug`.
- `Ctrl+Shift+P` → "CMake: Build" (or the status bar build button) builds
  all targets; individual targets (`molly-editor`, `molly-demo`,
  `molly-tests`) can be selected as the active target for build/run/debug.
- `launch.json` is checked in with debug configurations for `molly-editor`
  and `molly-demo`, both pointing `cwd` at their respective `assets/`
  directory so relative asset paths resolve correctly when debugging.

---

## 6. Dependency Vendoring

### 6.1 Policy

Every third-party dependency is vendored **inside the repository** under
`vendor/`. Nothing required to build the project is fetched from, or
expected to already exist in, a location outside the project directory —
no vcpkg, no Conan, no system-wide installed libraries, no environment
variables pointing outside the repo tree. The only external tool required
is Git itself (for submodules) and the MSVC/CMake toolchain.

### 6.2 Setup Step

```powershell
git clone <repo-url> molly
cd molly
git submodule update --init --recursive
```

That's the entire setup. `tools/scripts/bootstrap.ps1` wraps this (plus a
CMake configure) into a single command for convenience, but it does nothing
that isn't achievable with plain `git`/`cmake` commands — no hidden magic.

### 6.3 Per-Library Integration Notes

| Library      | Integration method | Notes |
|--------------|--------------------|-------|
| GLFW          | `add_subdirectory(vendor/glfw)`, link `glfw` | Disable `GLFW_BUILD_DOCS`/`GLFW_BUILD_TESTS`/`GLFW_BUILD_EXAMPLES` via `set(... OFF)` before adding, to keep configure/build fast |
| GLAD           | Checked-in generated source (not a submodule) | Generated once via the glad web generator (OpenGL 4.6 core, no extensions beyond what's needed) and committed directly, since it's generated output rather than a library with its own repo to track |
| GLM             | `add_subdirectory(vendor/glm)`, header-only interface target | `GLM_ENABLE_EXPERIMENTAL` left off unless a specific experimental header is needed |
| entt             | Header-only; add `vendor/entt/single_include` as an `INTERFACE` include dir | Using the single-header amalgamated distribution avoids needing entt's own CMake build |
| Dear ImGui        | No native CMake; hand-rolled `molly-imgui` static lib target in `vendor/CMakeLists.txt` compiling core `.cpp` files + `backends/imgui_impl_glfw.cpp` + `backends/imgui_impl_opengl3.cpp` | Docking branch checked out explicitly |
| Lua                 | Hand-rolled `molly-lua` static lib target compiling the Lua C sources (excluding `lua.c`/`luac.c` which are the standalone interpreter/compiler, not needed as a library) | Compiled as C, not C++ (Lua's headers handle `extern "C"` when included from C++) |
| sol2                  | Header-only; `INTERFACE` include dir pointing at `vendor/sol2/include`, requires linking against the `molly-lua` target above | `SOL_ALL_SAFETY_ON_OFF` tuned per build config (safety checks on in Debug, off in Release for perf) |
| Jolt Physics            | `add_subdirectory(vendor/JoltPhysics/Build)` per Jolt's own CMake layout, link `Jolt` | Double-precision mode left off (single precision is sufficient and faster for this scale of game) |
| Assimp                    | `add_subdirectory(vendor/assimp)` with importer list trimmed via `ASSIMP_BUILD_<X>_IMPORTER` flags to only the formats actually needed (glTF2, OBJ, FBX) to keep build times down | Exporters, tools, and tests disabled |
| stb_image                   | Single header, `INTERFACE` include dir; one `.cpp` in `engine/src/resource/` does `#define STB_IMAGE_IMPLEMENTATION` | Standard header-only pattern |
| miniaudio                     | Single header, `INTERFACE` include dir; one `.cpp` in `engine/src/audio/` does `#define MINIAUDIO_IMPLEMENTATION` | Same pattern as stb_image |
| spdlog                          | `add_subdirectory(vendor/spdlog)`, link `spdlog::spdlog` | Header-only mode considered but compiled-library mode chosen for faster incremental builds |
| nlohmann/json                     | Header-only single-include; `INTERFACE` include dir | No CMake subdirectory needed |
| Catch2                              | `add_subdirectory(vendor/catch2)`, link `Catch2::Catch2WithMain`; `include(CTest)` + `catch_discover_tests` in `tests/CMakeLists.txt` | Standard Catch2 v3 CMake integration |

### 6.4 Version Pinning

Each submodule is pinned to a specific commit/tag (not tracking a floating
branch) so builds are reproducible. Upgrading a dependency is a deliberate
action: `cd vendor/<lib> && git checkout <new-tag> && cd ../.. && git add
vendor/<lib> && git commit`. Dependency upgrades get their own commit,
never bundled silently into an unrelated feature commit.

### 6.5 Non-Submodule Vendoring (fallback path)

If a library doesn't have a clean git repo to submodule (or the user
prefers to hand-place files), the fallback is: download the source/release
archive, extract it directly into `vendor/<lib>/`, and commit it as regular
tracked files rather than a submodule. The CMake wiring in
`vendor/CMakeLists.txt` is agnostic to whether a given `vendor/<lib>/`
directory is a submodule or a plain committed directory — both look the
same to the build.

---

## 7. Coding Conventions

### 7.1 Formatting

- `.clang-format` at repo root, LLVM-derived base style with these
  overrides: 4-space indentation, 100-column limit, braces on the same line
  for control statements (`if (x) {`), Allman-style braces for function/
  class bodies is *not* used — consistent "attached brace" style throughout.
- Format-on-save enabled via VS Code `settings.json` (`editor.formatOnSave`
  scoped to C++ files) so formatting is never a manual/reviewable concern.

### 7.2 Naming

| Category                  | Convention        | Example |
|----------------------------|--------------------|---------|
| Types (class/struct/enum)   | `PascalCase`        | `class RigidBody`, `enum class LightType` |
| Functions & methods           | `camelCase`          | `void updateTransform()` |
| Local variables & parameters    | `camelCase`            | `float deltaTime` |
| Member variables                  | `m_camelCase`            | `float m_health` |
| Static member variables             | `s_camelCase`              | `static int s_instanceCount` |
| Global variables (avoid where possible) | `g_camelCase`         | `g_engineVersion` |
| Constants / `constexpr`                | `kPascalCase`           | `constexpr int kMaxEntities = 100000;` |
| Macros (rare — prefer constexpr/templates) | `SCREAMING_SNAKE_CASE` | `#define MOLLY_ASSERT(x)` |
| Namespaces                                | lowercase, single word    | `namespace molly { namespace renderer { ... } }` |
| File names                                  | match primary class name    | `RigidBody.h` / `RigidBody.cpp` |
| Lua-exposed identifiers (see §13)             | `snake_case`                   | matches Lua community convention, not C++ convention, deliberately, so scripts read like idiomatic Lua |

### 7.3 Header Hygiene

- `#pragma once` in every header (not include guards — MSVC supports it
  universally and it's simpler).
- Forward-declare where possible in headers; only `#include` what a header
  itself needs to compile, not what its consumers happen to also need
  (no "convenience" transitive includes).
- One primary class per header/source pair. Small tightly-coupled helper
  types may share a file (e.g. `Event.h` holding the `Event` base class and
  small event category enums).
- `Molly.h` is the single umbrella header for `game`/`editor` code that
  wants "the whole engine API" without hunting for individual headers —
  internal engine `.cpp` files never include this umbrella header
  themselves (avoids unnecessary compile-time bloat and hides internal
  implementation headers from consumers who only need the public API).

### 7.4 Error Handling Philosophy

See §25 for the full policy; summarized here for the style guide:

- Assertions (`MOLLY_ASSERT`) for programmer errors / invariant violations —
  compiled out in `Release` builds.
- Return values (`std::optional`, small result/status types) for expected,
  recoverable failure conditions (e.g. "texture file not found").
- Exceptions are **not** used in engine code, to keep the ABI simple and
  avoid exception-handling overhead in hot loops; the one exception (pun
  intended) is that Lua/sol2 errors surface as C++ exceptions internally to
  `sol2`, but these are caught at the script-call boundary and converted to
  logged errors — they never propagate into engine code.

### 7.5 Ownership & Smart Pointers

- `std::unique_ptr` is the default for owned heap objects.
- `std::shared_ptr` is reserved for genuinely shared-lifetime resources
  (loaded textures/meshes/shaders managed by the `ResourceManager`, where
  multiple entities/materials reference the same underlying GPU resource).
- Raw pointers/references are used for non-owning "borrowed" access — a
  raw pointer in a function signature always means "does not own this."
- entt entities themselves are lightweight handles (`entt::entity`, an
  integer), not smart-pointer-managed at all — ownership lives in the
  `entt::registry`.

### 7.6 Comments

- No comments explaining *what* code does — names should carry that.
- Comments are reserved for *why*: non-obvious constraints, workarounds for
  a specific library quirk (e.g. "Jolt requires this to be 16-byte aligned"),
  or a warning about a subtle invariant a future editor of the code needs to
  preserve.
- Every public header gets a one-line Doxygen-style `///` summary on each
  public class/function *only* where the name doesn't already make the
  purpose obvious (favor good names over documentation compensating for bad
  ones).

---

## 8. Core Architecture

### 8.1 Layering

```
 ┌─────────────────────────────────────────────┐
 │   game/  (molly-demo)     editor/ (molly-editor) │
 └───────────────────────┬───────────────────────┘
                          │  links against
 ┌───────────────────────▼───────────────────────┐
 │                engine/ (molly-engine)              │
 │  ┌───────────┐ ┌────────┐ ┌─────────┐ ┌─────────┐  │
 │  │  renderer  │ │  ecs   │ │ physics │ │scripting│  │
 │  └───────────┘ └────────┘ └─────────┘ └─────────┘  │
 │  ┌───────────┐ ┌────────┐ ┌─────────┐ ┌─────────┐  │
 │  │  resource  │ │ scene  │ │  audio  │ │   ui    │  │
 │  └───────────┘ └────────┘ └─────────┘ └─────────┘  │
 │  ┌────────────────────────────────────────────┐    │
 │  │             core (Application, Window, Log,       │
 │  │             Time, Events, LayerStack)              │
 │  └────────────────────────────────────────────┘    │
 └───────────────────────┬───────────────────────┘
                          │  wraps
 ┌───────────────────────▼───────────────────────┐
 │  vendor/ (GLFW, GLAD, GLM, entt, Jolt, Lua/sol2,   │
 │           ImGui, Assimp, stb_image, miniaudio,      │
 │           spdlog, nlohmann/json, Catch2)             │
 └─────────────────────────────────────────────────┘
```

### 8.2 Application & Layer Stack

The engine's `Application` class owns the main loop and a `LayerStack` — a
Cherno-engine-style pattern that is well suited to a solo learning project
because it keeps the loop generic while letting `editor`/`game` push their
own layers with `onAttach`/`onUpdate`/`onImGuiRender`/`onEvent` hooks.

```cpp
class Application {
public:
    void run();               // owns the main loop
    void pushLayer(Layer* layer);
    void pushOverlay(Layer* overlay);   // rendered/updated after regular layers
    void onEvent(Event& e);

private:
    Window m_window;
    LayerStack m_layerStack;
    bool m_running = true;
    float m_lastFrameTime = 0.0f;
};
```

Main loop shape (conceptually — see §27 for the fixed/variable timestep
split):

```cpp
void Application::run() {
    while (m_running) {
        float time = getTime();
        float dt = time - m_lastFrameTime;
        m_lastFrameTime = time;

        for (Layer* layer : m_layerStack)
            layer->onUpdate(dt);

        m_imguiLayer->begin();
        for (Layer* layer : m_layerStack)
            layer->onImGuiRender();
        m_imguiLayer->end();

        m_window.onUpdate();  // poll events, swap buffers
    }
}
```

`molly-editor` pushes an `EditorLayer`; `molly-demo` pushes a `GameLayer`.
Both are built from the same `Application`/`Window`/`LayerStack` machinery
in `engine/`.

### 8.3 Event System

- Events (`WindowResizeEvent`, `KeyPressedEvent`, `MouseMovedEvent`, etc.)
  are simple structs derived from an `Event` base with a category bitmask
  and a `handled` flag.
- Dispatched top-down through the layer stack (most-recently-pushed layer
  first, so ImGui/editor overlays can consume input before it reaches
  underlying game layers) via a small `EventDispatcher` helper that
  type-matches and invokes a handler lambda.
- No global event bus / pub-sub system for gameplay events in v1 — that's
  explicitly deferred; window/input events use this dispatcher, gameplay
  events (e.g. "enemy died") are handled directly via ECS systems reacting
  to component state, or via direct Lua callbacks, not a generic event bus.

### 8.4 Logging

- `Log::init()` sets up two spdlog loggers: `"MOLLY"` (engine-internal) and
  `"APP"` (game/editor-level), both writing to a colored console sink and a
  rotating file sink (`logs/molly.log`).
- Convenience macros: `MOLLY_TRACE/INFO/WARN/ERROR/CRITICAL(...)` for engine
  code, `APP_TRACE/INFO/WARN/ERROR/CRITICAL(...)` for game/editor code —
  both compile out `TRACE` in non-Debug builds.
- The editor's Console panel (§18) subscribes to a custom spdlog sink that
  forwards formatted log lines into an in-memory ring buffer for on-screen
  display, so engine/game logs are visible inside the editor itself, not
  just in a terminal.

### 8.5 Time

- `Time::getSeconds()` wraps `glfwGetTime()`.
- `deltaTime` is clamped to a maximum (e.g. 0.1s) to avoid a "spiral of
  death" if the application is paused in a debugger or the OS stalls the
  process — a single abnormally long frame should not translate into a huge
  simulated timestep.

---

## 9. Platform & Windowing Layer

### 9.1 Window

- `Window` wraps a `GLFWwindow*`, owns creation/destruction, and exposes
  `onUpdate()` (poll events + swap buffers), `getWidth()/getHeight()`,
  `setVSync(bool)`, and a `setEventCallback(fn)` used by `Application` to
  route GLFW callbacks into the engine's `Event` system.
- GLFW callbacks (`glfwSetKeyCallback`, `glfwSetCursorPosCallback`,
  `glfwSetWindowSizeCallback`, etc.) are registered once at window creation
  and translate directly into constructing the matching `Event` subtype and
  invoking the stored callback — GLFW-specific code never leaks past the
  `Window` class boundary.
- OpenGL context creation requests version 4.6 core profile explicitly;
  context creation failure is a hard, loud startup error (logged + message
  box), not a silent fallback to a lower version, since the renderer relies
  on 4.6-specific features (e.g. direct state access functions) once it
  matures past the bootstrap milestone.

### 9.2 GLAD Integration

- GLAD is generated once (OpenGL 4.6 core, no extensions beyond
  `GL_ARB_direct_state_access` if needed) via the online generator, and the
  generated `gl.h`/`gl.c` are checked into `vendor/glad/` directly — see
  `tools/scripts/gen_glad.md` for exact regeneration steps if the extension
  set ever needs to change.
- `gladLoadGL(glfwGetProcAddress)` is called immediately after
  `glfwMakeContextCurrent`, inside `Window`'s creation path, before any
  other GL call anywhere in the engine.

### 9.3 VSync & Frame Pacing

- VSync is on by default (`glfwSwapInterval(1)`), toggleable from the
  editor's settings/debug menu once that exists (Milestone 7+). No frame
  rate cap logic beyond VSync in v1 — uncapped/custom frame limiting is not
  needed at this scope.

---

## 10. Renderer Subsystem

### 10.1 Design Approach

The renderer is a thin, explicit wrapper over OpenGL — not an abstract
"RHI" designed to someday swap in Vulkan/DirectX. A one-layer
`RendererAPI` interface exists (see §10.2) purely to keep raw `gl*` calls
out of gameplay/ECS-facing code, not because a second backend is planned
for v1. This keeps the renderer approachable while leaving a clean seam if
a second backend is ever pursued post-v1.

### 10.2 Core Types

- **`RendererAPI`** — thin static-ish wrapper: `clear()`, `setClearColor()`,
  `drawIndexed(VertexArray&)`, `setViewport()`. Every raw `glClear`,
  `glDrawElements`, etc. call in the engine goes through here, nowhere else.
- **`Shader`** — compiles/links a vertex+fragment (later, optionally
  geometry) program from source files; exposes `bind()`/`unbind()` and
  typed uniform setters (`setMat4`, `setFloat3`, `setInt`, etc.) via
  cached uniform location lookups.
- **`Buffer`** family — `VertexBuffer`, `IndexBuffer`, with a
  `BufferLayout` describing attribute stride/offset/type, consumed by
  `VertexArray` to configure `glVertexAttribPointer` calls automatically
  instead of hand-writing attribute setup per mesh type.
- **`VertexArray`** — wraps a VAO, owns/binds its associated vertex +
  index buffers.
- **`Texture2D`** — loads via stb_image (through the resource loader, see
  §14), owns a GL texture handle, exposes `bind(slot)`.
- **`Framebuffer`** — render-to-texture target; used first for the editor
  viewport (rendering the scene into a texture displayed inside an ImGui
  window rather than directly to the swapchain), later reusable for any
  future post-processing.
- **`Mesh`** — vertex/index data + a `VertexArray`; loaded via Assimp
  (§14.3) or generated procedurally (e.g. primitive shapes for
  placeholder/debug rendering).
- **`Material`** — a `Shader` reference + a set of named parameter values
  (colors, texture slots, scalar values) applied to the shader's uniforms
  before a draw call. Materials are simple value structs in v1, not a full
  shader-graph/node system.
- **`Camera`** — base interface producing a view-projection matrix;
  `OrthographicCamera` (useful for pure 2D debug rendering / UI) and
  `PerspectiveCamera` (the primary camera type for the 2.5D gameplay view,
  see §20) both implement it.
- **`Light`** — simple struct: type (directional/point), color, intensity,
  position/direction, radius (for point lights). No shadow casting in v1
  (see Non-Goals).

### 10.3 Render Pipeline (v1 scope)

A single forward-rendering pass per frame:

1. Bind the target framebuffer (the default swapchain for `game`, an
   off-screen framebuffer for `editor`'s viewport panel).
2. Clear color + depth buffers.
3. For each entity with `TransformComponent` + `MeshRendererComponent`
   (see §11.2): bind its material's shader, upload the active camera's
   view-projection matrix, the entity's model matrix, and active light
   data (a small fixed-size array of lights uploaded as uniforms — no
   deferred/tiled lighting needed at this scale), then issue the draw call.
4. Depth testing is on; simple alpha blending is available per-material
   for transparent objects (sorted back-to-front only if actually needed —
   deferred until the demo game has transparent geometry).
5. (Editor only) after the scene pass, an optional debug-draw pass renders
   gizmos, collision shape wireframes, and grid lines directly into the
   same framebuffer.

### 10.4 Shading Model

- Blinn-Phong lighting (ambient + diffuse + specular) in GLSL, computed
  per-fragment. This is a deliberate simplification over full PBR — it's
  visually adequate for a stylized top-down action game and vastly
  simpler to implement and debug than a metallic/roughness PBR pipeline.
- One directional "sun" light plus a small number (e.g. up to 8) of point
  lights uploaded as a uniform array; the demo game's single arena-scale
  level does not need more than that.

### 10.5 Batching / Renderer2D (optional, post-core)

Once the core 3D mesh pipeline works, a `Renderer2D` batched quad renderer
may be added for UI elements, sprites/decals, and debug overlays that don't
warrant full mesh draw calls each — this is a nice-to-have inside Milestone
2/3 scope, not a hard blocker for the demo game.

---

## 11. ECS Subsystem

### 11.1 Design Approach

entt's `entt::registry` is the single source of truth for all scene state.
There is no parallel "GameObject" wrapper type — code that needs to work
with an entity either holds an `entt::entity` handle directly, or uses the
engine's thin `Entity` convenience wrapper (a `{entt::entity, Scene*}` pair
with templated `getComponent<T>()`/`addComponent<T>()`/`hasComponent<T>()`
helpers) purely for ergonomics — it adds zero indirection at runtime, it's
inlined convenience over the raw registry API.

### 11.2 Core Components (v1)

| Component | Fields | Purpose |
|-----------|--------|---------|
| `TagComponent` | `std::string name` | Human-readable entity name, shown in the editor hierarchy |
| `TransformComponent` | `vec3 position, vec3 rotation (euler), vec3 scale` | World-space transform; `getTransform()` produces the model matrix |
| `HierarchyComponent` | `entt::entity parent`, `std::vector<entt::entity> children` | Parent/child relationships for the scene graph |
| `MeshRendererComponent` | `AssetHandle<Mesh> mesh`, `AssetHandle<Material> material` | What to draw |
| `CameraComponent` | `PerspectiveCamera camera`, `bool primary` | Marks the active scene camera |
| `LightComponent` | `LightType type, vec3 color, float intensity, float radius` | Scene lighting |
| `RigidBodyComponent` | `BodyType type (static/kinematic/dynamic)`, Jolt body handle | Physics body binding, see §12 |
| `ColliderComponent` | shape data (box/capsule/sphere) + material (friction/restitution) | Collision shape, paired with `RigidBodyComponent` |
| `CharacterControllerComponent` | movement speed, Jolt character handle | Top-down player/NPC movement |
| `ScriptComponent` | `std::string scriptPath`, sol2 table instance | Attaches Lua behavior to an entity, see §13 |
| `AudioSourceComponent` | `AssetHandle<AudioClip> clip`, volume, loop, 3D flags | Sound emission, see §16 |
| `NativeScriptComponent` (debug/bootstrap only) | function pointers | Escape hatch for engine-internal test behaviors; not used by actual gameplay once Lua scripting lands |

### 11.3 Systems (v1)

Systems are plain functions/classes operating on `entt::registry` views,
invoked in a fixed order each frame from `Scene::onUpdate(dt)`:

1. **ScriptSystem** — runs each `ScriptComponent`'s `on_update(dt)` Lua
   callback.
2. **PhysicsSyncSystem** — steps the Jolt physics world (fixed timestep,
   see §27), then writes resulting body transforms back into
   `TransformComponent` for rendering.
3. **CameraSystem** — resolves the active `primary` camera's
   view-projection matrix for the frame.
4. **RenderSystem** — iterates `TransformComponent + MeshRendererComponent`
   views and issues draw calls via the Renderer (§10).
5. **AudioSystem** — updates 3D audio source positions/listener position
   each frame.

### 11.4 Entity Lifecycle

- `Scene::createEntity(name)` creates a new `entt::entity` with a default
  `TagComponent` + `TransformComponent` + `HierarchyComponent`.
- `Scene::destroyEntity(entity)` removes it from the registry; hierarchy
  cleanup (reparenting or cascading-destroying children) is handled
  explicitly rather than relying on entt destruction hooks, to keep the
  behavior predictable and editor-visible (the editor asks "delete
  children too?" when relevant).
- Prefabs (§15.3) are just serialized entity + component blobs that get
  re-instantiated into a registry — no separate "prefab asset" runtime
  type distinct from a mini scene fragment.

---

## 12. Physics Subsystem

### 12.1 Jolt Integration Shape

- A single `PhysicsWorld` class owns Jolt's `JPH::PhysicsSystem`, job
  system, and temp allocator, initialized once at `Application` startup.
- Fixed timestep stepping (§27) — Jolt is stepped in fixed increments
  (e.g. 1/60s) accumulated from variable frame `deltaTime`, decoupling
  simulation stability from rendering frame rate.
- Collision layers (`CollisionLayers.h`): `STATIC`, `DYNAMIC`, `PLAYER`,
  `ENEMY`, `TRIGGER` — a small fixed layer set with an explicit
  layer-vs-layer collision matrix, rather than Jolt's more elaborate
  broad-phase layer configuration options (not needed at this scale).

### 12.2 Rigid Bodies

- `RigidBodyComponent` + `ColliderComponent` together describe a Jolt body;
  `PhysicsSyncSystem` creates the underlying `JPH::Body` when both
  components are present on an entity (lazily, on first physics step after
  the components are added) and destroys it when either is removed.
- Supported shapes for v1: box, sphere, capsule — sufficient for the demo
  game's characters (capsules) and simple level geometry (boxes), plus
  static mesh colliders for level geometry loaded via Assimp where a
  primitive shape isn't a good enough approximation.

### 12.3 Top-Down Character Controller

- Built on Jolt's `CharacterVirtual` (kinematic character controller, not
  a dynamic rigid body) — appropriate for responsive top-down movement
  where "floaty" dynamic-body movement feels wrong.
- No gravity-driven jumping needed for the 2.5D top-down genre; the
  controller resolves horizontal movement + collision sliding against
  level geometry each fixed step, with a very light constant downward
  bias (or none at all, depending on how the level geometry is authored)
  just to keep the character grounded against floor colliders.
- `CharacterControllerComponent` exposes `move(vec2 direction, float
  speed)`, called from either C++ input handling (early bootstrap/testing)
  or from Lua scripts (the actual gameplay path) each frame.

### 12.4 Triggers

- Trigger volumes are `ColliderComponent`s flagged `isTrigger = true` on
  the `TRIGGER` collision layer — they report overlap begin/end events via
  a Jolt contact listener, forwarded into the `ScriptSystem` as
  `on_trigger_enter(other_entity)` / `on_trigger_exit(other_entity)`
  callbacks on any `ScriptComponent` attached to either participant.
  Used for things like "enemy detects player" or "player enters level exit
  zone."

---

## 13. Scripting Subsystem

### 13.1 Lua VM Lifecycle

- One `sol::state` per running `Scene` (not one global VM for the whole
  application) — this makes "stop play mode, reset scene" in the editor a
  clean matter of tearing down and recreating the Lua state, with no
  leftover global script state bleeding between play sessions.
- `ScriptEngine::init(Scene*)` creates the state, opens the standard
  libraries needed (`base`, `math`, `table`, `string` — deliberately
  *not* `os`/`io`, to keep scripts sandboxed away from filesystem/process
  access), and registers all engine bindings (§13.3).

### 13.2 ScriptComponent Lifecycle

Each entity with a `ScriptComponent` gets a fresh Lua table (its "script
instance") built by loading and running its `scriptPath` file, which
returns a table of lifecycle functions:

```lua
-- assets/scripts/enemy_basic.lua
local Enemy = {}

function Enemy.on_start(entity)
    entity.speed = 2.5
end

function Enemy.on_update(entity, dt)
    local player = molly.find_entity("Player")
    if player then
        molly.move_towards(entity, player.transform.position, entity.speed, dt)
    end
end

function Enemy.on_trigger_enter(entity, other)
    if other.tag == "PlayerAttack" then
        molly.destroy_entity(entity)
    end
end

return Enemy
```

- `on_start(entity)` — called once when the entity's script becomes active
  (scene load / play mode start).
- `on_update(entity, dt)` — called every frame from `ScriptSystem`.
- `on_trigger_enter/exit(entity, other)` — called from the physics trigger
  callback path (§12.4).
- `on_destroy(entity)` — called just before the entity is removed from the
  scene, for cleanup.

### 13.3 C++ ↔ Lua Bindings (sol2)

Bound under a single global `molly` Lua table (not scattered globals) to
keep the scripting API namespaced and discoverable:

- `molly.find_entity(name) -> entity handle`
- `molly.destroy_entity(entity)`
- `molly.spawn_entity(prefab_path, position) -> entity handle`
- `entity.transform.position / .rotation / .scale` (readable/writable
  `vec3` proxies)
- `molly.move_towards(entity, target_pos, speed, dt)`
- `molly.input.is_key_down(key_name) -> bool`
- `molly.input.is_mouse_down(button) -> bool`
- `molly.input.mouse_world_position() -> vec3` (useful for mouse-aim)
- `molly.play_sound(entity, clip_path)`
- `molly.log(message)` — routes into the engine's spdlog `"APP"` logger, so
  script `print`-debugging shows up in the same Console panel as engine
  logs.

Full binding surface documented incrementally in §36 (Appendix B) as it
solidifies during Milestone 5 implementation — the list above is the
initial planned surface, expected to grow as the demo game's needs surface
gaps.

### 13.4 Hot Reload

- The `ResourceManager`'s file-watching (§14.4) detects changes to `.lua`
  files under `assets/scripts/` and triggers `ScriptEngine` to re-run the
  changed script's file and rebind the affected entities' script tables,
  preserving already-set entity-local state where field names match. This
  is a "nice to have, implement if time allows within Milestone 5/6" item,
  not a hard requirement — script iteration is still fast even with a
  manual play-mode restart if hot reload proves too fiddly to get right
  quickly.

### 13.5 Sandboxing / Safety

- No `os`/`io`/`require`/`dofile`/`loadfile` exposed to scripts — scripts
  can only do what the `molly` API surface explicitly allows. This isn't a
  security boundary against malicious actors (it's a single-player local
  game), it's a *correctness* boundary: it stops scripts from doing
  filesystem/process things that would make behavior non-deterministic or
  break editor hot-reload assumptions.
- Errors thrown inside a Lua callback are caught at the `sol2` call site,
  logged via `MOLLY_ERROR` with the script path + Lua error message, and
  do **not** crash the engine or halt other entities' scripts — one broken
  script should degrade gracefully, not take down the whole play session.

---

## 14. Resource / Asset Management

### 14.1 Asset Types (v1)

Textures, models (meshes), shaders, Lua scripts, audio clips, scenes,
prefabs, materials.

### 14.2 AssetHandle & Identity

- Assets are identified by their project-relative file path (e.g.
  `"models/player.gltf"`), not a generated GUID system — simpler to reason
  about and debug for a single-project engine, and human-readable inside
  serialized scene JSON (see §37). A GUID-based system is listed as a
  possible post-v1 upgrade if asset renaming/moving becomes painful, but
  is explicitly not built for v1.
- `AssetHandle<T>` is a thin wrapper around a path + a cached weak
  reference into the `ResourceManager`'s cache, resolved lazily on first
  access rather than eagerly holding a raw pointer, so a scene can
  reference assets that get hot-reloaded without the handle going stale.

### 14.3 Loading Pipeline

- `ResourceManager::load<T>(path)` checks its type-specific cache first (by
  path); on a miss, dispatches to the matching `AssetLoaders/*Loader`
  (`TextureLoader` → stb_image, `ModelLoader` → Assimp, `ShaderLoader` →
  raw GLSL source read + `Shader` compile, `ScriptLoader` → raw Lua source
  read), stores the result as a `std::shared_ptr<T>` in the cache, and
  returns it.
- All loading in v1 is **synchronous**, on the main thread, at the point of
  first use (scene load time, mostly). Asynchronous/streamed loading is
  explicitly deferred — the target content scale (one small demo level)
  doesn't need it, and async loading adds meaningful complexity (thread
  safety around the registry/renderer) that isn't worth paying for yet.

### 14.4 Hot Reload

- A lightweight file-watcher (polling file modification timestamps once
  per second — no need for OS-level file-change-notification APIs at this
  scale) checks watched asset paths and, on change, triggers the matching
  loader to reload in place: shaders recompile and swap, textures
  re-upload, scripts re-run (§13.4).
- Hot reload is an editor-mode-only convenience; it is disabled in the
  standalone `molly-demo` game build (`game` build config leaves the
  watcher thread out entirely) since a shipped game doesn't need to watch
  its own asset files for changes.

### 14.5 Asset Reference in Scenes

Scene/prefab JSON files reference assets by their project-relative path
string directly in component data (e.g. a `MeshRendererComponent`'s
`"mesh"` field is literally `"models/player.gltf"`) — no indirection
through an asset database/registry file. Simpler for a project this size,
and it means asset references are trivially greppable/diffable in version
control.

---

## 15. Scene System

### 15.1 Scene Graph

- `HierarchyComponent` (§11.2) provides parent/child relationships;
  `TransformComponent` stores **local** transforms, and world transforms
  are resolved by walking the hierarchy each frame (or cached and
  invalidated on change — a dirty-flag optimization considered but not
  required until profiling shows it matters).
- The editor's Scene Hierarchy panel (§18.3) is a direct visual
  representation of this parent/child tree, supporting drag-and-drop
  reparenting.

### 15.2 Scene Serialization

- `SceneSerializer::serialize(scene, path)` / `::deserialize(path) -> Scene`
  read/write the scene as JSON via nlohmann/json — see §37 for the exact
  format.
- Every registered component type has a serialize/deserialize function
  pair registered against it (a small internal registry mapping component
  type → JSON (de)serialization functions), so adding a new component type
  requires writing one serialization function pair, not touching a giant
  switch statement elsewhere.

### 15.3 Prefabs

- A prefab is just a single serialized entity (plus its full subtree of
  children) stored as its own `.prefab.json` file, using the exact same
  component serialization machinery as full scenes.
- `Scene::instantiatePrefab(path, position) -> Entity` loads and
  deserializes the prefab file into a new set of entities in the target
  scene, then applies the given spawn position override. This is the
  mechanism the demo game uses for spawning enemies from Lua
  (`molly.spawn_entity`, §13.3).
- No prefab "instance override tracking" (a la Unity's prefab variant/
  override system) in v1 — instantiating a prefab produces a fully
  independent copy; editing the original prefab file does not retroactively
  update already-placed instances. This is a known, accepted limitation
  for v1 scope.

---

## 16. Audio Subsystem

### 16.1 Engine Wrapper

- `AudioEngine` wraps a single `ma_engine` (miniaudio's high-level engine
  API), initialized once at startup, exposing `playOneShot(path)`,
  `playLooped(path) -> handle`, `stop(handle)`, and global volume control.

### 16.2 AudioSourceComponent

- Entities with `AudioSourceComponent` (§11.2) get their sound's position
  updated each frame by the `AudioSystem` (§11.3) from their
  `TransformComponent`, enabling basic positional/3D attenuation via
  miniaudio's built-in spatialization — sufficient for "footsteps/impacts
  come from roughly the right place," not aiming for full HRTF-quality 3D
  audio.
- A single `AudioListener` marker component identifies which entity (the
  player, effectively) miniaudio treats as the listener position for
  spatialization.

### 16.3 Scope for v1

SFX (attack hits, footsteps, UI clicks) and background music (looped,
non-positional). No dynamic mixing graphs, DSP effects chains, or audio
occlusion — straightforward playback is enough for the demo game.

---

## 17. UI System

### 17.1 Two Distinct UI Surfaces

It's important to keep these conceptually separate even though both are
built on the same ImGui library underneath:

1. **Editor UI** — the tool's own interface (panels, menus, gizmos). Not
   shipped with the game; exists only in `molly-editor`.
2. **In-game UI** — what a player sees (health bar, wave counter, menus)
   inside `molly-demo`. Built with plain ImGui calls for v1 (see §2.2 —
   a proper data-driven HUD system is a non-goal for v1).

### 17.2 ImGuiLayer

- A shared `ImGuiLayer` (in `engine/include/molly/ui/`) handles ImGui
  context creation, GLFW/OpenGL3 backend init, style setup, and per-frame
  `NewFrame`/`Render` bracketing — used by both `editor` and `game`, so
  neither reimplements ImGui bootstrapping.
- Docking + viewports enabled (`ImGuiConfigFlags_DockingEnable`) — required
  for the editor's dockable panel layout (§18); `molly-demo` doesn't use
  docking itself but shares the same ImGui context setup for simplicity.

### 17.3 In-Game UI (v1 scope)

Simple, direct ImGui calls inside `GameLayer::onImGuiRender()`: a health
bar (progress bar widget), current wave number/enemy count text, and a
game-over/restart screen. Styled minimally (custom colors, no ImGui
default "engineering tool" look bleeding into the player-facing game) —
enough visual polish to not look like a debug overlay, without building a
general theming system.

---

## 18. Editor Application

### 18.1 Visual Design (starting direction — yours to redirect)

- Dark theme by default, ImGui's native dark style lightly customized
  (accent color TBD once there's a real UI to look at and react to) rather
  than a from-scratch reskin.
- Flat, low-chrome panels; no gradients/skeuomorphism.
- This is explicitly flagged as a placeholder — once the editor shell
  (Milestone 7) exists and is runnable, revisit this with actual visual
  taste input rather than deciding it in the abstract now.

### 18.2 Layout

Standard four-quadrant docked layout, matching the conventions of
Unity/Unreal/Godot editors (familiar layout reduces the learning curve of
the tool itself):

```
┌──────────────┬───────────────────────────┬──────────────┐
│              │                                │              │
│  Hierarchy    │           Viewport              │  Inspector    │
│   Panel       │      (render-to-texture)         │    Panel      │
│              │                                │              │
├──────────────┴───────────────────────────┴──────────────┤
│         Content Browser          │           Console        │
└───────────────────────────────────┴──────────────────────┘
```

- Default layout persisted via ImGui's `.ini` docking layout file
  (`editor/assets/default_layout.ini`), checked in so a fresh clone opens
  the editor with a sensible layout instead of a blank/undocked mess.

### 18.3 Panels

- **Scene Hierarchy** — tree view of the active scene's entities
  (respecting `HierarchyComponent` parent/child structure); click to
  select (drives Inspector), drag-drop to reparent, right-click context
  menu for create/delete/duplicate.
- **Inspector** — shows all components on the currently selected entity,
  each with an editable widget block (drag-float for `TransformComponent`,
  color picker for `LightComponent`, asset-path picker for
  `MeshRendererComponent`'s mesh/material, a text field + "open in editor"
  button for `ScriptComponent`'s script path); an "Add Component" dropdown
  at the bottom.
- **Viewport** — the scene rendered into a `Framebuffer` (§10.2), displayed
  via `ImGui::Image`; supports a gizmo overlay (translate/rotate/scale —
  likely via the vendored-in ImGuizmo helper library, evaluated during
  Milestone 7 as an additional small vendored dependency if it clearly
  saves time over hand-rolling gizmo math) and camera fly/pan controls
  scoped to editor (non-play) mode only.
- **Content Browser** — a simple file-tree view rooted at the active
  project's `assets/` directory; double-click to open (`.scene.json` opens
  it in the Hierarchy/Viewport, `.lua` opens in the system default text
  editor or VS Code via a shell-out, image files show a thumbnail preview).
- **Console** — the spdlog-fed log panel (§8.4), with severity filtering
  (Trace/Info/Warn/Error toggle buttons) and a search/filter text box.
- **Toolbar** — Play / Pause / Stop buttons controlling scene simulation
  state (§18.4).

### 18.4 Play Mode

- **Edit mode** (default): scene is static, editable, physics/scripts not
  running; editor camera controls the viewport freely.
- **Play mode**: the current scene state is deep-copied into a runtime
  scene instance (so edit-mode data is untouched), physics + scripting +
  gameplay camera all become active, exactly mirroring how `molly-demo`
  runs the scene standalone.
- **Stop**: discards the runtime scene instance, returns to displaying the
  untouched edit-mode scene — i.e., editing during play mode never
  persists (matches the standard Unity-style "changes made in play mode
  don't stick" expectation, which avoids a whole class of "why did my
  scene file change" confusion).
- **Pause**: freezes the runtime scene's `onUpdate` calls (physics/scripts)
  while still rendering the current frame, useful for frame-by-frame
  debugging.

### 18.5 Undo/Redo

- A simple command-pattern undo stack (`ICommand` with `execute()`/
  `undo()`), used for Inspector field edits and Hierarchy operations
  (create/delete/reparent entity). Scoped to edit-mode only — play-mode
  changes are discarded on Stop anyway (§18.4), so they don't need undo
  support. This is a "build once the editor shell is stable" item
  (Milestone 7 stretch, acceptable to slip into a follow-up pass if time
  is tight).

---

## 19. Input System

### 19.1 Raw Input

- `Input::isKeyDown(KeyCode)`, `Input::isMouseButtonDown(MouseCode)`,
  `Input::getMousePosition()` — thin static wrappers over GLFW polling
  functions, engine-wide, always available (not scene/entity-scoped).

### 19.2 Action Mapping (post-bootstrap)

- Once raw input works (Milestone 1–2), an `InputActionMap` layer is added:
  named actions (`"MoveUp"`, `"Attack"`, `"Interact"`) bound to one or more
  physical inputs, defined in a small JSON config
  (`game/assets/input_bindings.json`), queried from Lua as
  `molly.input.is_action_down("Attack")` rather than raw key codes — this
  keeps gameplay scripts decoupled from specific keybindings and makes
  future rebinding-UI work straightforward.
- Default demo game bindings: WASD (move), Left Mouse (primary attack),
  Right Mouse or Space (secondary/ranged attack), Escape (pause menu).

### 19.3 Gamepad (stretch goal)

GLFW's joystick/gamepad API (`glfwGetGamepadState`) is the planned
integration point if/when gamepad support is picked up — not required for
the v1 demo game, listed here so the `InputActionMap` design (binding
*actions* rather than hardcoding devices) already accounts for it without
rework later.

---

## 20. Camera System

### 20.1 Gameplay Camera

- A `PerspectiveCamera` fixed at a top-down/angled orientation (something
  in the 45–60° pitch range, tunable during Milestone 2 once there's
  actual geometry to look at and judge feel against) framing the play
  area, following the player entity's XZ position with a smoothed
  (lerped) follow rather than a hard-locked snap.
- Zoom/FOV is fixed per-level for v1 (no dynamic zoom-on-action system);
  revisit only if the demo game's feel clearly needs it.

### 20.2 Editor Camera

- A separate, independent free-fly/orbit camera used only in the Viewport
  panel during edit mode (§18.3) — pan (middle-mouse-drag), orbit
  (alt+left-drag or right-drag, matching common DCC-tool conventions),
  zoom (scroll wheel). Entirely decoupled from the gameplay
  `PerspectiveCamera` — switching to Play mode swaps the viewport to render
  through the scene's actual gameplay camera instead.

### 20.3 Coordinate Convention

Y-up, right-handed coordinate system (matching GLM's/OpenGL's conventional
default) — gameplay movement operates on the X/Z plane, Y is "up" (used for
things like light height, camera pitch, and any future jump/verticality
even though the v1 character controller doesn't use it for movement).

---

## 21. Math & Coordinate Conventions

- All engine math types are GLM types directly (`glm::vec3`, `glm::mat4`,
  `glm::quat`) — no custom math wrapper layer. Reduces boilerplate and
  avoids reinventing well-tested vector/matrix code.
- Rotations stored as Euler angles (degrees, for human-readability in the
  Inspector/serialized JSON) in `TransformComponent`, converted to
  quaternions internally when composing matrices, to sidestep gimbal-lock
  surprises in the actual transform math while keeping the *authored*
  representation simple for a top-down game where most entities only ever
  rotate around the Y axis anyway.
- World units are meters, at roughly human/game-object scale (a
  player capsule is ~1.8 units tall) — chosen so Jolt's default physics
  tuning (gravity, mass, velocities) behaves sensibly without unusual
  unit-scale correction factors.
- Screen-space/UI coordinates (ImGui) remain in ImGui's native pixel
  space; no attempt to unify that with world-space units.

---

## 22. Testing Strategy

### 22.1 What Gets Unit Tested

Pure-logic, deterministic pieces are the highest-value targets for Catch2
unit tests — not everything needs a test, especially not GL-calling or
window-dependent code:

- **Math/utility helpers** — any custom helper functions layered on top of
  GLM (e.g. `moveTowards`, angle-wrapping helpers).
- **ECS component (de)serialization** — round-trip a component through
  `SceneSerializer`'s JSON functions and assert equality; this is high
  value because silent serialization bugs are otherwise only discovered by
  manually noticing a scene didn't reload correctly.
- **ResourceManager caching behavior** — loading the same path twice
  returns the same cached instance; reload invalidates correctly.
- **Physics collision layer matrix logic** — pure logic, no need to spin
  up a real Jolt world for the layer-matrix decision function itself.
- **Lua binding smoke tests** — a minimal `sol::state` loading a trivial
  test script and asserting a bound function was called with expected
  arguments, catching binding signature regressions early.

### 22.2 What Does Not Get Unit Tested

Rendering output correctness (no automated screenshot/pixel-diff testing
in v1 — out of scope for the project's size), full physics simulation
behavior (validated by hand via the demo game, not automated), and editor
UI interaction flows (manual testing only).

### 22.3 Manual Test Checklist

A living checklist (`tests/MANUAL_CHECKLIST.md`, created once Milestone 7
lands) covering editor workflows that are impractical to automate: create/
delete/reparent entity via Hierarchy, edit each component type via
Inspector, Play/Pause/Stop round-trip leaves edit-mode scene untouched,
drag-drop asset from Content Browser into Viewport, save/reload a scene
and confirm it matches.

### 22.4 CTest Integration

`tests/CMakeLists.txt` uses `catch_discover_tests(molly-tests)` so every
Catch2 `TEST_CASE` is registered as an individual CTest test, runnable via
`ctest` from the build directory or the CMake Tools "Run Tests" command in
VS Code, and consumed the same way by CI (§23).

---

## 23. CI/CD Pipeline

### 23.1 Scope for v1

A single GitHub Actions workflow (`.github/workflows/build.yml`) that, on
every push and pull request:

1. Checks out the repo **with submodules** (`submodules: recursive`).
2. Configures CMake using the `windows-release` preset (fast enough for CI,
   still catches real bugs, unlike a pure Debug build's different codegen
   characteristics).
3. Builds all targets (`engine`, `editor`, `game`, `tests`).
4. Runs `ctest` and fails the workflow on any test failure.

No artifact publishing, no release automation, no deployment step in v1 —
purely a build+test correctness gate. This is explicitly the *only* CI
scope for now; anything more (matrix builds, nightly builds, automated
releases) is deferred until there's an actual release to make.

### 23.2 Why Windows-Only CI

Matches the project's Windows-only support scope (§1.2) — running a Linux
CI job for a project that isn't targeting Linux would just be maintenance
overhead validating a configuration nobody uses.

### 23.3 Build Time Considerations

Vendored dependencies (especially Assimp and Jolt) are non-trivial to
compile from scratch. GitHub Actions' cache action
(`actions/cache`) is used keyed on the vendored submodule commit SHAs plus
CMake config, caching the CMake build directory's dependency object files
between CI runs so a docs-only or gameplay-script-only change doesn't
rebuild the entire vendor tree every time.

---

## 24. Performance Budgets & Profiling

### 24.1 v1 Targets

Not aiming for AAA-scale numbers — targets are set relative to the actual
demo game's content scale (one small arena, a handful of enemies):

- 60 FPS sustained on a mid-range Windows 11 machine with a discrete or
  reasonably modern integrated GPU, at the demo game's actual content
  scale (dozens, not thousands, of on-screen entities).
- Scene load (level start) under ~2 seconds given the demo game's small
  asset set.
- No specific micro-benchmarks required until profiling (§24.2) actually
  identifies a bottleneck — premature optimization is explicitly
  discouraged at this project scale; correctness and working features come
  first.

### 24.2 Profiling Approach

- Early milestones: simple frame-time logging (rolling average FPS printed
  to the Console panel) is sufficient to notice regressions by eye.
- Post-v1 (or earlier if a real performance problem surfaces): integrate
  Tracy for proper frame-time/GPU-timing visualization — deliberately not
  pulled in on day one to avoid adding vendoring/build complexity before
  it's actually needed.

---

## 25. Error Handling & Logging Philosophy

- **Programmer errors** (violated invariants, "this should never happen"
  states): `MOLLY_ASSERT(condition, message)`, compiled to a no-op in
  Release builds, triggers a debugger breakpoint (`__debugbreak()`) in
  Debug builds.
- **Expected, recoverable failures** (file not found, malformed JSON,
  shader compile error): logged via `MOLLY_ERROR`/`MOLLY_WARN` and handled
  gracefully — e.g. a missing texture falls back to a bright "missing
  texture" checkerboard placeholder rather than crashing or leaving
  undefined GL state.
- **Unrecoverable startup failures** (no OpenGL 4.6 context available,
  vendored asset directory missing): logged via `MOLLY_CRITICAL` and the
  application exits cleanly with a non-zero status and, where feasible, a
  Win32 message box (since a user might not have a console attached).
- **No exceptions in engine code** (§7.4) — the one boundary where
  exceptions exist is internal to `sol2`'s Lua error propagation, caught
  and converted to logged errors at the script-call boundary (§13.5),
  never allowed to unwind into engine/ECS code.

---

## 26. Memory Management Strategy

- Standard heap allocation (`new`/`std::make_unique`/STL containers) is the
  default throughout — this project is not chasing custom
  allocator/arena-based memory management in v1; that's a performance-tier
  concern that isn't justified at this content scale (§24.1).
- entt's registry already provides cache-friendly, pooled storage for
  component data internally — that's where the "data-oriented memory
  layout" benefit actually comes from, without the engine needing to hand-
  roll its own arena allocators on top.
- The `ResourceManager`'s `shared_ptr`-based asset cache (§14.3) is the one
  place shared ownership is deliberately used; everything else follows the
  unique-ownership-by-default rule from §7.5.
- Jolt and Assimp manage their own internal allocations; the engine does
  not attempt to route their allocations through a shared custom allocator
  in v1 (Jolt does support allocator injection — revisit only if profiling
  shows it matters).

---

## 27. Threading Model

### 27.1 v1: Effectively Single-Threaded

- The main thread owns the window, GL context, ECS registry, physics
  world, and Lua VM — all game logic, rendering, and physics stepping
  happen on this one thread each frame, in the fixed order given in
  §11.3.
- **Fixed timestep physics, variable timestep everything else**: an
  accumulator pattern in `Application::run()` steps `PhysicsWorld` in
  fixed 1/60s increments (possibly multiple steps per frame if the frame
  took longer than 1/60s, capped to avoid runaway catch-up — see the
  deltaTime clamp in §8.5) while rendering and script `on_update` calls run
  once per actual frame with the real `deltaTime`.
- Jolt's own internal job system *is* multithreaded (it parallelizes
  physics solving internally across worker threads it manages) — this is
  the one place multithreading exists in v1, and it's entirely encapsulated
  inside `PhysicsWorld`, invisible to the rest of the engine.
- The `ResourceManager`'s file-watcher (§14.4) runs on its own lightweight
  background thread (just polling file timestamps), but actual
  reload/GL-resource-recreation work is marshaled back onto the main
  thread — no GL calls ever happen off the main thread.

### 27.2 Deferred

Job-system-based parallel gameplay code, multithreaded rendering command
recording, and async asset streaming are all explicitly out of scope for
v1 (§2.2) — revisit only if the demo game's actual performance
requirements demand it, which is unlikely at this content scale.

---

## 28. Versioning & Release Process

- Semantic-ish versioning for the engine itself: `MAJOR.MINOR.PATCH`,
  starting at `0.1.0` for the Milestone-1 bootstrap and incrementing MINOR
  at each completed milestone that adds a major subsystem; `0.x` signals
  "pre-stable, API can still shift" throughout v1 development.
- Version string lives in one place (`engine/include/molly/Version.h`,
  generated or hand-maintained) and is surfaced in the editor's title bar
  and a Console startup log line.
- No formal release/tagging process until the demo game (Milestone 10) is
  complete — until then, `main` branch HEAD *is* the current state, and
  there's no need for release branches or changelogs at this project
  stage. Revisit this section once there's a first "1.0 demo" worth
  tagging.

---

## 29. Git Workflow & Branching Strategy

- **`main`** is always kept in a buildable state (enforced by CI, §23) —
  no direct force-pushes, no committing known-broken states intentionally.
- Given this is a solo hobby project, a lightweight workflow is
  appropriate: feature work happens on short-lived branches
  (`feature/renderer-core`, `feature/editor-hierarchy-panel`, etc.) merged
  back into `main` once a milestone sub-task is complete and builds/tests
  pass; trivial fixes can go straight to `main`.
- Commit messages follow a conventional, imperative style ("add mesh
  loading via Assimp", not "added" or "adds") — no strict Conventional
  Commits (`feat:`/`fix:` prefixes) requirement, since there's no
  automated changelog generation depending on it at this stage.
- Submodule bumps always get their own dedicated commit (§6.4), never
  bundled with unrelated feature work, so `git log -- vendor/` stays a
  clean record of dependency history.
- `.gitattributes` configures Git LFS tracking for binary asset types
  (`*.png *.jpg *.gltf *.glb *.fbx *.wav *.mp3 filter=lfs diff=lfs
  merge=lfs -text`) — set up before real game content is added in
  Milestone 10, not needed for the bootstrap milestones which have no
  real binary assets yet.

---

## 30. Documentation Strategy

- **This document (`PLAN.md`)** is the living architectural/planning
  reference — updated as decisions are made or revised, not treated as a
  frozen spec written once and ignored.
- **`README.md`** stays short: what the project is, quickstart build
  instructions (clone, submodule init, CMake configure/build/run), and a
  link back to this plan for anyone wanting the full picture.
- **Header-level documentation**: one-line `///` summaries on public API
  surfaces where names don't already make purpose obvious (§7.6) — no
  separate generated API-doc site (Doxygen HTML output, etc.) planned for
  v1; the headers themselves are the reference.
- **No separate design-doc sprawl** — the demo game's design lives in §31
  of this document, not scattered across additional files, so there's one
  place to look for "what is this project and why."

---

## 31. Demo Game Design Document

### 31.1 Working Title

*(untitled — "Molly Demo" internally; a real name isn't needed for a tech
demo, but nothing stops picking one later if it starts feeling like a real
little game worth naming.)*

### 31.2 Premise

A single-arena, top-down 2.5D action game: the player fights escalating
waves of enemies in one contained level until they die or clear a fixed
number of waves. Minimal narrative framing — a wave-survival arena game,
not a story-driven adventure. This keeps content scope small while still
exercising combat, enemy AI, physics, and UI end-to-end.

### 31.3 Core Loop

1. Player spawns in the arena.
2. A wave of enemies spawns (enemy count/type mix increases per wave).
3. Player fights until the wave's enemies are all defeated.
4. Brief pause / "Wave Cleared" UI beat, then the next wave begins.
5. Loop continues until either the player dies (Game Over screen, restart
   option) or a fixed final wave count is cleared (Victory screen).

### 31.4 Player Character

- **Movement**: WASD, top-down 8-directional movement via the
  `CharacterControllerComponent` (§12.3), constant speed (no
  sprint/stamina system in v1).
- **Primary attack**: melee swing (left mouse button) — a short-range,
  short-duration hitbox check (via a physics trigger volume spawned
  briefly in front of the player) dealing fixed damage to overlapping
  enemies, on a fixed cooldown.
- **Secondary attack**: ranged projectile (right mouse button or Space) —
  spawns a small projectile prefab traveling toward the mouse-aim world
  position (`molly.input.mouse_world_position()`, §13.3), dealing fixed
  damage on enemy contact, on a longer cooldown than the melee attack.
- **Health**: a fixed max HP value, reduced on enemy contact/attack, with
  a brief invulnerability window after taking damage (prevents
  instant-death from standing in multiple overlapping enemy hitboxes at
  once).
- All of the above (speed, damage values, cooldowns, max HP) live as
  tunable fields at the top of the player's Lua script, not hardcoded
  deep in logic — makes balancing a matter of editing a few numbers, not
  hunting through code.

### 31.5 Enemies

Two enemy archetypes for v1 — enough variety to matter, small enough to
stay in scope:

- **Chaser** — simple melee enemy: on spawn, finds the player entity and
  moves directly toward them each frame (`molly.move_towards`, §13.3);
  deals contact damage to the player on overlap, then briefly backs off
  before re-engaging (prevents it from "sticking" to the player
  uncomfortably).
- **Shooter** — ranged enemy: keeps distance from the player (moves away
  if too close, holds position if within its preferred range), periodically
  fires a slow projectile toward the player's position.

Both are implemented as Lua scripts (`assets/scripts/enemy_chaser.lua`,
`enemy_shooter.lua`) attached via `ScriptComponent` to their respective
prefabs (`assets/prefabs/enemy_chaser.prefab.json`,
`enemy_shooter.prefab.json`), spawned by a wave-management script (not
hand-placed per wave) that reads a simple per-wave spawn table (enemy
type + count) and calls `molly.spawn_entity` at randomized points along
the arena's edge.

### 31.6 Level / Arena Design

- One level: a bounded rectangular-ish arena with simple static level
  geometry (outer walls, a small number of interior obstacles/pillars for
  cover and movement variety) built from primitive box colliders/meshes —
  no need for hand-modeled level art; simple textured boxes are entirely
  sufficient for a tech-demo-scope level.
- Enemy spawn points are placed just outside the visible play area at the
  arena's perimeter, so enemies visibly enter rather than popping in on
  top of the player.

### 31.7 Win / Lose States

- **Lose**: player HP reaches 0 → Game Over screen (ImGui overlay, §17.3)
  showing waves survived, with a "Restart" button that reloads the scene
  from scratch (fresh play-mode-equivalent state, not just resetting HP).
- **Win**: a fixed final wave count (e.g. wave 10) is cleared → Victory
  screen, same restart affordance.
- No save/progression system between sessions — every run starts fresh;
  this matches the "single-session arena survival" scope and avoids
  needing a save-file format for v1.

### 31.8 Art & Audio Direction (minimal, placeholder-friendly)

- **Visual style**: simple, flat-shaded/lightly-textured low-poly
  primitives (boxes, capsules, simple modeled character shapes at most) —
  a stylized "grayscale/flat-color blockout that looks intentional"
  aesthetic rather than attempting realistic or highly detailed art,
  which is both outside this project's art-production capacity and
  unnecessary for proving the engine works.
- **Audio**: a small set of placeholder SFX (attack swing, hit impact,
  enemy defeated, wave start/clear stingers) and one looped background
  music track — free/CC0 asset packs (e.g. Kenney.nl's audio packs) are a
  reasonable source, sourced and dropped into `game/assets/audio/` by you
  when Milestone 9/10 is reached, per your original offer to handle asset
  downloads yourself.
- **UI style**: matches the in-game UI scope from §17.3 — minimal, clean,
  not reusing the editor's dark-tool aesthetic verbatim (the player-facing
  UI should read as "game," not "debug overlay").

### 31.9 Explicitly Out of Scope for the Demo Game

Multiple levels, a meta-progression/unlock system, an inventory or
itemization system, more than two enemy archetypes, boss encounters,
difficulty settings, and any narrative/dialogue system. All of these are
reasonable *next* things to add once the core loop above is proven fun and
functional, but none are required to validate the engine end-to-end.

---

## 32. Milestones — Detailed Task Breakdown

Each milestone below expands the top-level summary from earlier planning
into concrete sub-tasks with rough acceptance criteria. Milestones are
meant to be completed roughly in order, though some overlap (e.g. starting
basic Lua bindings while still polishing the renderer) is fine in practice.

### Milestone 1 — Bootstrap

1.1. Initialize repository structure per §4; add `.gitmodules`,
     `.gitignore`, `.editorconfig`, `.clang-format`, `LICENSE` (MIT).
1.2. Add GLFW, GLM, entt, and ImGui (docking branch) as git submodules
     under `vendor/`.
1.3. Generate GLAD (OpenGL 4.6 core) and check in the generated source.
1.4. Write root `CMakeLists.txt`, `vendor/CMakeLists.txt` wiring the four
     libraries above, and `CMakePresets.json` (`windows-debug`,
     `windows-release`).
1.5. Implement `Window` (GLFW + GLAD context creation), `Log` (spdlog
     init), and a minimal `Application`/`Layer`/`LayerStack`.
1.6. Implement `ImGuiLayer`; render the ImGui demo window over a cleared
     GL viewport.
1.7. Render a single hardcoded triangle via a minimal `Shader`/`Buffer`/
     `VertexArray` to confirm the full GL pipeline path works.
1.8. Stand up `molly-editor` and `molly-demo` as separate executables both
     linking `molly-engine`, both showing the window + triangle + ImGui
     demo window.

**Acceptance**: `windows-debug` preset builds cleanly from a fresh clone
(after submodule init) via VS Code; both `molly-editor` and `molly-demo`
launch, show a window, a triangle, and the ImGui demo window; closing the
window exits cleanly.

### Milestone 2 — Renderer Core

2.1. `Shader` class: compile/link from vertex+fragment source files, typed
     uniform setters with cached locations.
2.2. `BufferLayout`/`VertexBuffer`/`IndexBuffer`/`VertexArray` abstraction
     replacing the hardcoded Milestone-1 triangle setup.
2.3. `Texture2D` + `stb_image` vendoring + `TextureLoader`.
2.4. Add Assimp submodule; `ModelLoader` loading a glTF test model into a
     `Mesh`.
2.5. `PerspectiveCamera` with configurable position/pitch/yaw/FOV; basic
     free-fly debug camera controls for testing.
2.6. `Material` (shader + uniform parameter set) and a Blinn-Phong
     lit shader supporting one directional light + up to 8 point lights.
2.7. `Framebuffer` (render-to-texture), validated by rendering the scene
     into a texture and displaying it via `ImGui::Image` in a test window.

**Acceptance**: a loaded glTF test model renders lit, textured, from a
top-down-angled perspective camera, both directly to the swapchain
(`molly-demo`) and into an ImGui-hosted viewport texture (`molly-editor`
prototype).

### Milestone 3 — ECS Integration

3.1. Add entt-backed `Scene` class wrapping `entt::registry`.
3.2. Implement `Entity` convenience wrapper (`entt::entity` + `Scene*`,
     templated component helpers).
3.3. Implement core components: `TagComponent`, `TransformComponent`,
     `HierarchyComponent`, `MeshRendererComponent`, `CameraComponent`,
     `LightComponent`.
3.4. Implement `RenderSystem` (iterates transform+mesh-renderer views,
     issues draw calls) and `CameraSystem` (resolves active camera).
3.5. Rework `molly-demo`'s test scene to be entity-driven (a floor plane
     entity, a few lit prop entities, one camera entity, one light entity)
     instead of hardcoded Milestone-2 test objects.

**Acceptance**: a scene built entirely from entt entities + components
renders correctly; adding/removing entities and their components at
runtime (via a temporary debug ImGui panel) visibly changes the rendered
scene.

### Milestone 4 — Physics

4.1. Add JoltPhysics submodule; wire up `PhysicsWorld` (system, job
     system, temp allocator) initialization/shutdown.
4.2. Define `CollisionLayers` and the layer-vs-layer collision matrix.
4.3. Implement `RigidBodyComponent` + `ColliderComponent` (box, sphere,
     capsule shapes) and `PhysicsSyncSystem` (create/destroy Jolt bodies
     from components; step the world on a fixed timestep accumulator;
     write results back to `TransformComponent`).
4.4. Implement `CharacterControllerComponent` on Jolt's `CharacterVirtual`,
     with a `move(direction, speed)` API, tested via temporary hardcoded
     WASD input in `molly-demo`.
4.5. Implement trigger volumes + a Jolt contact listener forwarding
     enter/exit events (initially logged, not yet wired to scripting since
     Lua doesn't exist until Milestone 5).

**Acceptance**: a capsule character entity moves via WASD and correctly
collides with/slides along static box-collider level geometry; a trigger
volume logs enter/exit as the character passes through it; physics
stepping is visibly stable (no jitter/tunneling) at both 60 FPS and
artificially throttled low frame rates (tested by temporarily capping
`deltaTime`).

### Milestone 5 — Scripting

5.1. Add Lua + sol2 submodules/vendoring; build `molly-lua` static target.
5.2. Implement `ScriptEngine`: per-`Scene` `sol::state` creation/teardown,
     restricted standard library set (§13.5).
5.3. Implement `ScriptComponent` + `ScriptSystem` (load script file, call
     `on_start`, call `on_update(dt)` each frame).
5.4. Implement the initial `molly` Lua binding surface from §13.3:
     `find_entity`, `destroy_entity`, `spawn_entity`, `entity.transform.*`,
     `move_towards`, `input.is_key_down`, `input.is_mouse_down`,
     `input.mouse_world_position`, `log`.
5.5. Wire physics trigger callbacks (Milestone 4.5) into
     `on_trigger_enter`/`on_trigger_exit` script callbacks.
5.6. Move the Milestone-4 hardcoded WASD character movement out of C++ and
     into a Lua player-control script, proving the full loop end-to-end.
5.7. (Stretch) implement file-watcher-driven script hot reload (§13.4).

**Acceptance**: player movement, previously hardcoded in C++, is entirely
driven by a Lua script using only the bound `molly` API; a script runtime
error (deliberately introduced) logs cleanly to the Console without
crashing the app or halting other entities' scripts.

### Milestone 6 — Resource Manager

6.1. Implement `ResourceManager` with type-specific caches
     (`Texture2D`, `Mesh`, `Shader`, Lua script source) keyed by
     project-relative path.
6.2. Implement `AssetHandle<T>` (lazy-resolving path-based handle).
6.3. Route all existing ad-hoc loading (Milestones 2, 5) through the
     `ResourceManager` instead of direct loader calls scattered in
     `molly-demo`/test code.
6.4. Implement the background file-watcher thread + main-thread-marshaled
     reload for textures and shaders (scripts already covered by 5.7 if
     completed, otherwise folded in here).

**Acceptance**: loading the same texture/mesh/shader path twice returns
the identical cached instance (verified via a unit test, §22.1); editing a
shader file on disk while `molly-editor` is running hot-reloads it within
~1 second without a restart.

### Milestone 7 — Editor Shell

7.1. Implement `EditorLayer` + panel scaffolding (`SceneHierarchyPanel`,
     `InspectorPanel`, `ViewportPanel`, `ContentBrowserPanel`,
     `ConsolePanel`) per §18.3.
7.2. Wire the Viewport panel to the Milestone-2 `Framebuffer` render-to-
     texture path, replacing the temporary test window from 2.7.
7.3. Implement the editor free-fly camera (§20.2), active only in edit
     mode.
7.4. Implement Inspector widgets for every component type that exists so
     far (Transform, MeshRenderer, Camera, Light, RigidBody/Collider,
     CharacterController, Script) plus "Add Component."
7.5. Implement Play/Pause/Stop (§18.4): deep-copy scene into a runtime
     instance on Play, discard on Stop, freeze `onUpdate` on Pause.
7.6. Implement the Console panel's custom spdlog sink + severity filtering.
7.7. Set up default docking layout (`default_layout.ini`) checked in.
7.8. (Stretch) gizmos (translate/rotate/scale) in the Viewport, evaluate
     vendoring ImGuizmo if it clearly beats hand-rolled gizmo math.
7.9. (Stretch) undo/redo command stack for edit-mode Inspector/Hierarchy
     operations (§18.5).
7.10. Revisit editor visual theme (§18.1) now that there's a real UI to
      look at — pick an accent color/typography pass.

**Acceptance**: a scene can be fully authored from inside the editor
(create entities, add/edit components, arrange hierarchy) without hand-
editing JSON or C++ test code; Play/Stop round-trips cleanly with no
edit-mode data mutation; the Console shows live engine/game logs.

### Milestone 8 — Scene Serialization

8.1. Implement `SceneSerializer` JSON (de)serialization for every
     component type that exists at this point (§37 documents the target
     format).
8.2. Implement per-component-type registration so adding future component
     types doesn't require touching a central switch statement.
8.3. Wire Content Browser + editor menu (`File > Save Scene` / `Open
     Scene` / `New Scene`).
8.4. Implement `Scene::instantiatePrefab` + prefab file save-as
     ("Create Prefab From Selection" in the Hierarchy panel's context
     menu).
8.5. Add round-trip unit tests (save → load → compare) for each component
     type (§22.1).

**Acceptance**: a scene authored in the editor saves to
`.scene.json`, closes/reopens (or the editor is fully restarted) and
reloads identically; a prefab created from a selected entity subtree can
be re-instantiated (via the editor and via `molly.spawn_entity` from Lua)
correctly.

### Milestone 9 — Audio

9.1. Vendor miniaudio; implement `AudioEngine` wrapper (init/shutdown,
     `playOneShot`, `playLooped`/`stop`, global volume).
9.2. Implement `AudioSourceComponent` + `AudioListener` marker component +
     `AudioSystem` (position sync each frame).
9.3. Implement `molly.play_sound(entity, clip_path)` Lua binding.
9.4. Add Inspector widget for `AudioSourceComponent`.
9.5. Source/placeholder a small SFX + music set (your task per §31.8) and
     wire background music playback + a couple of gameplay SFX triggers
     (attack swing, hit impact) as a smoke test.

**Acceptance**: background music loops correctly; a positional SFX
audibly pans/attenuates based on emitter-vs-listener distance as the
player moves around the Milestone-3 test scene.

### Milestone 10 — Demo Game

10.1. Build the arena level geometry (§31.6) as an editor-authored scene.
10.2. Implement the player Lua script (movement, melee attack, ranged
      attack, health/damage, invulnerability window) per §31.4.
10.3. Implement `enemy_chaser.lua` and `enemy_shooter.lua` per §31.5,
      plus their prefabs.
10.4. Implement the wave-management script (spawn table, wave
      progression, win condition check).
10.5. Implement Game Over / Victory ImGui screens + restart flow (§31.7,
      §17.3).
10.6. Source/place final placeholder art (simple textured primitives) and
      audio (§31.8, §9.5) into `game/assets/`.
10.7. Full playtest pass: balance tuning (speed/damage/cooldown/HP values
      in the player script, §31.4), fix any physics/scripting/rendering
      bugs surfaced only by real gameplay under real content load.
10.8. Update `README.md` with final build/run instructions and a short
      description of the finished demo.

**Acceptance**: the full core loop (§31.3) is playable start to finish —
spawn, fight escalating waves, die or win, restart — with no crashes,
using only the shipped `molly-demo` executable (not requiring the editor
to be running).

---

## 33. Risk Register

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| Solo-project scope creep (adding "just one more system") derails milestone pacing | High | High | Strict adherence to the Non-Goals list (§2.2); any new idea gets written down as a post-v1 note, not immediately built |
| Jolt Physics integration complexity (job system, allocator setup) takes longer than expected | Medium | Medium | Milestone 4 is scoped narrowly (box/sphere/capsule only, no complex mesh colliders required for the demo's simple arena geometry) |
| ImGui docking-branch + viewport-framebuffer integration has rough edges (docking API changes between versions) | Medium | Low | Pin a known-stable docking-branch commit (§6.4); avoid tracking a floating branch |
| Editor scope (Milestone 7) balloons because "just one more panel feature" is tempting | High | Medium | Gizmos and undo/redo are explicitly marked stretch (§32, Milestone 7.8/7.9) — the editor is "good enough to author the demo game," not a commercial-grade tool |
| Motivation loss from long stretches without a visible result | Medium | High | Every milestone (§32) has an explicit, visible acceptance criterion; none should take so long that there's nothing to look at for weeks |
| Assimp build times slow down iteration | Low | Low | Importer list trimmed to only glTF/OBJ/FBX (§6.3); CI dependency caching (§23.3) |
| Lua sandboxing gaps allow scripts to do something undesirable (e.g. infinite loop hangs the game) | Low | Medium | Not solved in v1 (no Lua execution timeout/instruction-count limiting) — acceptable given scripts are all self-authored, not third-party/untrusted content |
| Windows-only scope becomes limiting if the project ever wants a wider audience | Low (accepted, deliberate) | Low | Explicitly deferred per §1.2/§2.2; portable stdlib choices where convenient keep the door open without spending effort now |

---

## 34. Glossary

- **ECS** — Entity Component System; entities are IDs, components are pure
  data, systems are functions operating over components.
- **entt** — the C++ ECS library used (§11).
- **sol2** — the C++/Lua binding library used (§13).
- **Jolt** — the physics engine used (§12).
- **2.5D** — gameplay logic constrained to a 2D plane, rendered through a
  full 3D pipeline.
- **RHI** — Render Hardware Interface; an abstraction layer over a
  graphics API. Molly's `RendererAPI` is a minimal one (§10.1).
- **Fixed timestep** — simulating physics in constant-size time increments
  regardless of variable frame rate, for stability (§27.1).
- **Prefab** — a reusable, serialized entity (+ subtree) template (§15.3).
- **Layer** (engine sense) — a pushable unit in the `LayerStack` with
  update/render/event hooks (§8.2); not to be confused with physics
  collision layers (§12.1) or rendering layers (not used in v1).
- **Vendoring** — checking third-party dependency source directly into the
  repository (as submodules or plain files) instead of relying on a
  package manager (§6).

---

## 35. Appendix A — Component Reference

Full field-level reference for every v1 component (expands §11.2):

```cpp
struct TagComponent {
    std::string name;
};

struct TransformComponent {
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};   // Euler degrees
    glm::vec3 scale{1.0f};

    glm::mat4 getTransform() const;   // composes T * R * S
};

struct HierarchyComponent {
    entt::entity parent{entt::null};
    std::vector<entt::entity> children;
};

struct MeshRendererComponent {
    AssetHandle<Mesh> mesh;
    AssetHandle<Material> material;
    bool castsShadows = false;   // reserved; unused until shadows exist (post-v1)
};

struct CameraComponent {
    PerspectiveCamera camera;
    bool primary = true;
};

enum class LightType { Directional, Point };

struct LightComponent {
    LightType type = LightType::Point;
    glm::vec3 color{1.0f};
    float intensity = 1.0f;
    float radius = 10.0f;   // point lights only
};

enum class BodyType { Static, Kinematic, Dynamic };

struct RigidBodyComponent {
    BodyType type = BodyType::Static;
    float mass = 1.0f;             // dynamic only
    JPH::BodyID bodyId;             // runtime handle, not serialized
};

enum class ColliderShape { Box, Sphere, Capsule };

struct ColliderComponent {
    ColliderShape shape = ColliderShape::Box;
    glm::vec3 halfExtents{0.5f};    // box
    float radius = 0.5f;             // sphere/capsule
    float height = 1.0f;              // capsule
    float friction = 0.6f;
    float restitution = 0.0f;
    bool isTrigger = false;
};

struct CharacterControllerComponent {
    float speed = 3.0f;
    JPH::Ref<JPH::CharacterVirtual> character;   // runtime handle, not serialized
};

struct ScriptComponent {
    std::string scriptPath;
    sol::table instance;   // runtime, not serialized
};

struct AudioSourceComponent {
    AssetHandle<AudioClip> clip;
    float volume = 1.0f;
    bool loop = false;
    bool is3D = true;
};

struct AudioListenerComponent {
    // marker component, no fields
};
```

---

## 36. Appendix B — Lua API Sketch

Grows incrementally as Milestone 5 (and later, Milestone 10's actual
gameplay needs) surface gaps — this is the initial planned surface:

```lua
-- Entity lookup / lifecycle
molly.find_entity(name: string) -> Entity | nil
molly.spawn_entity(prefab_path: string, position: vec3) -> Entity
molly.destroy_entity(entity: Entity)

-- Transform access (entity.transform is a live proxy, not a copy)
entity.transform.position   -- vec3, readable & writable
entity.transform.rotation   -- vec3 (euler degrees)
entity.transform.scale      -- vec3

-- Movement helper
molly.move_towards(entity: Entity, target: vec3, speed: number, dt: number)

-- Input
molly.input.is_key_down(key: string) -> bool
molly.input.is_mouse_down(button: string) -> bool
molly.input.mouse_world_position() -> vec3
molly.input.is_action_down(action: string) -> bool   -- added once InputActionMap (19.2) lands

-- Audio
molly.play_sound(entity: Entity, clip_path: string)

-- Logging
molly.log(message: string)

-- Entity fields (common)
entity.tag   -- string, read-only accessor for TagComponent.name
entity.id    -- opaque handle, for equality comparison between Entity refs
```

Lifecycle callback contract every script module returns:

```lua
local M = {}
function M.on_start(entity) end
function M.on_update(entity, dt) end
function M.on_trigger_enter(entity, other) end
function M.on_trigger_exit(entity, other) end
function M.on_destroy(entity) end
return M
```

---

## 37. Appendix C — Scene JSON Format

Illustrative example (Milestone 8 target format), one entity with a
transform, mesh renderer, and script:

```json
{
  "molly_version": "0.6.0",
  "scene_name": "Arena_Main",
  "entities": [
    {
      "id": 1,
      "components": {
        "Tag": { "name": "Player" },
        "Transform": {
          "position": [0.0, 0.0, 0.0],
          "rotation": [0.0, 0.0, 0.0],
          "scale": [1.0, 1.0, 1.0]
        },
        "MeshRenderer": {
          "mesh": "models/player.gltf",
          "material": "materials/player_mat.json"
        },
        "RigidBody": { "type": "kinematic" },
        "Collider": {
          "shape": "capsule",
          "radius": 0.4,
          "height": 1.8,
          "isTrigger": false
        },
        "CharacterController": { "speed": 3.5 },
        "Script": { "scriptPath": "scripts/player.lua" }
      }
    },
    {
      "id": 2,
      "components": {
        "Tag": { "name": "MainLight" },
        "Transform": {
          "position": [10.0, 15.0, 10.0],
          "rotation": [-45.0, 45.0, 0.0],
          "scale": [1.0, 1.0, 1.0]
        },
        "Light": {
          "type": "directional",
          "color": [1.0, 0.95, 0.9],
          "intensity": 1.2
        }
      }
    }
  ]
}
```

Notes:

- `molly_version` records the engine version (§28) that wrote the file —
  used to detect and, if ever needed, migrate older scene files.
- Component keys match the C++ component type names directly (`Transform`
  for `TransformComponent`, etc.) with the `Component` suffix dropped for
  readability.
- Asset fields (`mesh`, `material`, `scriptPath`) are project-relative
  path strings (§14.5) — no separate GUID indirection.
- Runtime-only fields (Jolt body handles, sol2 table instances) are never
  serialized — only authoring-time data round-trips through JSON.

---

## 38. Appendix D — Engine Class Sketches

Illustrative (not final) header sketches for a handful of central classes,
to make the architecture in §8–§16 concrete:

```cpp
// engine/include/molly/core/Application.h
#pragma once
#include "molly/core/Window.h"
#include "molly/core/LayerStack.h"

namespace molly {

class Application {
public:
    Application(const std::string& name);
    virtual ~Application();

    void run();
    void close();
    void onEvent(Event& e);

    void pushLayer(Layer* layer);
    void pushOverlay(Layer* overlay);

    Window& getWindow() { return *m_window; }
    static Application& get() { return *s_instance; }

private:
    bool onWindowClose(WindowCloseEvent& e);
    bool onWindowResize(WindowResizeEvent& e);

    std::unique_ptr<Window> m_window;
    ImGuiLayer* m_imguiLayer;
    LayerStack m_layerStack;
    bool m_running = true;
    bool m_minimized = false;
    float m_lastFrameTime = 0.0f;

    static Application* s_instance;
};

}  // namespace molly
```

```cpp
// engine/include/molly/ecs/Scene.h
#pragma once
#include <entt/entt.hpp>

namespace molly {

class Entity;

class Scene {
public:
    Entity createEntity(const std::string& name = "Entity");
    void destroyEntity(Entity entity);

    Entity instantiatePrefab(const std::string& prefabPath,
                              const glm::vec3& position);

    void onUpdate(float dt);          // edit-mode: no physics/scripts
    void onUpdateRuntime(float dt);   // play-mode: full system pipeline
    void onRenderRuntime(Camera& camera);

    entt::registry& getRegistry() { return m_registry; }

private:
    entt::registry m_registry;
    friend class Entity;
    friend class SceneSerializer;
};

}  // namespace molly
```

```cpp
// engine/include/molly/ecs/Entity.h
#pragma once
#include "molly/ecs/Scene.h"

namespace molly {

class Entity {
public:
    Entity() = default;
    Entity(entt::entity handle, Scene* scene) : m_handle(handle), m_scene(scene) {}

    template<typename T, typename... Args>
    T& addComponent(Args&&... args) {
        return m_scene->getRegistry().emplace<T>(m_handle, std::forward<Args>(args)...);
    }

    template<typename T>
    T& getComponent() {
        return m_scene->getRegistry().get<T>(m_handle);
    }

    template<typename T>
    bool hasComponent() const {
        return m_scene->getRegistry().all_of<T>(m_handle);
    }

    template<typename T>
    void removeComponent() {
        m_scene->getRegistry().remove<T>(m_handle);
    }

    operator bool() const { return m_handle != entt::null; }
    operator entt::entity() const { return m_handle; }
    bool operator==(const Entity& other) const {
        return m_handle == other.m_handle && m_scene == other.m_scene;
    }

private:
    entt::entity m_handle{entt::null};
    Scene* m_scene = nullptr;
};

}  // namespace molly
```

```cpp
// engine/include/molly/physics/PhysicsWorld.h
#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>

namespace molly {

class PhysicsWorld {
public:
    void init();
    void shutdown();

    void step(float fixedDt);   // called from the accumulator loop, §27.1

    JPH::PhysicsSystem& getSystem() { return *m_system; }

private:
    std::unique_ptr<JPH::PhysicsSystem> m_system;
    std::unique_ptr<JPH::JobSystemThreadPool> m_jobSystem;
    std::unique_ptr<JPH::TempAllocatorImpl> m_tempAllocator;
};

}  // namespace molly
```

```cpp
// engine/include/molly/scripting/ScriptEngine.h
#pragma once
#include <sol/sol.hpp>

namespace molly {

class Scene;

class ScriptEngine {
public:
    void init(Scene* scene);
    void shutdown();

    void onStart(entt::entity entity, const std::string& scriptPath);
    void onUpdate(entt::entity entity, float dt);
    void onTriggerEnter(entt::entity entity, entt::entity other);
    void onTriggerExit(entt::entity entity, entt::entity other);

private:
    void registerBindings();

    sol::state m_lua;
    Scene* m_scene = nullptr;
};

}  // namespace molly
```

---

## 39. Appendix E — Editor Keybindings

| Action | Binding |
|--------|---------|
| Save Scene | `Ctrl+S` |
| Open Scene | `Ctrl+O` |
| New Scene | `Ctrl+N` |
| Undo | `Ctrl+Z` |
| Redo | `Ctrl+Y` / `Ctrl+Shift+Z` |
| Delete selected entity | `Delete` |
| Duplicate selected entity | `Ctrl+D` |
| Play / Stop | `Ctrl+P` |
| Pause | `Ctrl+Shift+P` |
| Focus selected entity in Viewport | `F` |
| Translate gizmo | `W` |
| Rotate gizmo | `E` |
| Scale gizmo | `R` |
| Editor camera pan | Middle-mouse drag |
| Editor camera orbit | Alt + Left-mouse drag |
| Editor camera zoom | Scroll wheel |

---

## 40. Appendix F — CMake Skeleton

Root `CMakeLists.txt` (illustrative skeleton, not final):

```cmake
cmake_minimum_required(VERSION 3.28)
project(Molly LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(MOLLY_ENABLE_ASAN "Enable AddressSanitizer in Debug builds" OFF)
option(MOLLY_BUILD_TESTS "Build the Catch2 test suite" ON)

include(cmake/CompilerWarnings.cmake)
include(cmake/Sanitizers.cmake)

add_subdirectory(vendor)
add_subdirectory(engine)
add_subdirectory(editor)
add_subdirectory(game)

if(MOLLY_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
```

`vendor/CMakeLists.txt` (illustrative skeleton):

```cmake
# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

# GLAD (generated, checked in)
add_library(glad STATIC glad/src/gl.c)
target_include_directories(glad PUBLIC glad/include)

# GLM
add_subdirectory(glm)

# entt (header-only)
add_library(entt INTERFACE)
target_include_directories(entt INTERFACE entt/single_include)

# Dear ImGui (hand-rolled target, no native CMake)
add_library(molly-imgui STATIC
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_demo.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(molly-imgui PUBLIC imgui imgui/backends)
target_link_libraries(molly-imgui PUBLIC glfw glad)

# Lua (hand-rolled C library target)
file(GLOB LUA_SOURCES lua/src/*.c)
list(REMOVE_ITEM LUA_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/src/lua.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/src/luac.c)
add_library(molly-lua STATIC ${LUA_SOURCES})
target_include_directories(molly-lua PUBLIC lua/src)
set_source_files_properties(${LUA_SOURCES} PROPERTIES LANGUAGE C)

# sol2 (header-only)
add_library(sol2 INTERFACE)
target_include_directories(sol2 INTERFACE sol2/include)
target_link_libraries(sol2 INTERFACE molly-lua)

# Jolt Physics
add_subdirectory(JoltPhysics/Build)

# Assimp (importer list trimmed)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_FBX_IMPORTER ON CACHE BOOL "" FORCE)
add_subdirectory(assimp)

# stb_image (header-only)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE stb)

# miniaudio (header-only)
add_library(miniaudio INTERFACE)
target_include_directories(miniaudio INTERFACE miniaudio)

# spdlog
add_subdirectory(spdlog)

# nlohmann/json (header-only)
add_library(json INTERFACE)
target_include_directories(json INTERFACE json/single_include)

# Catch2
if(MOLLY_BUILD_TESTS)
    add_subdirectory(catch2)
endif()
```

`engine/CMakeLists.txt` (illustrative skeleton):

```cmake
file(GLOB_RECURSE MOLLY_ENGINE_SOURCES src/*.cpp)

add_library(molly-engine STATIC ${MOLLY_ENGINE_SOURCES})

target_include_directories(molly-engine PUBLIC include)

target_link_libraries(molly-engine PUBLIC
    glfw glad glm entt molly-imgui sol2
    Jolt assimp stb miniaudio spdlog::spdlog json
)

target_link_libraries(molly-engine PRIVATE molly_warnings)
```

---

## 41. Appendix G — Sample Shaders

Illustrative Blinn-Phong forward-lit shader pair matching §10.4, showing
the uniform layout the `RenderSystem` (§11.3) and `Material` (§10.2) are
expected to populate each draw call.

`game/assets/shaders/lit.vert`:

```glsl
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;
uniform mat3 u_NormalMatrix;   // inverse-transpose of u_Model, upper 3x3

out vec3 v_FragPos;
out vec3 v_Normal;
out vec2 v_TexCoord;

void main() {
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);
    v_FragPos = worldPos.xyz;
    v_Normal = normalize(u_NormalMatrix * a_Normal);
    v_TexCoord = a_TexCoord;
    gl_Position = u_ViewProjection * worldPos;
}
```

`game/assets/shaders/lit.frag`:

```glsl
#version 460 core

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float radius;
};

#define MAX_POINT_LIGHTS 8

in vec3 v_FragPos;
in vec3 v_Normal;
in vec2 v_TexCoord;

out vec4 o_Color;

uniform sampler2D u_AlbedoTexture;
uniform vec3 u_AlbedoColor;
uniform vec3 u_ViewPos;

uniform DirectionalLight u_DirLight;
uniform int u_PointLightCount;
uniform PointLight u_PointLights[MAX_POINT_LIGHTS];

vec3 computeDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 albedo) {
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 halfway = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfway), 0.0), 32.0);

    vec3 diffuse = diff * light.color * light.intensity * albedo;
    vec3 specular = spec * light.color * light.intensity * 0.3;
    return diffuse + specular;
}

vec3 computePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo) {
    vec3 lightDir = normalize(light.position - fragPos);
    float dist = length(light.position - fragPos);
    float attenuation = clamp(1.0 - (dist / light.radius), 0.0, 1.0);
    attenuation *= attenuation;

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfway = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfway), 0.0), 32.0);

    vec3 diffuse = diff * light.color * light.intensity * albedo;
    vec3 specular = spec * light.color * light.intensity * 0.3;
    return (diffuse + specular) * attenuation;
}

void main() {
    vec3 albedo = texture(u_AlbedoTexture, v_TexCoord).rgb * u_AlbedoColor;
    vec3 normal = normalize(v_Normal);
    vec3 viewDir = normalize(u_ViewPos - v_FragPos);

    vec3 ambient = 0.08 * albedo;
    vec3 result = ambient + computeDirLight(u_DirLight, normal, viewDir, albedo);

    for (int i = 0; i < u_PointLightCount; ++i) {
        result += computePointLight(u_PointLights[i], normal, v_FragPos, viewDir, albedo);
    }

    o_Color = vec4(result, 1.0);
}
```

Notes:

- `u_NormalMatrix` is computed on the CPU side (`glm::transpose(glm::inverse(glm::mat3(model)))`)
  per draw call rather than in the shader, since it only needs to be
  computed once per model matrix, not once per fragment.
- The specular term uses a fixed shininess exponent (`32.0`) and a fixed
  specular strength (`0.3`) rather than exposing them as per-material
  parameters in v1 — simplification consistent with §10.4's "adequate, not
  exhaustive" lighting model philosophy. Promoting these to `Material`
  parameters is a trivial, low-risk follow-up if a specific surface needs
  a different look.

---

## 42. Appendix H — Sample Catch2 Test

Illustrative test file matching the round-trip serialization tests
described in §22.1:

```cpp
// tests/scene/TransformSerialization.test.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "molly/ecs/Scene.h"
#include "molly/ecs/Components.h"
#include "molly/scene/SceneSerializer.h"

using namespace molly;
using Catch::Matchers::WithinAbs;

TEST_CASE("TransformComponent round-trips through scene serialization", "[scene][serialization]") {
    Scene scene;
    Entity entity = scene.createEntity("TestEntity");

    auto& transform = entity.getComponent<TransformComponent>();
    transform.position = { 1.5f, 2.0f, -3.25f };
    transform.rotation = { 0.0f, 90.0f, 0.0f };
    transform.scale = { 2.0f, 2.0f, 2.0f };

    SceneSerializer serializer;
    serializer.serialize(scene, "test_scratch/roundtrip.scene.json");

    Scene loaded;
    serializer.deserialize("test_scratch/roundtrip.scene.json", loaded);

    Entity loadedEntity = loaded.findEntityByName("TestEntity");
    REQUIRE(loadedEntity);

    auto& loadedTransform = loadedEntity.getComponent<TransformComponent>();
    CHECK_THAT(loadedTransform.position.x, WithinAbs(1.5f, 0.0001f));
    CHECK_THAT(loadedTransform.position.y, WithinAbs(2.0f, 0.0001f));
    CHECK_THAT(loadedTransform.position.z, WithinAbs(-3.25f, 0.0001f));
    CHECK_THAT(loadedTransform.rotation.y, WithinAbs(90.0f, 0.0001f));
    CHECK_THAT(loadedTransform.scale.x, WithinAbs(2.0f, 0.0001f));
}

TEST_CASE("ResourceManager caches assets by path", "[resource]") {
    ResourceManager manager;

    auto tex1 = manager.load<Texture2D>("textures/missing_test.png");
    auto tex2 = manager.load<Texture2D>("textures/missing_test.png");

    REQUIRE(tex1.get() == tex2.get());
}
```

---

## 43. Appendix I — CI Workflow

Illustrative full `.github/workflows/build.yml` matching §23:

```yaml
name: Build & Test

on:
  push:
    branches: [main]
  pull_request:
    branches: [main]

jobs:
  build-windows:
    runs-on: windows-latest

    steps:
      - name: Checkout (with submodules)
        uses: actions/checkout@v4
        with:
          submodules: recursive

      - name: Cache build directory
        uses: actions/cache@v4
        with:
          path: build
          key: ${{ runner.os }}-build-${{ hashFiles('vendor/**', 'CMakeLists.txt', 'CMakePresets.json') }}
          restore-keys: |
            ${{ runner.os }}-build-

      - name: Configure
        run: cmake --preset windows-release

      - name: Build
        run: cmake --build --preset windows-release --parallel

      - name: Test
        run: ctest --preset windows-release --output-on-failure
```

---

## 44. Appendix J — `.clang-format`

```yaml
BasedOnStyle: LLVM
Language: Cpp
Standard: c++20

IndentWidth: 4
TabWidth: 4
UseTab: Never
ColumnLimit: 100

BreakBeforeBraces: Attach
AllowShortIfStatementsOnASingleLine: false
AllowShortFunctionsOnASingleLine: Inline
AllowShortLoopsOnASingleLine: false

PointerAlignment: Left
ReferenceAlignment: Left

SortIncludes: CaseSensitive
IncludeBlocks: Regroup

NamespaceIndentation: None
AccessModifierOffset: -4

SpaceBeforeParens: ControlStatements
SpacesInAngles: false
```

---

## 45. Decision Log

A consolidated, chronological record of every major decision made across
this planning process, with the reasoning kept alongside the decision so
none of it needs to be re-derived later. Entries are grouped by topic; see
the referenced section for full detail on each.

| # | Decision | Reasoning summary | Section |
|---|----------|--------------------|---------|
| 1 | C++20, OpenGL 4.6, Lua, Jolt, ImGui as the core stack | User's initial stated stack; well-trodden combination for hobby engines | §3 |
| 2 | Target genre: 2.5D top-down action | Exercises rendering/physics/scripting/editor without the animation-rigging time sink of 3D action/platformers | §1, §31 |
| 3 | ECS via entt, not OOP GameObject/Component | Data-oriented iteration is an explicit learning goal; entt has the larger hobby-engine community | §3.1, §11 |
| 4 | GLFW over SDL2 | Smaller, more focused API; SDL's extra scope (audio) is already covered by miniaudio | §3.1 |
| 5 | sol2 for Lua binding | Ergonomic modern C++ binding layer, avoids hand-writing raw Lua C API glue | §3, §13 |
| 6 | Jolt over Bullet/Box2D | More modern/maintained than Bullet; full 3D (unlike Box2D) matches the 3D-rendered-2.5D-gameplay approach | §3.1, §12 |
| 7 | ImGui docking branch for editor + early in-game UI | Dockable panel layout is close to required for a usable editor; reuse avoids building a second UI system prematurely | §3, §17, §18 |
| 8 | Assimp for model loading, stb_image for textures | Broad format support; both are standard, low-friction choices for a learning project | §3 |
| 9 | miniaudio for audio | Header-only, zero external deps, sufficient feature set for SFX + basic positional audio | §3, §16 |
| 10 | spdlog for logging, nlohmann/json for serialization | Both are the de facto standard, low-friction choices in their category | §3 |
| 11 | CMake as build system | Only realistic option for a multi-target, multi-dependency C++ project without an IDE-solution-file dependency | §5 |
| 12 | Vendor all dependencies via git submodules under `vendor/`, no vcpkg/Conan | Explicit user requirement: the repository must be fully self-contained, nothing external to download/install beyond Git itself | §6 |
| 13 | Catch2 for testing | BDD-friendly, strong CMake/CTest integration, easy to vendor | §22 |
| 14 | GitHub Actions, Windows-only, build+test gate only | Matches Windows-only project scope; no release automation needed yet | §23 |
| 15 | MIT license | Permissive, standard hobby/open-source default | §3 |
| 16 | clang-format, LLVM-based style, project-specific naming conventions | Consistent formatting without manual enforcement; naming conventions chosen for clarity and to distinguish member/static/constant categories at a glance | §7 |
| 17 | Keyboard+mouse primary input; gamepad deferred as a stretch goal | Matches the top-down action genre's natural control scheme; `InputActionMap` design already leaves room for gamepad later | §19 |
| 18 | Editor visual design: dark theme placeholder, explicitly flagged for user redirection | This is the one decision genuinely about the user's taste, not derivable from engineering constraints | §18.1 |
| 19 | Demo game: single-arena wave-survival premise, two enemy archetypes | Keeps content scope minimal while still exercising combat/physics/scripting/UI end-to-end | §31 |
| 20 | No exceptions in engine code; asserts for programmer errors, return values for recoverable failures | Keeps ABI simple, avoids exception-handling overhead in hot loops; only sol2's internal Lua error handling is exempt, caught at the script boundary | §7.4, §25 |
| 21 | Fixed-timestep physics via accumulator, variable-timestep everything else | Standard, well-understood pattern for stable physics decoupled from render frame rate | §27.1 |
| 22 | No async/streamed asset loading in v1 | Content scale (one small demo level) doesn't need it; avoids thread-safety complexity around the registry/renderer before it's justified | §14.3 |
| 23 | No GUID-based asset identity; project-relative path strings used directly | Simpler to reason about and debug; human-readable/greppable in serialized scene JSON | §14.2, §14.5 |
| 24 | No prefab instance-override tracking (Unity-style) in v1 | Accepted limitation; instantiating a prefab produces an independent copy, full override-tracking is meaningfully more design work than v1 scope justifies | §15.3 |
| 25 | Single Lua VM per Scene, not one global VM | Makes editor Play/Stop a clean teardown/recreate with no leftover global state between sessions | §13.1 |
| 26 | Lua scripts sandboxed away from `os`/`io`/`require` | Correctness boundary (deterministic behavior, hot-reload safety), not a security boundary — acceptable since all scripts are self-authored | §13.5 |
| 27 | No automated rendering/physics-behavior testing; manual checklist instead | Screenshot/pixel-diff and full-simulation testing are disproportionate effort for this project's size | §22.2, §22.3 |

---

## 46. Post-v1 Backlog (ideas, deliberately not scheduled)

Captured here so ideas that come up mid-development have somewhere to land
without derailing the current milestone (§33's top risk). Nothing in this
list is committed to — it's a parking lot, reviewed only after Milestone 10
ships.

- **Vulkan renderer backend** behind the existing thin `RendererAPI` seam
  (§10.1), kept as an alternate backend rather than replacing OpenGL.
- **Skeletal animation** — would unlock third-person/platformer genres
  beyond the 2.5D top-down scope; substantial scope on its own (rigging
  pipeline, animation blending, IK) — see §2.2.
- **GUID-based asset identity** to replace path-based asset references
  (§14.2), if asset renaming/moving in a growing project starts causing
  broken references.
- **Prefab instance-override tracking** (§15.3), if hand-maintaining
  independent prefab copies becomes painful at larger content scale.
- **Data-driven in-game UI system** (a real HUD/menu framework, driven by
  Lua or a UI markup format) replacing the direct-ImGui-calls approach in
  §17.3, once a real game's UI needs outgrow what raw ImGui calls can
  reasonably express.
- **Job-system-based multithreading** for gameplay systems beyond Jolt's
  already-internal parallelism (§27.2), if profiling ever shows the
  single-threaded main loop as an actual bottleneck.
- **Async/streamed asset loading**, if content scale grows enough that
  synchronous load-on-first-use (§14.3) causes visible load-time hitches.
- **Shadow mapping** for the directional light, and a small post-processing
  stack (bloom, color grading) — both explicitly deferred per §2.2 pending
  the demo game's visual needs actually outgrowing flat Blinn-Phong
  lighting.
- **Gamepad support**, wired through the `InputActionMap` design that
  already anticipates it (§19.3).
- **Save/progression system** between play sessions, if the demo game
  concept grows beyond a single-session arena survival loop (§31.7).
- **Cross-platform builds** (Linux via GCC/Clang + X11/Wayland through
  GLFW's existing portability, macOS via Metal-backed OpenGL shim or a
  future Vulkan/MoltenVK backend) — reconsider only if there's an actual
  reason (a specific machine, a specific audience) to support them; not
  pursued speculatively.
- **Level editor quality-of-life**: snapping (grid/angle), multi-select
  transform editing, a proper undo/redo history view, prefab
  variant/override UI once instance-override tracking (above) exists.
- **Localization** — not considered at all for v1; the demo game's minimal
  text (wave counter, game over/victory screens) doesn't justify a
  string-table/localization pipeline. Revisit only if the project grows
  into something with substantial UI text.

## 47. Accessibility Notes (v1 scope)

Deliberately minimal for a solo-developer tech demo, but noted rather than
silently ignored:

- Key rebinding is implicitly supported by the `InputActionMap` design
  (§19.2) even though a rebinding *UI* isn't built for v1 — bindings live
  in an editable JSON config file as a manual workaround.
- No colorblind-mode palette adjustments, subtitle/caption system, or
  difficulty-scaling accessibility options in v1 — genuinely out of scope
  given the project's size and the Non-Goals in §2.2. Not treated as a
  permanent non-concern, just not warranted at demo-game scale.
- UI text (health, wave counter, game over/victory) uses ImGui's default
  font at a readable size; no dynamic text-scaling option in v1.

## 48. Project Configuration File

To avoid hardcoding paths/settings (window title, default window size,
starting scene) directly in `main.cpp` for either `molly-editor` or
`molly-demo`, both read a small JSON config file at startup:

`game/assets/project.json` (and an equivalent `editor/assets/project.json`
for editor-specific defaults):

```json
{
  "project_name": "Molly Demo",
  "window": {
    "title": "Molly Demo",
    "width": 1600,
    "height": 900,
    "vsync": true,
    "start_maximized": false
  },
  "startup_scene": "scenes/arena_main.scene.json",
  "asset_root": "assets/"
}
```

- `Application` reads this once at startup (before `Window` creation, so
  the configured width/height/title are used directly rather than
  hardcoded) via a small `ProjectConfig` loader in `engine/core/`.
- Keeping this as plain JSON (not yet another custom format) means it's
  editable by hand or, later, through an editor "Project Settings" panel
  without inventing new tooling.

## 49. Frequently Revisited Questions

Short answers to questions likely to resurface during implementation, so
they don't need re-deciding from scratch each time:

**Q: Should `TransformComponent` cache world matrices instead of
recomputing every frame?**
A: Not until profiling (§24.2) shows the hierarchy walk (§15.1) actually
costs meaningful frame time at the demo game's entity count. Recomputing
each frame is simpler and correct-by-construction; a dirty-flag cache adds
invalidation bugs that aren't worth pre-paying for.

**Q: Should the editor and the standalone game share one executable
(editor mode as a runtime toggle) instead of two separate targets?**
A: No — two separate targets (`molly-editor`, `molly-demo`) both linking
`molly-engine` keeps the shipped game binary free of editor-only code and
ImGui editor panels, and keeps the dependency direction in §4.1 clean.

**Q: What happens if a scene file references an asset that no longer
exists on disk?**
A: Per §14.3/§25, a missing texture falls back to a placeholder
checkerboard texture rather than failing to load the whole scene; missing
meshes/scripts log an error (§25) and the entity keeps its other
components intact (partial load, not an all-or-nothing failure) — losing
one broken reference shouldn't cost you the rest of a hand-authored scene.

**Q: Why JSON instead of a binary scene format?**
A: Diffability in version control and human-readability while debugging
matter more than load-time/file-size efficiency at this project's content
scale (§15.2, §37). Revisit only if scene file size or load time actually
becomes a measured problem.

**Q: Should physics and rendering use the same fixed-vs-variable timestep,
or should rendering also be fixed-step?**
A: Rendering stays variable-step (§27.1) — locking render rate to the
physics step would either cap visual smoothness to 60 FPS unnecessarily on
faster displays, or require interpolation between physics steps for smooth
rendering at higher refresh rates, which is more complexity than this
project's scope currently justifies.

**Q: Why is the demo game scoped to one arena instead of multiple small
levels?**
A: A single arena is enough to prove every engine system end-to-end
(§31.9) while keeping level-content production — the slowest part of game
development for a solo hobby project — to a minimum. Multiple levels are
explicitly parked in the Post-v1 Backlog (§46) rather than cut entirely;
the wave-management script (§31.5) is written generically enough that
adding a second arena later is a content task, not an engineering one.

**Q: If a milestone's stretch goal (e.g. gizmos, undo/redo, hot reload)
doesn't get done in time, does that block moving to the next milestone?**
A: No — stretch items are explicitly marked as such in §32 precisely so
they can slip without blocking progress. The acceptance criteria at the
end of each milestone define the actual gate; stretch goals are bonus
polish, not requirements.

---

*End of plan. This document is expected to evolve — treat it as the
current best understanding of the project, not an immutable spec. Update
sections as decisions are revisited during implementation.*
