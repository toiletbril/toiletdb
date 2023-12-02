#include "errors.hpp"

namespace toiletdb {

Parser_Error::Parser_Error(const std::string &message) :
    message(message) {}

const char *Parser_Error::what() const noexcept
{
    return message.c_str();
}

} // namespace toiletdb
