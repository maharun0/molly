#include "molly/ui/ImGuiLayer.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "molly/core/Application.h"

namespace molly {

ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer") {}

// A single consistent accent color + dark neutral palette, replacing
// ImGui's stock StyleColorsDark() defaults, per plan/EDITOR_UI_PLAN.md
// §10 and Appendix B/C. Applies engine-wide (both molly-editor and
// molly-demo share this ImGuiLayer) — harmless for the demo's own minor
// debug overlays, and keeps a single source of truth for the theme.
static void applyMollyTheme() {
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 6.0f;
    style.FrameRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 4.0f;
    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(6, 4);
    style.ItemSpacing = ImVec2(8, 6);
    style.ScrollbarSize = 12.0f;

    ImVec4* colors = style.Colors;

    // Backgrounds
    colors[ImGuiCol_WindowBg] = ImVec4(0.102f, 0.110f, 0.129f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.102f, 0.110f, 0.129f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.086f, 0.094f, 0.110f, 0.98f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.086f, 0.094f, 0.110f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.086f, 0.094f, 0.110f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.122f, 0.133f, 0.157f, 1.00f);

    // Frames (inputs, checkboxes, etc.)
    colors[ImGuiCol_FrameBg] = ImVec4(0.145f, 0.157f, 0.196f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.180f, 0.196f, 0.231f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.212f, 0.231f, 0.271f, 1.00f);

    // Accent (kAccent = #3D8BFF)
    ImVec4 accent = ImVec4(0.239f, 0.545f, 1.000f, 1.00f);
    ImVec4 accentHover = ImVec4(0.349f, 0.616f, 1.000f, 1.00f);
    ImVec4 accentDim = ImVec4(0.239f, 0.545f, 1.000f, 0.60f);

    colors[ImGuiCol_CheckMark] = accent;
    colors[ImGuiCol_SliderGrab] = accent;
    colors[ImGuiCol_SliderGrabActive] = accentHover;
    colors[ImGuiCol_Button] = ImVec4(0.157f, 0.169f, 0.196f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = accentDim;
    colors[ImGuiCol_ButtonActive] = accent;
    colors[ImGuiCol_Header] = accentDim;
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.239f, 0.545f, 1.000f, 0.35f);
    colors[ImGuiCol_HeaderActive] = accent;
    colors[ImGuiCol_Tab] = ImVec4(0.114f, 0.125f, 0.145f, 1.00f);
    colors[ImGuiCol_TabHovered] = accentDim;
    colors[ImGuiCol_TabActive] = ImVec4(0.157f, 0.169f, 0.196f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.130f, 0.141f, 0.165f, 1.00f);
    colors[ImGuiCol_DockingPreview] = accentDim;
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.239f, 0.545f, 1.000f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] = accentDim;
    colors[ImGuiCol_ResizeGripActive] = accent;
    colors[ImGuiCol_SeparatorHovered] = accentDim;
    colors[ImGuiCol_SeparatorActive] = accent;

    // Text
    colors[ImGuiCol_Text] = ImVec4(0.906f, 0.910f, 0.918f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.502f, 0.514f, 0.541f, 1.00f);
}

void ImGuiLayer::onAttach() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    applyMollyTheme();

    auto* window = static_cast<GLFWwindow*>(Application::get().getWindow().getNativeWindow());

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460 core");
}

void ImGuiLayer::onDetach() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiLayer::onEvent(Event& event) {
    if (m_blockEvents) {
        ImGuiIO& io = ImGui::GetIO();
        event.handled |= event.isInCategory(EventCategoryMouse) && io.WantCaptureMouse;
        event.handled |= event.isInCategory(EventCategoryKeyboard) && io.WantCaptureKeyboard;
    }
}

void ImGuiLayer::begin() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::end() {
    ImGuiIO& io = ImGui::GetIO();
    auto& window = Application::get().getWindow();
    io.DisplaySize = ImVec2(static_cast<float>(window.getWidth()),
                             static_cast<float>(window.getHeight()));

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

}  // namespace molly
