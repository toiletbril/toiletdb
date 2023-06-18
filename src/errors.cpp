#include "errors.hpp"

namespace toiletdb {

ParsingError::ParsingError(std::string const &msg) :
    std::logic_error(msg) {}

} // namespace toiletdb
