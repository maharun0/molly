# Molly Editor — UI/UX Overhaul Plan

**Making the editor look and behave like a standard game engine editor
(Unity / Godot / Unreal / Rider-style), instead of a cluster of
overlapping floating windows.**

---

## Table of Contents

1. [Problem Statement](#1-problem-statement)
2. [Screenshot Diagnosis](#2-screenshot-diagnosis)
3. [Root Cause Analysis](#3-root-cause-analysis)
4. [Reference Survey — How Standard Editors Do This](#4-reference-survey--how-standard-editors-do-this)
5. [Goals & Non-Goals](#5-goals--non-goals)
6. [Design Principles](#6-design-principles)
7. [Target Layout Architecture](#7-target-layout-architecture)
8. [Default Dock Layout — Technical Design](#8-default-dock-layout--technical-design)
9. [Panel-by-Panel Specification](#9-panel-by-panel-specification)
10. [Visual Theme & Styling System](#10-visual-theme--styling-system)
11. [Typography](#11-typography)
12. [Iconography](#12-iconography)
13. [Window Chrome & Title Bar](#13-window-chrome--title-bar)
14. [Responsive Behavior & Minimum Window Size](#14-responsive-behavior--minimum-window-size)
15. [Interaction Patterns](#15-interaction-patterns)
16. [Layout Persistence & Reset](#16-layout-persistence--reset)
17. [Multi-Monitor & DPI Considerations](#17-multi-monitor--dpi-considerations)
18. [Accessibility Notes](#18-accessibility-notes)
19. [Implementation Plan — Phased Milestones](#19-implementation-plan--phased-milestones)
20. [Testing & Verification Plan](#20-testing--verification-plan)
21. [Risk Register](#21-risk-register)
22. [Decision Log](#22-decision-log)
23. [Appendix A — DockBuilder Reference Implementation](#23-appendix-a--dockbuilder-reference-implementation)
24. [Appendix B — Style/Theme Reference Implementation](#24-appendix-b--styletheme-reference-implementation)
25. [Appendix C — Color Palette Table](#25-appendix-c--color-palette-table)
26. [Appendix D — ASCII Layout Mockups](#26-appendix-d--ascii-layout-mockups)
27. [Appendix E — Panel Window-Flag Reference](#27-appendix-e--panel-window-flag-reference)
28. [Appendix F — Before / After Comparison](#28-appendix-f--before--after-comparison)
29. [Appendix G — Toolbar & Status Bar Reference Implementation](#29-appendix-g--toolbar--status-bar-reference-implementation)
30. [Appendix H — Recursive Hierarchy Tree Reference Implementation](#30-appendix-h--recursive-hierarchy-tree-reference-implementation)
31. [Glossary](#31-glossary)
32. [Frequently Revisited Questions](#32-frequently-revisited-questions)

---

## 1. Problem Statement

The screenshot the user provided shows `molly-editor.exe` on first launch,
maximized to a large desktop resolution. What's actually on screen:

- A mostly **empty black window** — no visible 3D viewport, no visible
  Hierarchy panel, no visible Inspector panel, despite all of these being
  implemented and working (per Milestone 7's code).
- A small cluster of **overlapping floating windows** crammed into the
  top-left corner: the Console panel sitting directly on top of the
  Content Browser panel, which is itself sitting on top of leftover
  Toolbar controls (the "Prefab Path" field and "Create Prefab From
  Selection" button), with a sliver of the info window's text
  ("...ection.") visible behind everything.
- Overlapping/garbled text where two windows' title bars or content
  regions intersect (visible in the "Vonsole" artifact, which is actually
  the Console panel's title bar rendered on top of another window's
  content).
- No evidence of the standard "IDE-style" quadrant layout (Hierarchy /
  Viewport / Inspector / Content Browser / Console) that every mainstream
  engine editor uses.

This is not a rendering bug, a crash, or a logic error — every panel is
drawing correctly, in the sense that each individual `ImGui::Begin(...)` /
`ImGui::End()` call succeeds and produces valid, interactive ImGui
content. The problem is entirely **layout**: ImGui was never told where
each panel should go, so it fell back to its default behavior for
undocked windows, which is to cascade new windows in a loose stack near
the top-left of the viewport, slightly offset from each other. With seven
panels (Toolbar, info window, Hierarchy, Inspector, Viewport, Content
Browser, Console) all doing this simultaneously, the result is the
unusable pile in the screenshot.

This plan exists to fix that properly — not with a quick patch, but with
a real editor UI architecture matching what Unity, Godot, Unreal, and
JetBrains Rider all converge on, because that convergence isn't an
accident: it's the layout that actually works well for this class of
tool, and it's the layout every user who's touched a game engine before
will already know how to navigate.

### 1.1 Why This Matters Beyond Cosmetics

A editor that looks like the screenshot isn't just ugly — it's
**functionally broken** for actual use:

- You cannot see the Viewport, which means you cannot see the scene you're
  editing, which means the editor cannot be used to do its one job.
- Panels overlapping means clicks land on the wrong window, buttons
  intended for one panel get triggered by a click that visually looks
  like it's over a different panel.
- There's no way to tell, at a glance, which panel is which, since
  several are only partially visible.
- Every fresh clone of the repository reproduces this exact mess on first
  launch, because there's no checked-in layout — this is not a one-off
  glitch on the user's machine, it will happen for anyone who builds and
  runs the editor for the first time.

### 1.2 Relationship to PLAN.md

This document is a **focused expansion** of `plan/PLAN.md` §18 (Editor
Application) and specifically closes the gap explicitly flagged in
§32 Milestone 7 as a deferred follow-up:

> "A pre-arranged first-launch layout via ImGui's DockBuilder API is
> listed there as a nice-to-have; left as a follow-up since getting it
> right needs visual iteration this session can't do — panels currently
> start floating and the user docks them once, which ImGui then remembers
> via imgui.ini."

That deferral turned out to matter a lot more than "nice to have" — with
enough panels, the undocked default state is not a minor inconvenience,
it's a non-functional editor. This document supersedes that deferral with
a full plan to implement it properly, plus a broader pass on visual
styling so the result actually resembles a standard, professional-looking
engine editor rather than ImGui's out-of-the-box demo aesthetic.

---

## 2. Screenshot Diagnosis

Going through exactly what's visible, panel by panel, to ground the rest
of this plan in the concrete, observed failure rather than a generic
"docking is broken" statement.

### 2.1 What's Visible

| Region | What it actually is | Why it looks broken |
|---|---|---|
| Top strip, "File" | The main menu bar (`ImGui::BeginMainMenuBar()`) | Working correctly, but stranded alone with nothing indicating it belongs to a structured layout below it |
| "▼ Console" bar with a small icon-like glyph before it | Console panel's title bar, with ImGui's collapse triangle | The "V" glyph rendering artifact and "onsole" text suggest a title bar drawn partially behind/under another window's edge |
| "Clear / Trace / Info / Warn / Error / Search..." row | Console panel's filter controls (§9.8 below) | Correctly laid out *within* the Console panel, but the panel itself is tiny and misplaced |
| "ection." fragment, upper right of the cluster | Leftover text from the info window ("...Create Prefab From Selection.") bleeding through from a window behind the Console panel | Two windows occupying overlapping screen space with no z-order the user asked for |
| "prefabs / scripts / shaders / textures" tree | Content Browser panel's file tree | Panel is present and functioning, but rendered *underneath* the Console panel, only the left edge visible |
| "new_prefab.prefab.json / Prefab Path / Create Prefab From Selection" | Toolbar panel's prefab-path controls | Displaced far from the rest of the Toolbar (Play/Stop buttons, Scene Path field aren't visible at all — they're occluded) |
| Everything else (roughly 90% of the window) | Nothing — solid black | This is where the Viewport, Hierarchy, and Inspector panels *should* be visible, but aren't, because they're either occluded underneath the cluster or ImGui placed them off in a location not currently in view (still cascading, just further down/right, or behind the visible cluster in z-order) |

### 2.2 What This Confirms

1. **`ImGui::DockSpaceOverViewport()` is being called** (per
   `EditorLayer::onImGuiRender()`), which is why there's a dockable region
   at all — but calling it only creates the *host* dock space, it does
   not by itself move any window into a dock node. Every panel is still
   an independent floating window until a user manually drags it onto the
   dockspace, or code does it programmatically via `ImGuiDockBuilder`.
2. **No `imgui.ini` was present** for this run (or it was stale/deleted —
   consistent with `.gitignore` excluding it, §16 below), so ImGui had no
   remembered layout to restore and fell back to its default window
   placement heuristic, which is a simple cascade from a fixed origin.
3. **The window is fully maximized** (large desktop resolution, per the
   screenshot's proportions), which makes the tiny cluster look even more
   lost — on a maximized 1920×1080+ window, seven small cascaded panels
   occupy a small fraction of the top-left corner, leaving the rest black
   because nothing is drawn there (no dockspace background fill, no
   viewport, nothing).
4. **Z-order is arbitrary** — whichever panel's `ImGui::Begin()` call
   happened most recently in program order (or was last focused) ends up
   on top, which is why Console is on top of Content Browser, which is on
   top of the Toolbar's prefab controls, with no relationship to which
   panel the user would actually want visible.

---

## 3. Root Cause Analysis

### 3.1 The Missing Piece

`engine/src/ui/ImGuiLayer.cpp` sets up ImGui correctly for docking
(`ImGuiConfigFlags_DockingEnable`), and `editor/src/EditorLayer.cpp`
correctly creates a dockspace host each frame:

```cpp
ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
```

What's missing is the one-time (or "if no saved layout exists yet")
step that actually **splits that dockspace into named regions and assigns
each panel's window name to a region**, using ImGui's `ImGuiDockBuilder`
API family (declared in `imgui_internal.h`):

- `ImGui::DockBuilderRemoveNode`
- `ImGui::DockBuilderAddNode`
- `ImGui::DockBuilderSetNodeSize`
- `ImGui::DockBuilderSplitNode`
- `ImGui::DockBuilderDockWindow`
- `ImGui::DockBuilderFinish`

Without this, every panel's `ImGui::Begin("PanelName")` call creates (or
re-finds) a floating window with no dock assignment, and ImGui's fallback
placement (cascading near the top-left, each new window offset slightly
from the last) is exactly what produces the screenshot.

### 3.2 Why It Wasn't Caught Earlier

Milestone 7's own build/run verification (per `PLAN.md` Milestone 7's
acceptance criteria) confirmed the editor **launched without crashing**
and that the Hierarchy/Inspector/Viewport/Console panels **existed and
functioned individually** when tested in isolation or on a smaller
window. The specific failure mode — many panels cascading into an
unusable pile — only becomes visually obvious with (a) enough panels
simultaneously active, and (b) a large/maximized window making the "empty
90% of the screen" starkly visible. Milestone 7's own text explicitly
flagged this exact gap as a known, deferred risk (§1.2 above) rather than
missing it entirely — this plan is that deferred work, now scoped
properly with an actual screenshot to design against instead of guessing.

### 3.3 Secondary Contributing Issues

Beyond the missing DockBuilder call, the screenshot and a review of the
current panel code surfaces several secondary issues worth fixing in the
same pass, since they all bear on "does this look like a standard engine
editor":

1. **No visual distinction between panel types.** Every panel uses
   ImGui's stock dark theme with no accent color, no header styling
   difference between the Viewport (should feel like a "canvas") and
   data panels like Inspector (should feel like a "form").
2. **No status bar.** Standard editors show contextual info (selected
   entity count, current scene name, play-mode state, FPS) in a slim
   status strip — Molly currently scatters this across floating text in
   the info window (`"Molly Editor - Milestone 8"`), which is itself a
   debug leftover, not a real UI element.
3. **The "info window"** (`ImGui::Begin("Molly Editor - Milestone X")`)
   is a Milestone-numbered debug/status window that was useful during
   development but reads as unfinished/placeholder in a "real" editor —
   it should be retired in favor of a proper status bar and, if needed, a
   dedicated "About" or "Welcome" panel.
4. **No window icon** beyond the default. The screenshot shows a generic
   icon in the title bar; a custom icon is cheap polish that meaningfully
   changes the "is this a real application" impression.
5. **Toolbar mixes unrelated concerns** — Play/Pause/Stop buttons,
   Scene Path text field, Prefab Path text field, and "Create Prefab From
   Selection" all live in one undifferentiated horizontal strip. Standard
   editors separate *transport controls* (play/pause/stop) from *file
   operations* (which belong in menus, not inline text fields).
6. **No visual feedback for Play/Edit mode.** Unity tints its whole
   viewport border blue during Play mode specifically so you can't
   forget you're in Play mode and lose edits; Molly has only a small text
   label (`"(Playing)"`).

This plan addresses the primary docking failure (§7–§9) and all of these
secondary issues (§10–§15) in one coherent pass, rather than patching the
dock layout alone and leaving the editor merely "not broken" instead of
"good."

---

## 4. Reference Survey — How Standard Editors Do This

Before designing Molly's layout, it's worth being explicit about what
"standard" means here, since the user asked for a look matching Unity or
Godot specifically. All the mainstream engine editors converge on a very
similar shape, which is strong evidence it's the right shape rather than
an arbitrary convention.

### 4.1 Unity

- **Menu bar** across the very top (File, Edit, Assets, GameObject,
  Component, Window, Help).
- **Toolbar** directly below the menu bar: transform-tool buttons on the
  left, Play/Pause/Step transport controls centered, account/cloud/layers
  controls on the right.
- **Hierarchy** docked left: a tree of every GameObject in the open
  scene(s).
- **Scene view** (viewport) center, usually tabbed with **Game view**
  (the actual runtime camera output) — clicking between tabs swaps what's
  shown without changing layout.
- **Inspector** docked right: shows every Component on the selected
  GameObject, stacked vertically, each in a collapsible header.
- **Project panel** (asset browser) and **Console** share the bottom
  strip, usually as tabs of the same dock node or side-by-side.
- Dark theme (default since Unity 5+), a single blue accent color used
  consistently for selection highlights, active tabs, and the Play-mode
  border tint.

### 4.2 Godot

- **Menu bar** top-left (Scene, Project, Debug, Editor, Help), with
  **Scene/2D/3D/Script/AssetLib** top-center tabs that swap the *entire*
  main workspace (closer to an IDE's perspective-switching than a
  sub-panel).
- **Scene tree** (their Hierarchy) docked left.
- **Viewport** center, with a **toolbar strip directly above it**
  (specific to the current tab — move/rotate/scale/snap tools for the 3D
  tab).
- **Inspector** docked right, and notably Godot adds a **Node** tab next
  to Inspector for signal/connection editing — a Molly-relevant pattern
  worth keeping in mind for a future "Script Connections" panel, though
  out of scope here.
- **FileSystem** (asset browser) and **Output** (console) bottom, as
  separate dockable panels rather than forced tabs.
- Very dark theme, a purple/violet accent (configurable), consistent
  corner-rounding on panels and buttons.

### 4.3 Unreal Engine

- **Menu bar + toolbar combined** at the top (unusually toolbar buttons
  are large and icon-only, with dropdown menus like "Content Drawer"
  fused into the same strip).
- **World Outliner** (Hierarchy) docked right (Unreal is a notable
  exception to "Hierarchy on the left" — Details panel is *also* on the
  right, stacked below Outliner).
- **Viewport** takes up the dominant central+left area.
- **Details** panel (Inspector) docked right, below World Outliner.
- **Content Browser** (asset browser) docked along the bottom, spanning
  the full width, with a persistent path breadcrumb.
- Very dark, near-black theme, a muted blue-gray accent, heavy use of
  icons over text labels.

### 4.4 JetBrains Rider / Visual Studio (IDE reference point)

Not a game engine, but worth citing since much of "does this look
professional" transfers from general IDE conventions:

- Menu bar, then a toolbar with build/run/debug transport controls
  (visually similar in *role*, if not appearance, to Unity's Play
  button).
- Solution Explorer (Hierarchy-equivalent) docked left.
- Editor area center (tabbed documents).
- Properties/tool windows dockable to any edge, remembered per-workspace.
- A **status bar** along the very bottom — build status, line/column,
  encoding, notifications. Molly currently has no equivalent at all.

### 4.5 What This Means for Molly

Given Molly's existing panel set (Hierarchy, Inspector, Viewport, Content
Browser, Console) maps almost one-to-one onto Unity's core four-panel
layout (Hierarchy, Scene/Game view, Inspector, Project+Console), and the
user explicitly named Unity and Godot as the target look, **Molly's
default layout should follow the Unity/Godot convention directly**:
Hierarchy left, Viewport center (dominant), Inspector right, Content
Browser + Console sharing the bottom strip as tabs. This is figured out
in detail in §7–§8.

---

## 5. Goals & Non-Goals

### 5.1 Goals

- On a **fresh clone + first launch**, the editor opens with a sensible,
  professional-looking, fully visible four-quadrant layout — no manual
  docking required by the user.
- The layout **visually reads as "a real engine editor"** at a glance:
  proper panel arrangement, a real status bar, a coherent color/accent
  scheme, no debug-leftover windows.
- Users can still **freely redock, resize, float, or close** any panel
  (ImGui's docking is a feature, not something this plan removes) — the
  default layout is a *starting point*, not a restriction.
- A **"Reset Layout"** menu action exists so a user who's mangled their
  layout can get back to the good default without deleting files by
  hand.
- The plan is **implementable incrementally** — each phase in §19
  produces a visible, testable improvement, none require reworking
  panels that already work correctly internally (Hierarchy selection
  logic, Inspector widgets, etc. are all untouched by this plan).

### 5.2 Non-Goals

- **Not** rewriting any panel's internal logic/functionality — this is a
  layout and visual-styling pass, not a feature pass. Hierarchy still
  lists entities the same way; Inspector still shows the same component
  widgets; only *where they're docked and how they look* changes.
- **Not** implementing gizmos (translate/rotate/scale handles in the
  viewport) — still explicitly deferred per `PLAN.md` §32 Milestone 7.8.
- **Not** implementing undo/redo — still explicitly deferred per
  `PLAN.md` §32 Milestone 7.9.
- **Not** adding ImGui multi-viewport support (panels floating as
  separate OS windows) — Molly deliberately stays single-OS-window per
  `PLAN.md` §17.2; this plan does not revisit that decision.
- **Not** building a full theme-editor/user-customizable-skin system —
  one well-chosen dark theme with one accent color, not a settings page
  for picking arbitrary colors.
- **Not** redesigning the Content Browser's or Console's *internal*
  widget layout beyond what's needed for consistent spacing/styling —
  their file-tree and log-list logic stays as implemented in Milestone 7.

---

## 6. Design Principles

Six principles guide every specific decision in the sections that follow:

1. **Familiarity over novelty.** Every layout/interaction decision should
   default to "what would a Unity or Godot user expect here," not a
   creative reinvention. The value of matching a standard is that users
   arrive already knowing how to use it.
2. **The Viewport is the star.** In every reference editor, the viewport
   is the largest, most central element. Every other panel is a
   supporting tool arranged *around* it, never competing with it for
   primary screen space.
3. **One accent color, used consistently.** Selection highlights, active
   tab indicators, Play-mode framing, and primary buttons should all use
   the *same* accent color, so the eye learns "this color means
   selected/active/primary" once and it holds everywhere.
4. **No dead/debug UI in the default build.** Anything that reads as "a
   developer's scratch window" (the Milestone-numbered info window) gets
   replaced with production-appropriate UI (a status bar) before this
   plan is considered done.
5. **Fail toward visible, not toward hidden.** If ImGui ever can't
   restore a saved layout (corrupt/missing `imgui.ini`), the fallback
   must be the good default layout (via DockBuilder), never ImGui's raw
   cascade-and-hope behavior that produced the screenshot.
6. **Incremental, testable phases.** Every phase in the implementation
   plan (§19) must leave the editor in a *working, launchable* state —
   no phase should require several others to be complete before the
   editor runs again, mirroring how `PLAN.md`'s own milestones are
   structured.

---

## 7. Target Layout Architecture

### 7.1 The Four-Quadrant-Plus-Strip Model

```
┌──────────────────────────────────────────────────────────────────────┐
│  File   Edit   Window   Help                                          │  ← Main menu bar
├──────────────────────────────────────────────────────────────────────┤
│  [Play] [Pause] [Stop]        Molly Editor              (Edit Mode)   │  ← Toolbar
├───────────────┬────────────────────────────────────┬─────────────────┤
│               │                                    │                 │
│   Hierarchy   │                                    │    Inspector    │
│               │              Viewport               │                 │
│  (tree of     │        (dominant, central,           │  (component    │
│   entities)   │         largest region)               │   widgets for  │
│               │                                    │   selection)    │
│               │                                    │                 │
├───────────────┴──────────────┬─────────────────────┴─────────────────┤
│  Content Browser              │  Console                              │
│  (asset file tree, tabbed     │  (log output, tabbed with Content     │
│   with Console)               │   Browser — see §7.3)                 │
├──────────────────────────────────────────────────────────────────────┤
│  Ready │ Scene: untitled │ 3 entities selected │ Edit Mode │ 60 FPS    │  ← Status bar
└──────────────────────────────────────────────────────────────────────┘
```

This is deliberately the Unity/Godot four-quadrant-plus-strip shape
described in §4.5: Hierarchy left, Viewport center (dominant), Inspector
right, a bottom strip for asset browsing and logging, menu bar and
toolbar on top, and — new to Molly — a status bar on the very bottom.

### 7.2 Proportions

Approximate default proportions, as fractions of the dockspace's total
size (excluding menu bar, toolbar, and status bar, which have fixed
pixel heights, §7.4):

| Region | Width | Height |
|---|---|---|
| Hierarchy (left column) | 18% of total width | 70% of total height (down to the bottom strip) |
| Inspector (right column) | 22% of total width | 70% of total height |
| Viewport (center) | remaining ~60% of width | 70% of total height |
| Bottom strip (Content Browser + Console) | 100% of width | 30% of total height |

These are starting proportions, not hard constraints — once docked, the
user can drag any splitter to resize, and ImGui remembers that (§16).
Inspector is given slightly more width than Hierarchy because component
widgets (drag-floats, color pickers, dropdowns) need more horizontal
room than a simple name-per-row entity list.

### 7.3 Why Content Browser and Console Are Tabbed Together

Godot docks them side by side; Unity tabs them in the same node by
default (though users commonly split them). Given Molly's window is not
guaranteed to be very wide (§14), and both panels are inherently "less
important, glanced at occasionally" compared to Hierarchy/Viewport/
Inspector, tabbing them together in one bottom dock node is the more
robust default — it guarantees both are reachable via a single click
regardless of window width, rather than each getting squeezed to an
unusably narrow sliver on a smaller window. Users who prefer them
side-by-side can drag the Console tab out to split the node themselves;
that becomes their new remembered layout (§16).

### 7.4 Fixed-Height Strips

- **Menu bar**: ImGui's standard main-menu-bar height (driven by font
  size + `FramePadding`; not manually set, matches `ImGui::
  BeginMainMenuBar()` defaults, ~19–22px depending on font).
- **Toolbar**: fixed at 36px tall (comfortable click target for the
  Play/Pause/Stop buttons, discussed in §9.2), not part of the
  dockspace — a plain top-anchored window outside the dock hierarchy,
  matching how it already exists in the code.
- **Status bar**: fixed at 24px tall, bottom-anchored, also outside the
  dockspace.

The dockspace itself (Hierarchy/Viewport/Inspector/Content
Browser/Console) occupies exactly the remaining space between the
Toolbar's bottom edge and the status bar's top edge — this is what
`ImGuiDockNodeFlags_PassthruCentralNode` plus explicit `ImGui::
SetNextWindowPos`/`SetNextWindowSize` calls around the existing
`DockSpaceOverViewport` call achieve (§8.4).

---

## 8. Default Dock Layout — Technical Design

### 8.1 The Core Idea

On the **first-ever launch** (no valid `imgui.ini` present, or a
"Reset Layout" menu action was triggered), build the dock layout
programmatically using `ImGuiDockBuilder`, splitting one root dock node
into five named sub-nodes and assigning each panel's window name to the
correct one. On every subsequent launch, ImGui's normal `imgui.ini`
persistence (§16) restores whatever the user last had — the
programmatic builder only runs when there's nothing to restore, or when
explicitly requested.

### 8.2 Detecting "First Launch / No Saved Layout"

ImGui exposes `ImGui::DockBuilderGetNode(dockspace_id)` — if it returns
`nullptr`, or the returned node has no child nodes yet
(`node->IsSplitNode() == false && node->Windows.Size == 0` immediately
after creating the dockspace this frame), that's the reliable signal
that nothing has been docked into this dockspace ID before, whether
because `imgui.ini` didn't exist, didn't contain this dockspace, or was
just wiped by a "Reset Layout" action. Full logic in Appendix A.

### 8.3 The Split Sequence

Starting from one root node (the full dockspace region), splits happen
in this order — each split takes a node and divides it into two,
returning both halves' IDs:

1. **Root** → split **left 18%** → `dock_hierarchy` ; remainder →
   `dock_main_area`
2. **`dock_main_area`** → split **right 22%** → `dock_inspector` ;
   remainder → `dock_center_and_bottom`
3. **`dock_center_and_bottom`** → split **bottom 30%** →
   `dock_bottom` ; remainder → `dock_viewport`
4. **`dock_bottom`** stays as one node — both Content Browser and
   Console dock into it as tabs (no further split, per §7.3)

Then each panel's window name is assigned to its node:

| Window name (must match `ImGui::Begin(...)` exactly) | Target node |
|---|---|
| `"Hierarchy"` | `dock_hierarchy` |
| `"Viewport"` | `dock_viewport` |
| `"Inspector"` | `dock_inspector` |
| `"Content Browser"` | `dock_bottom` |
| `"Console"` | `dock_bottom` |

Full C++ implementation in Appendix A.

### 8.4 Interaction with `DockSpaceOverViewport`

The existing call:

```cpp
ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
```

passes `dockspace_id = 0`, which means "let ImGui generate an ID for
me" — that's a problem for the builder logic, which needs a **stable,
known ID** to look up/split across frames. Fix: generate the ID
explicitly once and reuse it:

```cpp
ImGuiID dockspaceId = ImGui::GetID("MollyMainDockspace");
ImGui::DockSpaceOverViewport(dockspaceId, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
```

Additionally, `DockSpaceOverViewport` currently spans the *entire* main
viewport, including where the Toolbar and Status bar are drawn — since
those are separate, non-docked windows drawn on top, this mostly works
visually today (they simply draw over the dockspace background), but
it means the dockspace's *usable* area passed to the DockBuilder split
math should account for the toolbar/status-bar heights, otherwise
docked panels' top/bottom edges are hidden behind those strips. The
reference implementation in Appendix A handles this by inset-ing the
viewport rect passed to `DockSpaceOverViewport` by the toolbar height
(top) and status bar height (bottom) before creating the dockspace,
rather than passing the raw main viewport.

### 8.5 Persistence Across Restarts

Once the DockBuilder has run once and the user (or the default) has
docked every window, ImGui's normal `imgui.ini` write-on-exit /
read-on-startup behavior takes over completely — the DockBuilder code
only executes when `ImGui::DockBuilderGetNode()` indicates there's
nothing to restore. See §16 for the full persistence story, including
why `imgui.ini` is currently gitignored and what (if anything) should
change about that.

---

## 9. Panel-by-Panel Specification

### 9.1 Main Menu Bar

**Current state**: a single `"File"` menu containing New/Save/Open Scene.

**Target state**: expand to match the standard menu-bar shape users
expect, without inventing features this plan doesn't cover:

| Menu | Items |
|---|---|
| **File** | New Scene, Save Scene, Save Scene As..., Open Scene, — (separator) —, Exit |
| **Edit** | (disabled placeholder items: Undo, Redo — grayed out, since undo/redo is explicitly deferred per `PLAN.md` §32 Milestone 7.9; showing them *disabled* rather than omitting the menu entirely sets correct expectations that it's planned, not forgotten) |
| **Window** | Reset Layout, — (separator) —, checkboxes to show/hide each panel (Hierarchy, Viewport, Inspector, Content Browser, Console) — checkbox state reflects whether that panel's ImGui window is currently open |
| **Help** | About Molly (opens a small modal with engine version, build config, and a link back to this repo — no external URL fetching, just static text) |

"Exit" calls `Application::get().close()`. "Reset Layout" is the trigger
described in §8.2/§16.4. The Window menu's show/hide checkboxes are a
standard IDE affordance for recovering an accidentally-closed panel
without needing "Reset Layout" (which resets *everything's position*,
not just visibility).

### 9.2 Toolbar

**Current state**: Play/Stop/Pause buttons, a state label, then — in the
same window — a Scene Path field, a Prefab Path field, and a "Create
Prefab From Selection" button, all in one undifferentiated strip.

**Target state**: split into two visually distinct zones within the same
toolbar strip, matching how Unity separates transport controls from
everything else:

```
┌────────────────────────────────────────────────────────────────────┐
│   ▶ Play   ⏸ Pause   ■ Stop        │      [Edit Mode]                │
└────────────────────────────────────────────────────────────────────┘
```

- **Left zone**: Play/Pause/Stop as icon buttons (§12) with tooltips,
  not text labels — matches every reference editor's transport-control
  convention. Only the relevant buttons are shown per state (already the
  existing logic: Play alone in Edit mode; Stop+Pause/Resume once
  playing) — kept, just re-skinned.
- **Right zone** (right-aligned via `ImGui::SameLine()` +
  `ImGui::SetCursorPosX()` math, or a right-aligned child region): a
  colored state badge — "Edit Mode" (neutral gray), "Playing" (accent
  color background), "Paused" (amber/yellow background) — replacing the
  plain `ImGui::TextDisabled("(%s)")` label with something that reads
  at a glance, matching Unity's play-mode-tint philosophy (§9.3).

**Removed from the Toolbar entirely**: Scene Path field, Prefab Path
field, "Create Prefab From Selection" button. These move to:
- Scene Path + Save/Open actions → already covered by the **File menu**
  (§9.1); the free-text path field is replaced by a proper (if still
  simple) file-path input living in a small "Save Scene As..." modal
  dialog rather than a permanently-visible toolbar field (§9.1, §15.4).
- Prefab Path + "Create Prefab From Selection" → moves to the
  **Hierarchy panel's right-click context menu** (§9.4) as a
  "Create Prefab From Selection..." item, which opens a small modal
  prompting for the save path — this matches where a user would
  intuitively look for "turn my selected entity into a prefab" (acting
  *on* the selection, in the panel that shows the selection), rather
  than a disconnected toolbar field.

This is a meaningful rearrangement, not just re-skinning, but it directly
fixes root-cause issue #5 from §3.3 (Toolbar mixing unrelated concerns)
and removes two of the elements crowding the toolbar in the screenshot.

### 9.3 Viewport

**Current state**: functionally correct (framebuffer render-to-texture,
`ImGui::Image` display, resize handling, editor fly-camera) but visually
plain — no border treatment, no on-screen hints beyond the separate info
window's text.

**Target state additions** (all additive — no change to the existing
render/resize/camera logic in `ViewportPanel.cpp`):

1. **Play-mode border tint.** When `EditorPlayState != Edit`, draw a
   2–3px colored border around the Viewport's image using
   `ImGui::GetWindowDrawList()->AddRect(...)` immediately after the
   `ImGui::Image()` call — accent-colored for Playing, amber for Paused.
   This is the single highest-value addition from this section: it's
   the exact mechanism Unity uses to stop users from editing a scene
   they think is stopped but is actually mid-Play, and Molly has no
   equivalent today.
2. **Corner overlay text**, drawn via `ImGui::GetWindowDrawList()->
   AddText()` (not a child ImGui window, to avoid stealing input focus
   from the 3D view) in the viewport's top-left corner: current FPS,
   and — only when hovered and in Edit mode — a one-line reminder of
   the fly-camera controls ("RMB + WASD/QE to fly"), replacing the
   permanently-visible text currently living in the separate info
   window.
3. **Empty-state message.** If no primary camera exists in the active
   scene (`findPrimaryCamera` returns `entt::null`), draw a centered
   "No active camera in this scene" message instead of leaving the
   framebuffer's clear color as the only feedback — a small but real
   usability improvement over silently showing a blank dark rectangle
   indistinguishable from "still loading."

### 9.4 Hierarchy

**Current state**: functionally correct — lists every entity with a
`TagComponent`, click-to-select, "Create Entity" button, click-empty-
space-to-deselect. Visually plain (no icons, no indentation reflecting
actual parent/child structure despite `HierarchyComponent` existing).

**Target state additions**:

1. **Actual tree structure.** Currently every entity is listed flat via
   `registry.view<TagComponent>()`, ignoring `HierarchyComponent.parent`/
   `.children` entirely for display purposes (it's used for transform
   composition and serialization, just not shown in the UI tree). Change
   to a recursive draw: start from root entities (those with
   `parent == entt::null`), use `ImGui::TreeNodeEx` per entity with
   `ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth`,
   recursing into `.children`. This is a real, if small, functional
   change (not just styling) but is included here because it's the
   single biggest gap between "list of names" and "looks like a
   standard engine Hierarchy panel."
2. **Right-click context menu** per entity: Rename, Duplicate, Delete,
   Create Prefab From Selection... (moved here per §9.2), and — for
   empty space — Create Entity, Create Empty Parent.
3. **Row icons.** A small icon per entity row based on its component
   makeup (camera icon if it has `CameraComponent`, light bulb if
   `LightComponent`, cube if `MeshRendererComponent`, plain dot
   otherwise) — see §12.2 for the icon strategy.
4. **Selection highlight** uses the shared accent color (§10.3),
   replacing ImGui's default selection blue with Molly's own accent so
   it's visually consistent with the Viewport's play-mode border and the
   Inspector's active-field indicators.

### 9.5 Inspector

**Current state**: functionally complete per `PLAN.md` Milestone 7.4 —
every component type has widgets, "Add Component" dropdown works.
Visually plain — no icons per component header, no visual grouping.

**Target state additions**:

1. **Component header icons**, matching the Hierarchy's row icons
   (§9.4.3) for visual consistency — same icon set, reused.
2. **Component header right-click → "Remove Component"** context menu
   (currently there's no way to remove a component once added except by
   hand-editing — a real, small functional gap worth closing in this
   pass since it directly relates to the header styling work already
   being done here).
3. **Consistent field width.** `ImGui::PushItemWidth(-FLT_MIN)` (or a
   fixed fraction of available width) at the top of each component's
   draw block, so drag-floats/dropdowns fill the panel's width instead
   of using ImGui's default fixed pixel width, which currently makes the
   Inspector look cramped on its right edge whenever the panel is wider
   than ImGui's default assumption.
4. **Entity header block** above the component list: a larger, bold
   rendering of the entity's name/tag (currently a plain
   `ImGui::InputText` at default size) plus the "Add Component" button
   moved to the right side of that same header row rather than
   `ImGui::SameLine()`-appended directly after the name field.

### 9.6 Content Browser

**Current state**: read-only file tree under `assets/`, functionally
matches `PLAN.md` §18.3's Milestone-7 scope (full asset interaction
explicitly deferred to Milestone 8+, which has since landed serialization
but not asset-browser interactivity).

**Target state additions** (kept intentionally minimal — this plan is
UI/layout-focused, not an asset-pipeline feature pass):

1. **Breadcrumb path bar** at the top (`assets > scripts >`) instead of
   only a nested tree, matching Unreal's Content Browser convention
   (§4.3) and making "where am I in the asset tree" clearer than tree
   indentation alone.
2. **File-type icons** in the tree (script file icon for `.lua`, image
   icon for `.png`, JSON icon for `.json`, generic file icon otherwise)
   — same icon-strategy dependency as §9.4.3/§9.5.1.
3. **Double-click behavior**: for `.lua` files, shell out to open in the
   user's default text editor (`ShellExecuteW` on Windows, matching
   how a "reveal in file manager" action would also work) — small,
   genuinely useful, and squarely a "does this feel like a real tool"
   detail.

### 9.7 Console

**Current state**: functionally complete per Milestone 7 (custom spdlog
sink, severity filter checkboxes, search box, auto-scroll). Visually
plain — filter checkboxes are default ImGui checkboxes with no color
coding matching each severity's actual log-line color.

**Target state additions**:

1. **Colored filter checkboxes** — tint each severity checkbox's label
   with that severity's log color (already used for the log lines
   themselves via `colorForLevel()`), so "Warn" reads in the same amber
   used for warning lines, "Error" in the same red — currently the
   checkboxes are plain white/default-colored regardless of what they
   filter.
2. **Line count badge** next to the panel's tab label (e.g. "Console
   (3)" when there are 3 unread error-level lines since last viewed) —
   a small, standard "there's something you should look at" affordance
   for a panel that's tabbed with Content Browser and therefore easy to
   leave unfocused for a while (§7.3).
3. **Click-to-copy** a log line to the clipboard via
   `ImGui::SetClipboardText()` on click — trivial addition, standard
   console-panel behavior.

### 9.8 Status Bar (New Panel)

Does not exist today. New, thin (24px), bottom-anchored, non-dockable
window (outside the dockspace, like the Toolbar) showing, left to right:

```
Ready │ Scene: arena_main.scene.json │ 3 selected │ Edit Mode │ 60.2 FPS
```

- **"Ready"** (or a transient message like "Scene saved" /
  "Hot-reloaded shader: lit.frag", shown for ~2 seconds after the
  triggering event, then reverting to "Ready") — a lightweight,
  non-modal way to surface exactly the kind of one-off confirmation
  messages that currently only appear in the Console (buried among
  everything else logged there).
- **Scene name** — derived from the current `m_scenePathBuffer`
  (editor) filename, or "untitled" if never saved.
- **Selection count** — `0 selected`, `1 selected`, or `N selected` once
  multi-select exists (not yet implemented — single-select only today,
  so this always reads `0` or `1`; the status bar element is still worth
  adding now since it costs nothing extra and is forward-compatible).
- **Play state** — same three-state label as the Toolbar's badge
  (§9.2), duplicated here per standard IDE convention (status bar
  redundantly surfaces "mode" info that's also shown elsewhere, since
  it's cheap peripheral-vision information).
- **FPS** — already computed every frame via `ImGui::GetIO().Framerate`
  (currently only shown in the retiring info window, §9.9) — moved here.

This single new panel directly replaces the Milestone-numbered debug info
window (§9.9) as the home for "frame time / FPS," and gives transient
confirmations (like a successful scene save) a home that isn't "buried in
the Console."

### 9.9 Retiring the Info Window

`ImGui::Begin("Molly Editor - Milestone 8")` (the window whose title
changes every milestone, currently showing static instructional text and
the FPS counter) is removed entirely. Its three pieces of content
relocate as follows:

| Old content | New home |
|---|---|
| "Molly Editor - Milestone N" title / milestone description text | Removed — was development-process signaling, not user-facing UI; the editor no longer needs to announce its own milestone number to the end user |
| Frame time / FPS text | Status bar (§9.8) |
| "Hold Right Mouse in the Viewport to fly..." hint text | Viewport corner overlay, shown only when hovered + Edit mode (§9.3.2) |

This directly resolves root-cause issue #3 from §3.3.

---

## 10. Visual Theme & Styling System

### 10.1 Why a Custom Theme, Not Stock `StyleColorsDark()`

`PLAN.md` §18.1 explicitly flagged the editor's visual theme as a
placeholder — "dark theme by default, ImGui's native dark style lightly
customized... rather than a from-scratch reskin" — and deferred picking
an accent color until there was a real UI to react to. There now is one,
and the current screenshot demonstrates exactly why stock
`ImGui::StyleColorsDark()` alone isn't enough: every panel, button, and
highlight uses ImGui's default muted-blue-gray palette with no
identity, no consistent accent, and (per §6 principle 3) no single color
the eye can learn to associate with "selected/active."

This section defines that missing accent + supporting palette,
staying within `PLAN.md` §18.1's original direction (dark theme, flat,
low-chrome — not attempting a from-scratch reskin with custom-drawn
widgets, just a disciplined `ImGuiStyle` color/spacing pass).

### 10.2 Palette Selection Rationale

Given the target look is explicitly "Unity or Godot," and Unity's accent
is a saturated blue while Godot's is a saturated blue-violet, Molly's
accent lands **between the two**: a clean, moderately saturated blue
(`#3D8BFF`-ish) that reads as "engine tool blue" without directly
imitating either product's exact brand color. This keeps Molly visually
its own thing while unmistakably sitting in the same genre.

Supporting neutrals follow the same "near-black, not pure-black" dark
theme convention both reference editors use — pure black (`#000000`)
backgrounds read as harsh/unfinished; a very dark blue-gray
(`#1A1C21`-ish) reads as intentional.

Full color table with exact hex/RGBA values: **Appendix C**.

### 10.3 Where the Accent Color Is Used

Per §6 principle 3 (one accent, used consistently), the single accent
color (`kAccent`, Appendix C) drives *every* one of these, and nothing
else in the UI uses a different "highlight" color:

- Hierarchy selected-row background
- Inspector active/focused input border
- Active dock-tab underline/highlight
- Toolbar Play button icon + Playing-state badge background
- Viewport Playing-mode border tint (Paused uses the separate amber
  warning color, not the accent, since it's meaningfully a different
  state)
- Primary/default buttons in modals (e.g. "Save" in the Save-As dialog)
- Progress bars, checkboxes, and slider grab handles (ImGui style slots:
  `ImGuiCol_CheckMark`, `ImGuiCol_SliderGrab`, `ImGuiCol_SliderGrabActive`)

### 10.4 Spacing & Rounding

| Style var | Current value | New value | Rationale |
|---|---|---|---|
| `WindowRounding` | 4.0f | 6.0f | Slightly softer corners read as more polished; matches Godot's rounding more than Unity's near-zero rounding, a deliberate middle ground |
| `FrameRounding` | 0.0f (default) | 3.0f | Buttons/inputs get subtle rounding too, avoiding the "rounded windows, sharp buttons" mismatch |
| `WindowPadding` | (8,8) (default) | (10,10) | Slightly more breathing room in panel content, reduces the "cramped" feeling visible in the screenshot's tiny cluster |
| `FramePadding` | (4,3) (default) | (6,4) | Larger click targets on buttons/checkboxes, especially relevant for the Toolbar's icon buttons (§9.2) |
| `ItemSpacing` | (8,4) (default) | (8,6) | Slightly more vertical breathing room between Inspector rows |
| `ScrollbarSize` | 14 (default) | 12 | Marginally slimmer scrollbars, common in modern tool UIs |
| `TabRounding` | 4.0f (default) | 4.0f | Kept as-is — already reasonable |
| `GrabRounding` | 0.0f (default) | 3.0f | Slider/scrollbar grab handles match the general rounding pass |

### 10.5 What Does *Not* Change

- No custom fonts beyond what's covered in §11 (no per-panel font
  switching, no custom icon-font-driven redraws of ImGui's own chrome
  like scrollbar arrows).
- No custom `ImDrawList`-level redraws of standard widgets (buttons,
  checkboxes, sliders keep ImGui's normal rendering, just with new
  colors/rounding/spacing — not hand-drawn replacements).
- No light-theme variant — `PLAN.md` §18.1 already settled on dark-only,
  this plan doesn't revisit that.

---

## 11. Typography

### 11.1 Current State

ImGui's built-in default font (a bundled proggy-clean-derived bitmap
font) at its default size. Perfectly legible, but visibly "placeholder"
— every ImGui demo/prototype looks like this, which undercuts the "looks
like a finished tool" goal on its own, independent of layout.

### 11.2 Target State

Ship one real TrueType font, loaded once at `ImGuiLayer::onAttach()` via
`io.Fonts->AddFontFromFileTTF(...)`, at a comfortable base size (15–16px
depending on final visual testing). Candidate: **Inter** or **Roboto**
(both free, permissively licensed, and exactly the "clean, neutral,
professional UI sans-serif" register that Unity/Godot/Rider all use for
their own chrome) — vendored the same way other assets are, as a
checked-in `.ttf` file under a new `editor/assets/fonts/` directory (not
a submodule, a single small binary file, consistent with how
`floor_checker.png` was handled).

A **second, monospace font** (e.g. **JetBrains Mono** or **Fira Code**,
also free/permissive) loaded as an additional ImGui font and pushed via
`ImGui::PushFont()`/`PopFont()` specifically around the **Console
panel's** log-line rendering — log output reads meaningfully better in a
monospace face (aligned timestamps, easier-to-scan structured text) than
the general UI font, and this is exactly what every reference IDE/editor
does for their own console/output panels.

### 11.3 Why Not Just a Larger Default Font

Simply calling `io.Fonts->AddFontDefault()` with a size multiplier keeps
the same bitmap-font *character* even if scaled, which doesn't fix the
"looks like an ImGui demo" impression — the shape of the default font's
glyphs is the tell, not just their size. A real TTF font is a small,
one-time addition (a few hundred KB binary + ~10 lines of loading code)
for a large visual-polish return.

---

## 12. Iconography

### 12.1 Approach: One Bundled Icon Font

Rather than individual PNG icons per UI element (more asset-management
overhead, harder to recolor to match the accent-color system in §10.3),
use a single **icon font** — glyphs rendered as text characters via
ImGui, exactly like the main UI font, which means icons automatically
inherit `ImGui::PushStyleColor`-driven tinting for free (critical for
§9.4.3/§9.5.1's "icon matches component type" and §10.3's "everything
uses the one accent color where relevant").

Candidate: **Font Awesome Free** (solid variant) or **Material Symbols**
— both free/permissive, both already have the exact icon set this plan
needs (play, pause, stop, folder, file, camera, lightbulb, cube, script/
document, search, trash, copy, warning triangle, error circle,
info circle). Loaded as a second font merged into the main UI font's
range via `io.Fonts->AddFontFromFileTTF(..., ImGuiConfigFlags glyph
range for icons...)` — the standard "icon font merged with text font"
ImGui pattern, so icon glyphs can be inlined directly into normal
`ImGui::Text()`/`ImGui::Button()` calls via their Unicode codepoints.

### 12.2 Icon Assignment Table

| Context | Icon | Used in |
|---|---|---|
| Play | ▶ (solid play triangle) | Toolbar (§9.2) |
| Pause | ⏸ (solid pause bars) | Toolbar |
| Stop | ■ (solid square) | Toolbar |
| Entity (generic, no distinguishing component) | small dot/circle | Hierarchy row (§9.4.3) |
| Entity with `CameraComponent` | camera icon | Hierarchy row, Inspector Camera header |
| Entity with `LightComponent` | lightbulb icon | Hierarchy row, Inspector Light header |
| Entity with `MeshRendererComponent` | cube icon | Hierarchy row, Inspector Mesh Renderer header |
| Entity with `ScriptComponent` | document/script icon | Hierarchy row (secondary badge), Inspector Script header |
| Entity with `RigidBodyComponent`/`ColliderComponent` | physics/collision icon | Inspector Rigid Body / Collider headers |
| Entity with `AudioSourceComponent`/`AudioListenerComponent` | speaker icon | Inspector Audio headers |
| Folder (closed/open) | folder icons | Content Browser tree (§9.6) |
| `.lua` file | script icon | Content Browser |
| `.png`/`.jpg` file | image icon | Content Browser |
| `.json` file | data/document icon | Content Browser |
| Log: Trace | (no icon, or a faint dot) | Console filter row (§9.7) |
| Log: Info | info-circle icon | Console filter row |
| Log: Warn | warning-triangle icon | Console filter row |
| Log: Error/Critical | error-circle icon | Console filter row |
| Search | magnifying-glass icon | Console search box, Content Browser (if a search box is added later — not in this plan's scope) |
| Delete/Remove | trash-can icon | Hierarchy/Inspector context menus |

### 12.3 Fallback Behavior

If the icon font fails to load (missing file, corrupt TTF — same
defensive posture as any other asset load in the engine), every icon
usage site falls back to a short text label instead (e.g. "Play" instead
of ▶) rather than rendering a missing-glyph box (`□`) or crashing —
matching the engine-wide convention of graceful degradation over hard
failure (`PLAN.md` §25).

---

## 13. Window Chrome & Title Bar

### 13.1 Application Icon

The screenshot shows a generic/default window icon in the title bar next
to "Molly Editor". Add a real icon: a simple, small (256×256, standard
multi-resolution `.ico` for Windows) mark — doesn't need to be
elaborate, a clean geometric "M" monogram or an abstract shape in the
accent blue (§10.2) on a transparent background is sufficient — set via
`glfwSetWindowIcon()` in `Window::init()` (a small, additive change to
existing engine code, not editor-specific, so `molly-demo` picks up a
matching treatment for free if it sets its own icon file via the same
mechanism later).

### 13.2 Title Bar Text

Currently `"Molly Editor"` (static). No change needed to the base title,
but append the current scene name once a scene is loaded/saved,
matching standard document-editor convention (`"Molly Editor -
arena_main.scene.json"`), and an asterisk when there are unsaved changes
once change-tracking exists (not in this plan's scope — noted here as a
natural follow-up once undo/redo, §5.2, eventually lands, since dirty-
state tracking and undo history are closely related features).

---

## 14. Responsive Behavior & Minimum Window Size

### 14.1 Minimum Window Size

Currently unset (GLFW's default is effectively "any size, including
absurdly small"). Set an explicit minimum via
`glfwSetWindowSizeLimits()` in `Window::init()` — **1024×720** as a
floor, below which the five-panel default layout genuinely cannot
display anything usefully (matching roughly what Unity/Godot both
enforce as a practical minimum for their own default layouts).

### 14.2 Narrow-Window Behavior

Below the default-launch size but above the hard minimum (§14.1), ImGui's
existing dock-splitter dragging already lets the user shrink any panel
arbitrarily thin (down to a small minimum ImGui itself enforces per dock
node) — no special-cased "responsive" layout switching is planned (e.g.
no automatic collapse of Hierarchy/Inspector into icon-only strips at
narrow widths, which would be a significantly larger feature than this
plan's scope). This matches Unity/Godot's own behavior — they don't
auto-collapse either, they rely on the user manually resizing or hiding
panels via the Window menu (§9.1) when working in a constrained space.

### 14.3 Startup Size

Currently hardcoded to 1600×900 (`WindowProps` default, `PLAN.md`
Appendix D). Kept as-is — a reasonable default that comfortably fits the
five-panel layout with room to spare, and already configurable via
`project.json` per `PLAN.md` §48 once that's wired up (not yet
implemented; noted as consistent with existing plan, not a new
requirement from this document).

---

## 15. Interaction Patterns

### 15.1 Selection Synchronization

Already correctly implemented (Hierarchy owns `m_selected`, passed to
Inspector each frame per `PLAN.md` Milestone 7's
`SceneHierarchyPanel`/`InspectorPanel` split) — no change needed to the
underlying mechanism, only the visual highlight color (§9.4.4).

### 15.2 Drag-and-Drop (Deferred, Noted for Future Work)

Standard editors support dragging a Content-Browser asset directly onto
a Hierarchy entity or Viewport surface (e.g. drag a `.lua` file onto an
entity to attach it as a `ScriptComponent`, or drag a prefab file into
the Viewport to instantiate it at the drop location). **Not implemented
in this plan** — flagged here explicitly as a natural, high-value
follow-up once this layout/styling pass is done, using ImGui's
`ImGui::BeginDragDropSource()`/`AcceptDragDropPayload()` API, which pairs
naturally with the Content Browser's existing file-tree structure.
Listed for completeness/future-planning per this document's own
thoroughness goal, not because it's in scope now.

### 15.3 Context Menus

New in this plan for Hierarchy (§9.4.2) and Inspector component headers
(§9.5.2) — both via `ImGui::BeginPopupContextItem()`, the standard ImGui
per-row/per-item right-click pattern, consistent with the existing
"Add Component" popup's `ImGui::BeginPopup()` usage.

### 15.4 Modals: Save Scene As / Create Prefab From Selection

Both moved out of always-visible toolbar text fields (§9.2, §9.4.2) into
small modal dialogs (`ImGui::BeginPopupModal()`), each with: a single
text input pre-filled with a sensible default path, a "Save"/"Create"
primary button (accent-colored per §10.3), and a "Cancel" secondary
button. This is a meaningfully better interaction than a permanently-
visible text field the user has to remember is there — matches how every
reference editor handles "save as" style actions (a modal, not a
persistent toolbar field).

### 15.5 Keyboard Shortcuts

Not a focus of this plan (no new shortcut system is being built), but
since the File menu (§9.1) now exists in a real form, standard shortcuts
should be wired alongside it using ImGui's `ImGui::MenuItem(label,
shortcut_string)` (which only *displays* the shortcut text — actual key
handling still needs `ImGui::IsKeyPressed()` checks, small additions
next to the existing menu code):

| Action | Shortcut |
|---|---|
| Save Scene | Ctrl+S |
| Open Scene | Ctrl+O |
| New Scene | Ctrl+N |
| Play/Stop toggle | Ctrl+P |
| Delete selected entity | Delete |

This list intentionally matches `PLAN.md` Appendix E's already-planned
keybinding table — this plan is the point at which those bindings
actually get wired to the now-real menu items, closing a small
pre-existing gap between the documented plan and the implemented editor.

---

## 16. Layout Persistence & Reset

### 16.1 Current State

`imgui.ini` (ImGui's default persistence file, written on
`ImGui::DestroyContext()`/periodically, read on `ImGui::CreateContext()`
+ first `NewFrame()`) is **gitignored** (`.gitignore`: `imgui.ini`) and
written into whatever the process's current working directory is at
runtime (`editor/` or `game/`, per how the exes are launched per
`README.md`). This means every fresh clone starts with **no saved
layout**, which is exactly the state that produces the screenshot absent
the DockBuilder fix in §8.

### 16.2 What Changes

With §8's DockBuilder fix in place, a missing `imgui.ini` is no longer a
problem — it's precisely the condition that triggers the good
programmatic default layout. `imgui.ini` staying gitignored (not
checked in) is therefore the **correct** choice going forward, not an
oversight: every user's own manual layout tweaks (panel resizing,
re-tabbing Console next to Hierarchy if they prefer, etc.) persist
locally across their own sessions without polluting the repository with
a file that's really per-user preference, not shared project state.

This directly contradicts what `PLAN.md` §18.2 originally
envisioned — a **checked-in** `default_layout.ini` as the mechanism for
a good first-launch layout. This plan formally supersedes that specific
detail: the DockBuilder approach (programmatic, code-driven default)
achieves the same goal more robustly than a hand-crafted ini file would
have, for the reasons given in §8 and in `PLAN.md` §18.2's own
follow-up note acknowledging the ini-file approach wasn't attempted
because it "needs visual iteration this session can't do." A
programmatic layout needs no such visual hand-tuning of a binary-ish ini
format — it's specified in readable C++ (Appendix A) and reviewable like
any other code change.

### 16.3 `.gitignore` — No Change Needed

`imgui.ini` stays in `.gitignore` exactly as-is. No action item here
beyond confirming the reasoning (§16.2) — included as its own
subsection because a reader of this plan coming from `PLAN.md` §18.2
would reasonably expect a "check in default_layout.ini" task; this
section explains why that specific task is intentionally *not* being
done, in favor of the DockBuilder approach.

### 16.4 "Reset Layout" Menu Action

Per §9.1's Window menu: clicking "Reset Layout" should:

1. Call `ImGui::DockBuilderRemoveNode(dockspaceId)` to tear down the
   current dock structure entirely.
2. Re-run the exact same split/dock sequence from §8.3 (factored into a
   single `buildDefaultDockLayout(ImGuiID dockspaceId)` function,
   Appendix A, called both from the "first launch" path and this reset
   path — one implementation, two call sites, no duplicated logic).
3. Leave individual panels' *internal* state untouched (Hierarchy
   selection, Console filter checkboxes, etc.) — only positions/sizes/
   dock assignments reset, not panel content or settings.

---

## 17. Multi-Monitor & DPI Considerations

Out of deep scope for this plan (no work item requires touching DPI
scaling code), but worth a brief note since the screenshot's window
appears to be a large/high-resolution display: ImGui's `io.DisplaySize`
is already correctly driven by the actual framebuffer size each frame
(`ImGuiLayer::end()`, existing code, unchanged by this plan), so the
dockspace and DockBuilder splits in §8 are computed in imgui's own
logical coordinate space and scale correctly with window size —
no separate "high-DPI mode" branch is needed for the layout itself.
Font rendering crispness at very high DPI (>150% Windows scaling) is a
separate, genuinely unaddressed concern (`io.FontGlobalScale` /
per-monitor DPI callbacks are not implemented anywhere in the engine
today) — flagged here as a known gap for future work, not something this
UI plan resolves.

---

## 18. Accessibility Notes

Per `PLAN.md` §47's existing accessibility scoping (deliberately
minimal for a solo-developer tool), this plan doesn't add a colorblind
palette mode or scalable-text settings UI. Two small, low-cost
accommodations are included anyway since they fall directly out of work
already being done in this plan:

- The Console's severity colors (§9.7.1) use *both* color and an icon
  (§12.2) to distinguish Warn/Error/Info — not color alone, which is a
  basic colorblind-friendly practice that costs nothing extra once icons
  are already being added for other reasons.
- The accent color (§10.2, `#3D8BFF`-ish) was chosen partly for
  reasonable contrast against the dark background palette (Appendix C
  documents contrast-relevant values), though no formal WCAG contrast
  audit is being performed as part of this plan — noted as a limitation,
  not a solved problem.

---

## 19. Implementation Plan — Phased Milestones

Structured the same way `PLAN.md` §32 structures the engine's own
milestones: each phase is independently completable, leaves the editor
in a working/launchable state, and has explicit acceptance criteria.

### Phase 1 — Fix the Core Docking Failure (highest priority)

The minimum change that resolves the screenshot's actual bug, isolated
from every other improvement in this plan so it can ship first and
alone if needed.

1.1. Give the dockspace a stable, explicit `ImGuiID` (§8.4) instead of
     passing `0`.
1.2. Implement `buildDefaultDockLayout()` per §8.3/Appendix A.
1.3. Call it from `EditorLayer::onImGuiRender()` guarded by the "no
     existing layout" check (§8.2), immediately after the
     `DockSpaceOverViewport` call.
1.4. Inset the dockspace's rect by the Toolbar's and (once it exists,
     Phase 5) status bar's heights (§8.4) so docked panels don't render
     underneath either strip.

**Acceptance**: fresh clone → submodule init → build → first launch
shows all five panels (Hierarchy, Viewport, Inspector, Content Browser,
Console) correctly docked in the §7.1 quadrant layout, fully visible,
no overlapping windows, Viewport clearly showing the rendered scene.

### Phase 2 — Toolbar Rework & Status Bar

2.1. Split Toolbar into transport-controls zone + state-badge zone
     (§9.2); remove Scene Path/Prefab Path fields and the "Create
     Prefab From Selection" button from it.
2.2. Add the Save-Scene-As and Create-Prefab-From-Selection modals
     (§15.4), wired from the File menu (§9.1) and Hierarchy context menu
     (§9.4.2) respectively.
2.3. Implement the Status Bar panel (§9.8) as a new, small, non-dockable
     window.
2.4. Retire the Milestone-numbered info window (§9.9), relocating its
     content per that section's table.

**Acceptance**: Toolbar shows only transport controls + state badge;
Status Bar shows scene name/selection count/play state/FPS; no
standalone "Molly Editor - Milestone N" window exists anymore; Save
Scene As and Create Prefab From Selection both work via modals reachable
from File menu / Hierarchy context menu respectively.

### Phase 3 — Menu Bar Completion

3.1. Add Edit menu (disabled Undo/Redo placeholders, §9.1).
3.2. Add Window menu (Reset Layout + per-panel show/hide checkboxes,
     §9.1, §16.4).
3.3. Add Help menu (About modal, §9.1).
3.4. Wire the keyboard shortcuts table (§15.5) alongside their
     corresponding menu items.

**Acceptance**: all four menus (File/Edit/Window/Help) present and
functional per §9.1's table; "Reset Layout" correctly rebuilds the
default dock layout from any mangled state; closing a panel via its
window-close (×) button and re-checking it in the Window menu correctly
reopens it in its last-known (or default, if never moved) position.

### Phase 4 — Visual Theme Pass

4.1. Implement the full custom `ImGuiStyle` color table (Appendix B/C)
     in `ImGuiLayer::onAttach()`, replacing the bare
     `ImGui::StyleColorsDark()` + single `WindowRounding` tweak.
4.2. Apply the spacing/rounding table (§10.4).
4.3. Load and wire the UI font + monospace Console font (§11), including
     vendoring the two `.ttf` files under `editor/assets/fonts/`.
4.4. Load and wire the icon font (§12), including vendoring its `.ttf`
     file and defining the codepoint-constant table backing §12.2's
     icon-assignment list.

**Acceptance**: editor visually uses the accent-blue palette (Appendix
C) consistently across selection highlights/active tabs/Play button/
Playing-state badge; UI text renders in the real TTF font, not ImGui's
bitmap default; Console log lines render in the monospace font; icons
from §12.2's table appear in Hierarchy rows, Inspector headers, Content
Browser entries, Console filter checkboxes, and Toolbar transport
buttons.

### Phase 5 — Per-Panel Polish

5.1. Hierarchy: real parent/child tree rendering (§9.4.1), context menu
     (§9.4.2), row icons (§9.4.3), accent-colored selection (§9.4.4).
5.2. Inspector: component header icons (§9.5.1), "Remove Component"
     context menu (§9.5.2), full-width fields (§9.5.3), restyled entity
     header block (§9.5.4).
5.3. Viewport: Play-mode border tint (§9.3.1), corner overlay text
     (§9.3.2), empty-state message (§9.3.3).
5.4. Content Browser: breadcrumb bar (§9.6.1), file-type icons (§9.6.2),
     double-click-to-open for scripts (§9.6.3).
5.5. Console: colored filter checkboxes (§9.7.1), unread-count tab
     badge (§9.7.2), click-to-copy log lines (§9.7.3).

**Acceptance**: each sub-item's specific behavior verified individually
(this phase is naturally the largest and most parallelizable — sub-items
5.1–5.5 don't depend on each other and can land as separate, independently
testable changes).

### Phase 6 — Window Chrome

6.1. Design and add an application icon (`.ico`), wire via
     `glfwSetWindowIcon()` (§13.1).
6.2. Set minimum window size via `glfwSetWindowSizeLimits()` (§14.1).
6.3. Title bar scene-name suffix (§13.2).

**Acceptance**: window shows a custom icon (not GLFW's default) in the
title bar and taskbar; window cannot be resized below 1024×720; title
bar reflects the currently loaded scene's filename.

### Phase 7 — Final Verification Pass

7.1. Full manual walkthrough per §20's checklist.
7.2. Fresh-clone verification: delete local `build/` and any local
     `imgui.ini`, rebuild from scratch, confirm Phase 1's acceptance
     criterion still holds end-to-end.
7.3. Regression check: run the existing Catch2 test suite (unaffected by
     this plan's UI-only scope, but confirming no incidental engine-side
     breakage) and manually re-launch `molly-demo.exe` (also unaffected,
     but sharing `ImGuiLayer`/font-loading code paths worth re-verifying).

**Acceptance**: matches this document's own §20 checklist in full;
`ctest` still 100% passing; `molly-demo.exe` still launches and runs
correctly (font/icon loading is shared engine code, not editor-only, so
must not regress the game).

---

## 20. Testing & Verification Plan

Since this is UI/layout work, verification is necessarily
visual/interactive — the existing Catch2 suite (`PLAN.md` §22) doesn't
cover ImGui rendering output (§22.2 explicitly scopes that out, and this
plan doesn't revisit that scoping decision). Verification here means a
structured manual checklist, run at the end of Phase 7 and spot-checked
after each earlier phase:

- [ ] Fresh clone, submodule init, build, first launch of
      `molly-editor.exe`: all five panels visible, correctly positioned
      per §7.1, no overlapping windows, Viewport shows the rendered
      default scene.
- [ ] Resize the main window smaller/larger: dock splitters resize
      panels proportionally, no panel disappears or becomes
      inaccessible above the enforced minimum size (§14.1).
- [ ] Drag the Console tab out of the bottom dock node to split it from
      Content Browser: succeeds, and the new arrangement survives an
      editor restart (persistence, §16).
- [ ] Window menu → "Reset Layout": restores the exact default
      arrangement from §7.1, discarding the previous drag-split from the
      prior checklist item.
- [ ] Window menu → uncheck "Console": Console panel closes; re-check:
      reopens in its prior position.
- [ ] Click Play: Toolbar badge changes to "Playing" (accent-colored),
      Viewport gets the accent-colored border, Status Bar's play-state
      element updates to match.
- [ ] Click Pause while Playing: badge/border switch to the amber
      Paused treatment in both Toolbar and Viewport.
- [ ] Click Stop: returns to Edit-mode styling everywhere consistently
      (no stale Playing/Paused coloring left in any one location).
- [ ] File → New Scene, Save Scene, Open Scene, Save Scene As...: all
      four reachable and functional from the menu (no more toolbar text
      field required for Scene Path).
- [ ] Right-click an entity in Hierarchy: context menu shows
      Rename/Duplicate/Delete/Create Prefab From Selection...; the
      latter opens the modal from §15.4 and successfully writes a
      prefab file.
- [ ] Select an entity with several components: Inspector shows
      per-component icons; right-click a component header shows
      "Remove Component" and it works.
- [ ] Console: Warn/Error checkboxes render in their respective log
      colors; toggling them filters the log list; clicking a log line
      copies it to clipboard (paste somewhere to confirm).
- [ ] Content Browser: breadcrumb bar reflects the current tree
      position; double-clicking a `.lua` file opens it in the system's
      default editor.
- [ ] Window title bar shows a custom icon (not the default) and
      updates to include the loaded scene's filename after Save/Open.
- [ ] `molly-demo.exe` still launches correctly and is visually
      unaffected apart from sharing the new default font (if the font
      loading is engine-level rather than editor-only — confirm which,
      per Phase 4/6 implementation, and that either outcome is
      intentional, not accidental).
- [ ] `ctest` (with `-DMOLLY_BUILD_TESTS=ON`) — 100% pass, unaffected by
      this plan's changes.

---

## 21. Risk Register

| Risk | Likelihood | Impact | Mitigation |
|---|---|---|---|
| `ImGuiDockBuilder` functions live in `imgui_internal.h`, which is explicitly marked by Dear ImGui's authors as "subject to change between versions without notice" | Medium | Low | Pin the ImGui submodule commit as already practiced (`PLAN.md` §6.4); if a future ImGui upgrade breaks the DockBuilder API shape, it's a contained, well-isolated change in one function (`buildDefaultDockLayout`), not scattered through the codebase |
| Icon font + UI font + monospace font all loading correctly together (glyph range merging) is a known fiddly area of ImGui font setup | Medium | Medium | Implement and visually verify font loading *before* wiring the icon-assignment table (§12.2) into every panel, so failures are caught in one isolated step rather than discovered simultaneously across five panels |
| Moving Scene Path/Prefab Path out of always-visible toolbar fields into modals could be seen as *less* discoverable by a user who liked having them always visible | Low | Low | This matches every reference editor's convention (§4), and the File/Hierarchy-context-menu locations are the standard, expected places to look; accepted as the right tradeoff per §6 principle 1 (familiarity over novelty) |
| Recursive Hierarchy tree rendering (§9.4.1) is a small behavior change, not pure styling — risk of a bug in parent/child traversal (e.g. infinite loop if `HierarchyComponent` data is ever malformed with a cycle) | Low | Medium | Nothing in the engine currently creates cycles (parent/child assignment only happens through reviewed, controlled code paths), but the recursive draw function should track visited entities defensively and bail out rather than trust the data unconditionally — a small, cheap safeguard worth including in Phase 5.1's implementation |
| This plan's scope (7 phases, many sub-items) is large enough that partial completion could leave the editor in a visually inconsistent state (e.g. new font but old colors) | Medium | Low | Each phase's acceptance criteria (§19) are independently meaningful — Phase 1 alone already fixes the actual reported bug; later phases are additive polish, and the editor is never *worse* than before at any completed-phase boundary |
| Vendoring font files (§11, §12) adds binary files to the repository, contrary to the general "source, not binaries" preference | Low | Low | Consistent with existing precedent (`floor_checker.png`, audio `.wav` files already vendored the same way per `PLAN.md` Milestones 6/9) — small UI asset files are already an accepted category of committed binary, not a new precedent |

---

## 22. Decision Log

| # | Decision | Reasoning | Section |
|---|---|---|---|
| 1 | Fix via programmatic `ImGuiDockBuilder`, not a checked-in `default_layout.ini` | More robust, code-reviewable, doesn't require hand-tuning a binary-ish ini format; supersedes `PLAN.md` §18.2's original (deferred, never attempted) ini-file idea | §8, §16.2 |
| 2 | Target layout: Hierarchy left / Viewport center / Inspector right / Content Browser+Console tabbed bottom / Toolbar top / Status bar bottom | Directly matches the Unity/Godot convergent standard the user asked for (§4.5) | §7 |
| 3 | Content Browser and Console share one tabbed dock node rather than splitting side by side | More robust on narrower windows than a permanent side-by-side split (§7.3) | §7.3 |
| 4 | One accent color (`#3D8BFF`-ish blue), used for every "selected/active/primary" UI element without exception | Matches how every reference editor uses exactly one consistent highlight color; avoids the current "everything uses ImGui's default blue-gray, nothing stands out" problem | §10.3 |
| 5 | Retire the Milestone-numbered info window entirely, in favor of a real Status Bar | The info window read as a development-process leftover, not user-facing UI, and directly contributed to the screenshot's clutter | §9.9 |
| 6 | Move Scene Path / Prefab Path fields out of the permanently-visible Toolbar into File-menu and Hierarchy-context-menu modals respectively | Matches standard "Save As" / "act on selection" interaction conventions in every reference editor; also directly reduces Toolbar clutter, a contributing factor in the screenshot | §9.2, §9.4.2, §15.4 |
| 7 | Add a real TTF UI font + separate monospace Console font + icon font, vendored as small binary files | Single largest lever for "looks like a finished tool, not an ImGui demo," consistent with existing precedent for vendoring small binary assets | §11, §12 |
| 8 | `imgui.ini` stays gitignored (no change) | With the DockBuilder fix, a missing ini is the *correct* trigger for the good default layout, not a problem to route around | §16.2, §16.3 |
| 9 | No drag-and-drop, no undo/redo, no gizmos, no multi-select in this plan | All explicitly out of scope per `PLAN.md` (undo/redo, gizmos) or flagged as natural future follow-ups this plan doesn't attempt (drag-and-drop, multi-select) — keeps this plan focused on the layout/styling problem actually reported | §5.2, §15.2 |
| 10 | Minimum window size enforced at 1024×720 | Below this, the standard five-panel layout can't usefully display; matches practical minimums in reference editors | §14.1 |

---

## 23. Appendix A — DockBuilder Reference Implementation

Illustrative implementation of `buildDefaultDockLayout()`, matching the
split sequence from §8.3. Intended to live in `EditorLayer.cpp` (or a
small new `editor/src/DockLayout.h`/`.cpp` pair if it grows further),
called from `EditorLayer::onImGuiRender()`.

```cpp
// editor/src/DockLayout.h
#pragma once

#include <imgui.h>

namespace molly {

// Splits `dockspaceId`'s region into the standard Hierarchy / Viewport /
// Inspector / (Content Browser + Console) quadrant layout and docks each
// panel's window by name. Safe to call repeatedly — always tears down
// and rebuilds from scratch, so it doubles as the "Reset Layout" action.
void buildDefaultDockLayout(ImGuiID dockspaceId, const ImVec2& size);

// Returns true if `dockspaceId` has no existing dock structure — i.e.
// nothing was restored from imgui.ini, so the caller should build the
// default layout via buildDefaultDockLayout().
bool dockspaceNeedsDefaultLayout(ImGuiID dockspaceId);

}  // namespace molly
```

```cpp
// editor/src/DockLayout.cpp
#include "DockLayout.h"

#include <imgui_internal.h>  // ImGuiDockBuilder family — see PLAN.md UI
                              // plan §21's risk note on this header's
                              // stability guarantees.

namespace molly {

bool dockspaceNeedsDefaultLayout(ImGuiID dockspaceId) {
    ImGuiDockNode* node = ImGui::DockBuilderGetNode(dockspaceId);
    return node == nullptr || (!node->IsSplitNode() && node->Windows.Size == 0);
}

void buildDefaultDockLayout(ImGuiID dockspaceId, const ImVec2& size) {
    ImGui::DockBuilderRemoveNode(dockspaceId);
    ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspaceId, size);

    // 1) Root -> left 18% (Hierarchy) + remainder
    ImGuiID dockMainArea = dockspaceId;
    ImGuiID dockHierarchy =
        ImGui::DockBuilderSplitNode(dockMainArea, ImGuiDir_Left, 0.18f, nullptr, &dockMainArea);

    // 2) Remainder -> right 22% (Inspector) + remainder
    ImGuiID dockInspector =
        ImGui::DockBuilderSplitNode(dockMainArea, ImGuiDir_Right, 0.22f, nullptr, &dockMainArea);

    // 3) Remainder -> bottom 30% (Content Browser + Console) + remainder (Viewport)
    ImGuiID dockBottom =
        ImGui::DockBuilderSplitNode(dockMainArea, ImGuiDir_Down, 0.30f, nullptr, &dockMainArea);
    ImGuiID dockViewport = dockMainArea;

    // Assign each panel's window name (must match its ImGui::Begin(...) call exactly).
    ImGui::DockBuilderDockWindow("Hierarchy", dockHierarchy);
    ImGui::DockBuilderDockWindow("Viewport", dockViewport);
    ImGui::DockBuilderDockWindow("Inspector", dockInspector);
    ImGui::DockBuilderDockWindow("Content Browser", dockBottom);
    ImGui::DockBuilderDockWindow("Console", dockBottom);

    ImGui::DockBuilderFinish(dockspaceId);
}

}  // namespace molly
```

Call site, inside `EditorLayer::onImGuiRender()`, replacing the current
bare `DockSpaceOverViewport` call:

```cpp
constexpr float kToolbarHeight = 36.0f;
constexpr float kStatusBarHeight = 24.0f;

ImGuiViewport* viewport = ImGui::GetMainViewport();
ImVec2 dockspacePos = ImVec2(viewport->Pos.x, viewport->Pos.y + kToolbarHeight);
ImVec2 dockspaceSize =
    ImVec2(viewport->Size.x, viewport->Size.y - kToolbarHeight - kStatusBarHeight);

ImGuiID dockspaceId = ImGui::GetID("MollyMainDockspace");

if (dockspaceNeedsDefaultLayout(dockspaceId)) {
    buildDefaultDockLayout(dockspaceId, dockspaceSize);
}

ImGui::SetNextWindowPos(dockspacePos);
ImGui::SetNextWindowSize(dockspaceSize);
ImGui::DockSpace(dockspaceId, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);
```

(Note: this switches from the convenience wrapper
`ImGui::DockSpaceOverViewport()` to the lower-level `ImGui::DockSpace()`
call directly, since the convenience wrapper doesn't expose the explicit
position/size inset needed to reserve room for the Toolbar/Status Bar
strips per §8.4 — the wrapper is a thin helper around exactly this
pattern, so this is a well-supported, ordinary usage of the underlying
API, not a hack.)

"Reset Layout" menu item handler (§9.1, §16.4):

```cpp
if (ImGui::MenuItem("Reset Layout")) {
    m_resetLayoutRequested = true;
}

// ...later, in onImGuiRender(), before/instead of the needsDefaultLayout check:
if (m_resetLayoutRequested) {
    buildDefaultDockLayout(dockspaceId, dockspaceSize);
    m_resetLayoutRequested = false;
}
```

---

## 24. Appendix B — Style/Theme Reference Implementation

Illustrative `ImGuiLayer::onAttach()` replacement for the current bare
`ImGui::StyleColorsDark()` + single rounding tweak, implementing §10's
color/spacing table. Color constants reference Appendix C's table.

```cpp
void ImGuiLayer::applyMollyTheme() {
    ImGui::StyleColorsDark();  // start from the dark baseline, override selectively

    ImGuiStyle& style = ImGui::GetStyle();

    // Spacing & rounding (§10.4)
    style.WindowRounding = 6.0f;
    style.FrameRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 4.0f;
    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(6, 4);
    style.ItemSpacing = ImVec2(8, 6);
    style.ScrollbarSize = 12.0f;

    ImVec4* colors = style.Colors;

    // Backgrounds (Appendix C: kBgDarkest / kBgDark / kBgMedium)
    colors[ImGuiCol_WindowBg]          = ImVec4(0.102f, 0.110f, 0.129f, 1.00f);  // kBgDark
    colors[ImGuiCol_ChildBg]           = ImVec4(0.102f, 0.110f, 0.129f, 1.00f);
    colors[ImGuiCol_PopupBg]           = ImVec4(0.086f, 0.094f, 0.110f, 0.98f);  // kBgDarkest
    colors[ImGuiCol_MenuBarBg]         = ImVec4(0.086f, 0.094f, 0.110f, 1.00f);
    colors[ImGuiCol_TitleBg]           = ImVec4(0.086f, 0.094f, 0.110f, 1.00f);
    colors[ImGuiCol_TitleBgActive]     = ImVec4(0.122f, 0.133f, 0.157f, 1.00f);  // kBgMedium

    // Frames (inputs, checkboxes, etc.)
    colors[ImGuiCol_FrameBg]           = ImVec4(0.145f, 0.157f, 0.184f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]    = ImVec4(0.180f, 0.196f, 0.231f, 1.00f);
    colors[ImGuiCol_FrameBgActive]     = ImVec4(0.212f, 0.231f, 0.271f, 1.00f);

    // Accent (Appendix C: kAccent = #3D8BFF)
    ImVec4 accent      = ImVec4(0.239f, 0.545f, 1.000f, 1.00f);
    ImVec4 accentHover = ImVec4(0.349f, 0.616f, 1.000f, 1.00f);
    ImVec4 accentDim   = ImVec4(0.239f, 0.545f, 1.000f, 0.60f);

    colors[ImGuiCol_CheckMark]         = accent;
    colors[ImGuiCol_SliderGrab]        = accent;
    colors[ImGuiCol_SliderGrabActive]  = accentHover;
    colors[ImGuiCol_Button]            = ImVec4(0.157f, 0.169f, 0.196f, 1.00f);
    colors[ImGuiCol_ButtonHovered]     = accentDim;
    colors[ImGuiCol_ButtonActive]      = accent;
    colors[ImGuiCol_Header]            = accentDim;               // selected tree/list row
    colors[ImGuiCol_HeaderHovered]     = ImVec4(0.239f, 0.545f, 1.000f, 0.35f);
    colors[ImGuiCol_HeaderActive]      = accent;
    colors[ImGuiCol_Tab]               = ImVec4(0.114f, 0.125f, 0.145f, 1.00f);
    colors[ImGuiCol_TabHovered]        = accentDim;
    colors[ImGuiCol_TabActive]         = ImVec4(0.157f, 0.169f, 0.196f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive]= ImVec4(0.130f, 0.141f, 0.165f, 1.00f);
    colors[ImGuiCol_DockingPreview]    = accentDim;
    colors[ImGuiCol_ResizeGrip]        = ImVec4(0.239f, 0.545f, 1.000f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] = accentDim;
    colors[ImGuiCol_ResizeGripActive]  = accent;
    colors[ImGuiCol_SeparatorHovered]  = accentDim;
    colors[ImGuiCol_SeparatorActive]   = accent;

    // Text
    colors[ImGuiCol_Text]              = ImVec4(0.906f, 0.910f, 0.918f, 1.00f);
    colors[ImGuiCol_TextDisabled]      = ImVec4(0.502f, 0.514f, 0.541f, 1.00f);
}
```

Font loading, also in `onAttach()`, after context creation:

```cpp
ImGuiIO& io = ImGui::GetIO();

io.Fonts->AddFontFromFileTTF("assets/fonts/Inter-Regular.ttf", 16.0f);

ImFontConfig iconConfig;
iconConfig.MergeMode = true;
iconConfig.PixelSnapH = true;
static const ImWchar iconRanges[] = { 0xE000, 0xF8FF, 0 };  // private-use area, matches the icon font's mapping
io.Fonts->AddFontFromFileTTF("assets/fonts/MollyIcons.ttf", 16.0f, &iconConfig, iconRanges);

// Separate, non-merged font for the Console panel specifically:
s_monoFont = io.Fonts->AddFontFromFileTTF("assets/fonts/JetBrainsMono-Regular.ttf", 14.0f);
```

(`s_monoFont` stored as a small static/member `ImFont*`, pushed via
`ImGui::PushFont(s_monoFont)` / `PopFont()` around `ConsolePanel`'s
log-line rendering loop, per §11.2.)

---

## 25. Appendix C — Color Palette Table

All values given as both hex (for design-tool reference) and normalized
float RGBA (for direct use in `ImVec4`, matching Appendix B's code).

| Name | Hex | RGBA (float) | Usage |
|---|---|---|---|
| `kBgDarkest` | `#161821` | (0.086, 0.094, 0.129, 1.0) | Menu bar, popups, title bar |
| `kBgDark` | `#1A1C21` | (0.102, 0.110, 0.129, 1.0) | Main panel backgrounds |
| `kBgMedium` | `#1F2228` | (0.122, 0.133, 0.157, 1.0) | Active title bar, elevated surfaces |
| `kFrameBg` | `#252832` | (0.145, 0.157, 0.196, 1.0) | Input fields, checkboxes (idle) |
| `kFrameBgHover` | `#2E323B` | (0.180, 0.196, 0.231, 1.0) | Input fields (hovered) |
| `kFrameBgActive` | `#363B45` | (0.212, 0.231, 0.271, 1.0) | Input fields (active/focused) |
| `kAccent` | `#3D8BFF` | (0.239, 0.545, 1.000, 1.0) | Selection, active tabs, Play button, primary actions — the one accent color (§10.3) |
| `kAccentHover` | `#599DFF` | (0.349, 0.616, 1.000, 1.0) | Accent-colored elements, hovered state |
| `kAccentDim` | `#3D8BFF` @ 60% | (0.239, 0.545, 1.000, 0.6) | Accent-colored elements, dim/inactive-but-relevant state |
| `kWarnAmber` | `#E6B84D` | (0.902, 0.722, 0.302, 1.0) | Paused-state badge/border, Warn-severity log color/icon |
| `kErrorRed` | `#F25555` | (0.949, 0.333, 0.333, 1.0) | Error/Critical-severity log color/icon |
| `kInfoBlueGray` | `#8FA3BF` | (0.561, 0.639, 0.749, 1.0) | Info-severity log color/icon (distinct from `kAccent` to avoid overloading the one true accent, §10.3) |
| `kTextPrimary` | `#E7E8EA` | (0.906, 0.910, 0.918, 1.0) | Default text |
| `kTextDisabled` | `#80838A` | (0.502, 0.514, 0.541, 1.0) | Disabled/placeholder text |
| `kTextTrace` | `#6B6E76` | (0.420, 0.431, 0.463, 1.0) | Trace-severity log lines (deliberately the least prominent) |

---

## 26. Appendix D — ASCII Layout Mockups

### 26.1 Default Layout (Edit Mode)

```
┌────────────────────────────────────────────────────────────────────────┐
│ File   Edit   Window   Help                                             │
├────────────────────────────────────────────────────────────────────────┤
│ ▶            Molly Editor                              [Edit Mode]      │
├───────────┬──────────────────────────────────────────┬─────────────────┤
│ Hierarchy │                                            │  Inspector      │
│           │                                            │                 │
│ ▸ Floor    │                                            │  Player          │
│ ▸ WallNorth │                                            │  ─────────       │
│ ▸ WallSouth │              [ 3D Viewport ]                │  ▤ Transform      │
│ ▾ Player     │                                            │   Position ...     │
│   (camera)    │                RMB+WASD to fly              │  ▤ Health           │
│ ▸ WaveManager  │                                            │   100 / 100          │
│ ▸ Sun           │                                            │  ▤ Script              │
│ ▸ MainCamera     │                                            │   player.lua             │
│                   │                                            │                            │
│                    │                                            │  [+ Add Component]           │
├───────────┴──────────────────────────────────────────┴─────────────────┤
│ [Content Browser] [Console (2)]                                          │
│ assets ▸ scripts ▸                                                        │
│  📄 player.lua      📄 enemy_chaser.lua      📄 wave_manager.lua            │
├────────────────────────────────────────────────────────────────────────┤
│ Ready │ Scene: arena_main.scene.json │ 1 selected │ Edit Mode │ 60.1 FPS   │
└────────────────────────────────────────────────────────────────────────┘
```

### 26.2 Playing Mode (note the accent-tinted Viewport border and badge)

```
┌────────────────────────────────────────────────────────────────────────┐
│ File   Edit   Window   Help                                             │
├────────────────────────────────────────────────────────────────────────┤
│  ⏸  ■                Molly Editor                       ┃PLAYING┃       │
├───────────┬══════════════════════════════════════════┬─────────────────┤
│ Hierarchy ║                                            ║  Inspector      │
│           ║                                            ║                 │
│ ▸ Floor    ║          [ 3D Viewport — accent-blue        ║  Player          │
│ ▸ Player    ║           border while Playing ]              ║  (live values,     │
│ ▸ Enemy      ║                                            ║   updating)          │
│ ▸ Enemy       ║                                            ║                       │
├───────────┴══════════════════════════════════════════┴─────────────────┤
│ [Content Browser] [Console (5)]                                          │
├────────────────────────────────────────────────────────────────────────┤
│ Ready │ Scene: arena_main.scene.json │ 1 selected │ Playing │ 59.8 FPS    │
└────────────────────────────────────────────────────────────────────────┘
```

(`═`/`║` characters above are a plain-text stand-in for the 2–3px
accent-colored border treatment described in §9.3.1 — not a literal
double-line-box-drawing UI element.)

### 26.3 The Screenshot's Actual (Broken) State, For Contrast

```
┌────────────────────────────────────────────────────────────────────────┐
│ File                                                                     │
├────────────────────────────────────────────────────────────────────────┤
│┌────────────────────┐                                                    │
││▼Console             │                                                    │
││[Clear][ ][x][x][x]  │  ← Content Browser + Toolbar remnants visible       │
││ search...            │    only as slivers behind/under the Console        │
││ ▸prefabs ▸scripts     │                                                    │
││ ▸shaders ▸textures     │                                                    │
│└────────────────────┘                                                    │
│                                                                            │
│                                                                            │
│                    (everything else: solid black — Viewport,               │
│                     Hierarchy, and Inspector are all somewhere              │
│                     off-screen or fully occluded, per §2)                    │
│                                                                            │
└────────────────────────────────────────────────────────────────────────┘
```

---

## 27. Appendix E — Panel Window-Flag Reference

Exact `ImGuiWindowFlags` intended for each panel/strip after this plan's
changes, since several (Toolbar, Status Bar) are explicitly *not*
dockable and need specific flag combinations to stay pinned in place:

| Window | Flags |
|---|---|
| Toolbar | `ImGuiWindowFlags_NoDocking \| ImGuiWindowFlags_NoTitleBar \| ImGuiWindowFlags_NoResize \| ImGuiWindowFlags_NoMove \| ImGuiWindowFlags_NoScrollbar \| ImGuiWindowFlags_NoSavedSettings` |
| Status Bar | Same combination as Toolbar |
| Hierarchy | Default dockable window flags (none special) |
| Viewport | `ImGuiWindowFlags_NoScrollbar \| ImGuiWindowFlags_NoScrollWithMouse` (prevents accidental panel scrolling while the user is actually trying to interact with the 3D view) |
| Inspector | Default dockable window flags |
| Content Browser | Default dockable window flags |
| Console | Default dockable window flags |
| Save Scene As / Create Prefab modals | `ImGuiWindowFlags_AlwaysAutoResize \| ImGuiWindowFlags_NoResize \| ImGuiWindowFlags_NoCollapse`, opened via `ImGui::OpenPopup()` + `BeginPopupModal()` |
| About modal | Same as the Save/Create modals above |

---

## 28. Appendix F — Before / After Comparison

| Aspect | Before (screenshot state) | After (this plan) |
|---|---|---|
| First-launch layout | Cascaded, overlapping floating windows, mostly black empty window | Full five-panel quadrant layout, immediately usable |
| Viewport visibility | Not visible / occluded | Dominant, central, always visible by default |
| Menu bar | "File" only, 3 items | File / Edit / Window / Help, ~15 items total |
| Toolbar contents | Play/Pause/Stop + Scene Path + Prefab Path + Create-Prefab button, all mixed | Play/Pause/Stop + state badge only; other actions moved to menu/context-menu/modals |
| Status feedback | Buried in a Milestone-numbered debug window | Dedicated Status Bar with transient messages |
| Accent color | None (ImGui default blue-gray throughout) | One consistent accent blue, used everywhere selection/active/primary state is shown |
| Font | ImGui bitmap default | Real TTF UI font + dedicated monospace Console font |
| Icons | None | Icon font covering entities, components, files, log severities, transport controls |
| Play-mode awareness | Small text label only | Accent-tinted Viewport border + colored Toolbar/Status-Bar badges |
| Hierarchy structure | Flat list, ignores actual parent/child data for display | Real recursive tree matching `HierarchyComponent` |
| Window icon | GLFW default | Custom application icon |
| Minimum window size | Unbounded (can shrink to unusable) | Enforced 1024×720 floor |
| Layout recovery | None — a mangled layout stays mangled until `imgui.ini` is manually deleted | "Reset Layout" menu action |

---

## 29. Appendix G — Toolbar & Status Bar Reference Implementation

Illustrative implementation of the reworked Toolbar (§9.2) and new
Status Bar (§9.8), showing the `NoDocking`/pinned-strip flag combination
from Appendix E in context.

```cpp
void EditorLayer::drawToolbar() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    constexpr float kToolbarHeight = 36.0f;

    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + kMenuBarHeight));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, kToolbarHeight));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                              ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                              ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
    ImGui::Begin("##Toolbar", nullptr, flags);

    // --- Left zone: transport controls ---
    bool playing = m_playState != EditorPlayState::Edit;
    if (!playing) {
        if (ImGui::Button(ICON_PLAY " Play")) onPlay();
    } else {
        if (ImGui::Button(ICON_STOP)) onStop();
        ImGui::SameLine();
        const char* pauseIcon = m_playState == EditorPlayState::Paused ? ICON_PLAY : ICON_PAUSE;
        if (ImGui::Button(pauseIcon)) onPause();
    }

    // --- Right zone: state badge, right-aligned ---
    const char* badgeText = m_playState == EditorPlayState::Edit      ? "Edit Mode"
                             : m_playState == EditorPlayState::Playing ? "Playing"
                                                                       : "Paused";
    ImVec4 badgeColor = m_playState == EditorPlayState::Edit      ? ImVec4(0.3f, 0.32f, 0.36f, 1.0f)
                        : m_playState == EditorPlayState::Playing ? kAccentColor
                                                                    : kWarnAmberColor;

    float badgeWidth = ImGui::CalcTextSize(badgeText).x + 24.0f;
    ImGui::SameLine(ImGui::GetWindowWidth() - badgeWidth - 12.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, badgeColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, badgeColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, badgeColor);
    ImGui::Button(badgeText, ImVec2(badgeWidth, 0));  // non-interactive; a Button here purely for the
                                                        // colored-pill visual, not for click behavior
    ImGui::PopStyleColor(3);

    ImGui::End();
}
```

```cpp
void EditorLayer::drawStatusBar() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    constexpr float kStatusBarHeight = 24.0f;

    ImGui::SetNextWindowPos(
        ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - kStatusBarHeight));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, kStatusBarHeight));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                              ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                              ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
    ImGui::Begin("##StatusBar", nullptr, flags);

    // Transient message (§9.8) if one is still within its ~2s display window, else "Ready".
    std::string leftText = m_statusMessageTimer > 0.0f ? m_statusMessage : "Ready";
    ImGui::TextUnformatted(leftText.c_str());

    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    ImGui::Text("Scene: %s", m_currentSceneName.empty() ? "untitled" : m_currentSceneName.c_str());

    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    ImGui::Text("%d selected", m_hierarchyPanel.getSelected() ? 1 : 0);

    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    const char* stateLabel = m_playState == EditorPlayState::Edit      ? "Edit Mode"
                              : m_playState == EditorPlayState::Playing ? "Playing"
                                                                        : "Paused";
    ImGui::TextUnformatted(stateLabel);

    // FPS, right-aligned
    char fpsText[32];
    snprintf(fpsText, sizeof(fpsText), "%.1f FPS", ImGui::GetIO().Framerate);
    float fpsWidth = ImGui::CalcTextSize(fpsText).x;
    ImGui::SameLine(ImGui::GetWindowWidth() - fpsWidth - 12.0f);
    ImGui::TextUnformatted(fpsText);

    ImGui::End();
}
```

The transient-message mechanism (`m_statusMessage` / `m_statusMessageTimer`)
is set from wherever a confirmation-worthy action completes — e.g. inside
`saveScene()`:

```cpp
void EditorLayer::saveScene() {
    // ... existing SceneSerializer::serialize(...) call ...
    m_statusMessage = "Scene saved: " + m_scenePathBuffer;
    m_statusMessageTimer = 2.0f;  // seconds; ticked down in onUpdate()
}
```

---

## 30. Appendix H — Recursive Hierarchy Tree Reference Implementation

Illustrative replacement for `SceneHierarchyPanel`'s current flat
`registry.view<TagComponent>()` loop (§9.4.1), including the defensive
cycle guard called out in the Risk Register (§21).

```cpp
void SceneHierarchyPanel::drawEntityNode(entt::entity entity,
                                          std::unordered_set<entt::entity>& visited) {
    if (!visited.insert(entity).second) {
        // Already visited this frame — malformed/cyclic parent-child data.
        // Bail rather than trust it (Risk Register, EDITOR_UI_PLAN.md §21).
        return;
    }

    auto& registry = m_context->getRegistry();
    auto& tag = registry.get<TagComponent>(entity);
    auto* hierarchy = registry.try_get<HierarchyComponent>(entity);

    bool hasChildren = hierarchy && !hierarchy->children.empty();
    bool isSelected = m_selected == Entity(entity, m_context);

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (isSelected) flags |= ImGuiTreeNodeFlags_Selected;
    if (!hasChildren) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

    ImGui::PushID(static_cast<int>(entt::to_integral(entity)));

    const char* icon = iconForEntity(registry, entity);  // §9.4.3 / §12.2 lookup table
    bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<uintptr_t>(entity)),
                                     flags, "%s %s", icon, tag.name.c_str());

    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        m_selected = Entity(entity, m_context);
    }

    drawEntityContextMenu(Entity(entity, m_context));  // §9.4.2

    if (hasChildren && opened) {
        for (auto child : hierarchy->children) {
            drawEntityNode(child, visited);
        }
        ImGui::TreePop();
    }

    ImGui::PopID();
}

void SceneHierarchyPanel::onImGuiRender() {
    ImGui::Begin("Hierarchy");
    if (!m_context) {
        ImGui::TextDisabled("No active scene");
        ImGui::End();
        return;
    }

    drawToolbarRow();  // "Create Entity" button, moved to its own row above the tree

    std::unordered_set<entt::entity> visited;
    auto& registry = m_context->getRegistry();
    for (auto entity : registry.view<TagComponent, HierarchyComponent>()) {
        auto& hierarchy = registry.get<HierarchyComponent>(entity);
        if (hierarchy.parent == entt::null) {
            drawEntityNode(entity, visited);  // only draw roots; children are drawn recursively
        }
    }

    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
        !ImGui::IsAnyItemHovered()) {
        m_selected = {};
    }

    ImGui::End();
}
```

---

## 31. Glossary

- **Dockspace** — the ImGui concept of a region of screen space that
  panels ("windows," in ImGui's terminology) can be docked into, split,
  and tabbed within. Created once per frame via `ImGui::DockSpace()`.
- **Dock node** — a single rectangular region within a dockspace, either
  a leaf (containing one or more tabbed windows) or a split (containing
  two child nodes). §8.3's five named nodes (`dock_hierarchy`,
  `dock_viewport`, etc.) are all leaf nodes after the split sequence
  completes.
- **`ImGuiDockBuilder`** — the internal (non-public-API,
  `imgui_internal.h`) family of functions used to construct a dock
  layout programmatically in code, rather than relying on the user
  manually dragging windows to dock them. See §21's risk note on this
  header's stability guarantees.
- **`imgui.ini`** — ImGui's default persistence file, storing window
  positions, sizes, dock assignments, and a handful of other
  per-window settings, written on exit and read on startup. See §16.
- **Transport controls** — the Play/Pause/Stop button group, named by
  analogy to physical media transport controls (VCR/tape deck play/
  pause/stop buttons) — standard terminology across creative/dev tools,
  not Molly-specific.
- **Accent color** — the single color used consistently to indicate
  selection, active state, or primary actions throughout the UI (§10.3,
  `kAccent` in Appendix C) — as opposed to a full multi-color palette
  where different UI regions might otherwise pick arbitrary, inconsistent
  highlight colors.
- **Icon font** — a font file where each "character" glyph is a small
  icon image rather than a letter, allowing icons to be drawn via normal
  text-rendering calls (`ImGui::Text()`, etc.) and therefore to inherit
  ImGui's normal text-coloring behavior (§12.1).
- **Glyph range** — the set of Unicode codepoints a given font provides
  glyphs for; used in Appendix B's font-loading code to tell ImGui which
  codepoints the icon font's glyphs are mapped to, so they can be merged
  into the main UI font's lookup table.

---

## 32. Frequently Revisited Questions

**Q: Why not just ship a checked-in `imgui.ini` like `PLAN.md` §18.2
originally suggested, instead of writing DockBuilder code?**
A: A checked-in ini file only fixes the *first* launch — the moment any
user (or any future panel addition) causes ImGui to need to place a
window it doesn't have a saved position for, the exact same
cascade-and-overlap failure mode returns, just for one panel instead of
all seven. The DockBuilder function is the fix that keeps working as the
editor grows new panels, since it's driven by a maintained list of
"where does each named panel go," not a frozen snapshot of one specific
session's layout. See §8, §16.2.

**Q: Doesn't hardcoding panel window-name strings (`"Hierarchy"`,
`"Viewport"`, etc.) in `buildDefaultDockLayout()` create a fragile
coupling to each panel's exact `ImGui::Begin()` call?**
A: Yes, deliberately — this is the same kind of "string constant must
match exactly" coupling that already exists throughout the codebase
(e.g. `SceneSerializer`'s component-name JSON keys, `PLAN.md` §37). The
mitigation is the same as elsewhere: keep the panel window-title strings
as named constants (not inline string literals duplicated in two places)
once this plan reaches implementation, so a rename only needs to happen
in one place.

**Q: Why is the accent color a blue between Unity's and Godot's, instead
of just picking one of theirs directly?**
A: Matching a specific product's exact brand color would read as
imitation rather than "genre-appropriate default," and could be
confusing (a user seeing Unity's exact blue might reasonably assume some
deeper connection or asset-compatibility that doesn't exist). A distinct
but genre-consistent blue gets the "looks like a professional engine
tool" outcome the user asked for without that specific confusion. See
§10.2.

**Q: Should the Toolbar and Status Bar eventually become dockable too,
instead of being permanently pinned strips?**
A: No — this matches every reference editor (§4), where transport
controls and status information are *always* anchored in a fixed,
predictable location specifically so the user never has to hunt for
them or accidentally dock them somewhere awkward. Pinning them outside
the dockspace (via `ImGuiWindowFlags_NoDocking`, Appendix E) is a
deliberate, permanent design choice, not a placeholder.

**Q: This plan adds several binary asset files (fonts, an icon). Does
that conflict with the project's general preference for
source-over-binary?**
A: No — `PLAN.md` already established the precedent that small,
necessary binary assets (a placeholder texture, placeholder audio
clips) are vendored directly when there's no reasonable source-code
alternative, and UI/icon fonts fall in exactly that category. See §21's
risk-register entry on this specific point.

---

*End of plan. Like `PLAN.md`, this document is expected to evolve as
implementation proceeds — update sections as decisions are revisited,
rather than treating this as a frozen spec.*
