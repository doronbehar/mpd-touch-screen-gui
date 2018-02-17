#include <libwtk-sdl2/utf8.hpp>
#include "util.hpp"
#include "keypad.hpp"

keypad::keypad(vec size, std::string keys, std::function<void(std::string)> submit_callback)
    : keypad(size, construct(size, keys, submit_callback))
{
}

keypad::keypad(vec size, std::tuple<std::shared_ptr<button>, std::vector<table::entry>> tmp)
    : embedded_widget<table>(vec{ std::max(size.w, 3), std::max(size.h, 0) + 1 }, std::get<1>(tmp), 5)
    , _submit_button(std::get<0>(tmp))
{
}

keypad::~keypad()
{
}

void keypad::remove_last()
{
    if (!_search_term.empty())
    {
        int const len = count_utf8_backwards(_search_term.c_str() + _search_term.size() - 1);
        _search_term.resize(_search_term.size() - len);
        update();
    }
}

void keypad::update()
{
    _submit_button->set_label('\'' + _search_term + '\'');
}

void keypad::append(std::string c)
{
    _search_term += c;
    update();
}

void keypad::clear()
{
    _search_term.clear();
    update();
}

std::tuple<std::shared_ptr<button>, std::vector<table::entry>> keypad::construct(vec size, std::string keys, std::function<void(std::string)> submit_callback)
{
    int const num_letter_keys = size.w * size.h;
    char const * letter_ptr = keys.c_str();
    std::vector<table::entry> table_entries;

    auto submit_button = std::make_shared<button>("''", [this, submit_callback]{ submit_callback(this->_search_term); });

    table_entries.reserve(3 + num_letter_keys);

    table_entries.push_back({ { 0, 0, size.w - 2, 1 }, submit_button });
    table_entries.push_back(
        { { size.w - 2, 0, 1, 1}
        , std::make_shared<button>("⌫", [this]{ remove_last(); })
        });
    table_entries.push_back(
        { { size.w - 1, 0, 1, 1}
        , std::make_shared<button>("⌧", [this]{ clear(); })
        });

    {
        int y = 1;
        int x = 0;
        for (int i = 0; i < num_letter_keys; ++i)
        {
            if (*letter_ptr == '\0')
                break;
            char buff[8];

            int const num_bytes = fetch_utf8(buff, letter_ptr);

            if (num_bytes == 0)
                break;
            else
            {
                std::string char_utf8(buff, num_bytes);
                table_entries.push_back({ { x, y, 1, 1 }, std::make_shared<button>(char_utf8, [this, char_utf8]{ append(char_utf8); }) });
                letter_ptr += num_bytes;
                x += 1;
            }

            if (i % size.w == size.w - 1)
            {
                y += 1;
                x = 0;
            }
        }
    }
    return std::make_tuple(submit_button, table_entries);
}
