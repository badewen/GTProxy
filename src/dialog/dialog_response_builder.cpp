#include "dialog_response_builder.h"

using namespace dialog;

DialogResponseBuilder::DialogResponseBuilder(const std::string& dialog_request)
    : m_text_inputs {}
    , m_data_embeds {}
    , m_dialog_name {}
{
    for (const auto &raw_dialog_data: utils::TextParse::string_tokenize(dialog_request, "\n")) {
        std::vector<std::string> dialog_data_tokenize = utils::TextParse::string_tokenize(raw_dialog_data);

        if (dialog_data_tokenize.at(0) == "embed_data") {
            m_data_embeds.emplace_back(
                raw_dialog_data.substr(
                        strlen("embed_data") + 1,
                        raw_dialog_data.length() - strlen("embed_data")
                ) + "|"
            );
        } else if (dialog_data_tokenize.at(0) == "add_text_input" ||
                   dialog_data_tokenize.at(0) == "add_checkbox"
                ) {
            m_text_inputs.insert_or_assign(dialog_data_tokenize.at(1), dialog_data_tokenize.at(3));
        } else if (dialog_data_tokenize.at(0) == "end_dialog") {
            m_dialog_name = dialog_data_tokenize.at(1);
        } else if (dialog_data_tokenize.at(0) == "add_button") {
            m_button_list.push_back(dialog_data_tokenize.at(1));
        }
    }
}

void DialogResponseBuilder::input_text(const std::string& input_text_id, const std::string& value) {
    try {
        m_text_inputs.at(input_text_id) = value + "|";
    }
    catch (...) {
        return;
    }
}

std::string DialogResponseBuilder::build() {
    std::string ret {};

    ret += "action|dialog_return\n";
    ret += "dialog_name|" + m_dialog_name + "\n";

    for (const auto& data : m_data_embeds) {
        ret += data.get_all_raw() + "\n";
    }

    if (!m_clicked_button.empty()) {
        ret += "buttonClicked|" + m_clicked_button;
    }

    for (const auto& input : m_text_inputs) {
        ret += input.first + "|" + input.second;
    }

    return ret;

}
