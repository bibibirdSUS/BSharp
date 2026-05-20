//
// Created by bibib on 2026/2/28.
//

#include "bs_exception.h"
#include <ranges>
#include <sstream>
#include <utility>
#include "../../utils.h"

bs_exception::bs_exception(std::string msg, std::string source_code, position start,
                           position end,
                           std::vector<stack_frame> call_stack) : msg_(std::move(msg)),
                                                                  source_code_(std::move(source_code)),
                                                                  start_(std::move(start)), end_(std::move(end)),
                                                                  call_stack_(std::move(call_stack)) {
}

const char *bs_exception::what() const noexcept {
    std::stringstream ss;
    ss << type_name() << ": " << msg_ << '\n';

    ss << "Caused by: " << start_.file_name << " from " << start_.line + 1 << ":" << start_.column + 1 
       << " to " << end_.line + 1 << ":" << end_.column + 1 << "\n";

    for (const auto & it : std::views::reverse(call_stack_))
        ss << "\tat " << it.fn_name << "(" << it.start.file_name << ":" << it.start.line + 1 << ")\n";


    ss << "\n" << start_.line + 1 << " | " << get_line(source_code_, start_.line) << '\n';
    
    size_t length = 1;
    if (end_.line == start_.line && end_.column > start_.column)
        length = end_.column - start_.column;

    const std::string line_label = std::to_string(start_.line + 1) + " | ";
    ss << std::string(line_label.length() + start_.column, ' ') << std::string(length, '^');

    what_cache_ = ss.str();
    return what_cache_.c_str();
}

std::string bs_exception::to_string() const {
    return msg_;
}

bs_obj::bs_obj_ptr bs_exception::copy() const {
    return nullptr;
}
