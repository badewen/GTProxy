#include "command_base.h"


// debugging command

namespace commands {
class Sleep : public command::CommandBase {
public:
    Sleep() : command::CommandBase{ {"internal_sleep"}, {}, true } {}

    void execute(client::Client* client, std::vector<std::string> args) override;
};
}
