#include "save.h"

#include <fstream>

#include "../client/client.h"

using namespace commands;

void Save::execute(client::Client* client, std::vector<std::string> args) {
    std::string file_name{args[0]};

    std::ofstream out_file {file_name, std::ios::binary};

    out_file.write(reinterpret_cast<char*>(client->get_current_world().data.data()),
                   client->get_current_world().data.size()
                   );

    out_file.close();
}

