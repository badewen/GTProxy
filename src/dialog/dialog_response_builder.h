#pragma once

#include <string>
#include <unordered_map>

#include "../utils/text_parse.h"

namespace dialog {

class DialogResponseBuilder {
public:
    explicit DialogResponseBuilder(const std::string& dialog_request);

    void input_text(const std::string& input_text_id, const std::string& value);

    std::string build();

    void click_button(std::string button_id) {
        m_clicked_button = std::move(button_id);
    }

    void enable_checkbox(const std::string& checkbox_id, bool enable) {
        input_text(checkbox_id, std::to_string(enable));
    }

    std::string get_dialog_name() { return m_dialog_name; }

    bool has_button(const std::string& id) {
        return std::find(m_button_list.begin(), m_button_list.end(), id) != m_button_list.end();
    }
    bool has_text_input(const std::string& id) { return m_text_inputs.find(id) != m_text_inputs.end(); }
    bool has_data_embed(const std::string& embed_data_key) {
        for (const auto& data : m_data_embeds) {
            if (!data.get(embed_data_key, 0).empty())
                return true;
        }
        return false;
    }

private:
    // includes text input and checkbox
    std::unordered_map<std::string, std::string> m_text_inputs;
    std::vector<utils::TextParse> m_data_embeds;
    std::vector<std::string> m_button_list;
    std::string m_clicked_button;
    std::string m_dialog_name;
};
}
