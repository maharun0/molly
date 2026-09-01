#pragma once

// Umbrella header for consumers of the engine (editor, game). Engine
// internals should include specific headers directly rather than this file.

#include "molly/core/Application.h"
#include "molly/core/Assert.h"
#include "molly/core/Layer.h"
#include "molly/core/Log.h"
#include "molly/core/Time.h"
#include "molly/core/Window.h"

#include "molly/core/Events/Event.h"
#include "molly/core/Events/KeyEvent.h"
#include "molly/core/Events/MouseEvent.h"
#include "molly/core/Events/WindowEvent.h"

#include "molly/renderer/Buffer.h"
#include "molly/renderer/Camera.h"
#include "molly/renderer/Framebuffer.h"
#include "molly/renderer/Light.h"
#include "molly/renderer/Material.h"
#include "molly/renderer/Mesh.h"
#include "molly/renderer/Shader.h"
#include "molly/renderer/Texture.h"
#include "molly/renderer/VertexArray.h"

#include "molly/resource/ModelLoader.h"
