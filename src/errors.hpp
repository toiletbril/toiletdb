#ifndef TOILET_ERRORS_H_
#define TOILET_ERRORS_H_

#include <stdexcept>

namespace toiletdb {

/**
 * @class ParsingError
 * @brief Is thrown when internal parser encounters errors.
 */
class ParsingError : public std::logic_error
{
public:
    ParsingError(std::string const &msg);
};

} // namespace toiletdb

#endif // TOILET_ERRORS_H_