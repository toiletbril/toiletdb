#ifndef TDB_ERRORS_H
#define TDB_ERRORS_H

#include <exception>
#include <stdexcept>

namespace toiletdb {

class Parser_Error : public std::exception
{
    std::string message;

public:
    Parser_Error(const std::string &message);
    const char *what() const noexcept override;
};

} // namespace toiletdb

#endif // TDB_ERRORS_H
