#include <molly/Molly.h>

#include "EditorLayer.h"

class EditorApplication : public molly::Application {
public:
    EditorApplication() : molly::Application("Molly Editor") {
        pushLayer(new molly::EditorLayer());
    }
};

int main() {
    molly::Log::init();
    MOLLY_INFO("Molly Editor starting up");

    EditorApplication app;
    app.run();

    return 0;
}
