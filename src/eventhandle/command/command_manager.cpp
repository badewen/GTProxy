#include "command_manager.h"

#include "../enet/include/enet/enet.h"
#include "../../network/packet.h"
#include "../../eventmanager/event_manager.h"
#include "../../config.h"
#include "../../utils/text_parse.h"

bool onSendTankPacket(ENetPeer* peer, ENetPacket* packet) {
    player::eNetMessageType message_type{player::get_message_type(packet) };

    std::string message_data{ player::get_text(packet) };
    utils::TextParse text_parse{ message_data };

    if (message_type == player::NET_MESSAGE_GENERIC_TEXT) {
        if (message_data.find("action|input") != std::string::npos) {
            utils::TextParse text_parse{message_data};
            if (text_parse.get("text", 1).empty()) {
                return false;
            }

            std::vector<std::string> token{utils::TextParse::string_tokenize(message_data.substr(
                    message_data.find("text|") + 5,
                    message_data.length() - message_data.find("text|") - 1
            ), " ")};

            if (token[0] != config)

            if (CommandManager::Execute())

        }
    }
}
