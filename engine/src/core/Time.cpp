#include "molly/core/Time.h"

#include <GLFW/glfw3.h>

namespace molly {

float Time::getSeconds() { return static_cast<float>(glfwGetTime()); }

}  // namespace molly
