#include <molly/Molly.h>

#include "GameLayer.h"

class GameApplication : public molly::Application {
public:
    GameApplication() : molly::Application("Molly Demo") { pushLayer(new molly::GameLayer()); }
};

int main() {
    molly::setWorkingDirectoryToExecutable();
    molly::Log::init();
    MOLLY_INFO("Molly Demo starting up");

    GameApplication app;
    app.run();

    return 0;
}
