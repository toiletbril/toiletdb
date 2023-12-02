#ifndef TDB_FILE_MANAGER_H
#define TDB_FILE_MANAGER_H

#include <memory>
#include <vector>
#include <list>
#include <span>

#include "debug.hpp"
#include "common.hpp"
#include "types.hpp"

namespace toiletdb {

class File_Manager
{
public:
    virtual ~File_Manager() = default;

    virtual bool check() = 0;
    virtual std::vector<Column_Info> read_header() = 0;

    virtual void serialize(const std::vector<Column_Base *> *columns)  = 0;
    virtual void serialize(const std::vector<Column_Base *> *columns, std::string_view file_path)  = 0;

    virtual std::vector<Column_Base *> deserialize(std::vector<Column_Info> types) = 0;
    virtual std::vector<Column_Base *> deserialize(std::vector<Column_Info> types, std::string_view file_path) = 0;
};

class Memory_File_Manager : public File_Manager
{
public:
    Memory_File_Manager(std::string_view file_path);

    bool check() override;
    std::vector<Column_Info> read_header() override;

    void serialize(const std::vector<Column_Base *> *columns) override;
    std::vector<Column_Base *> deserialize(std::vector<Column_Info> types) override;

private:
    std::string file_path;

    size_t format_version { 0 };
    size_t header_size { 0 };

    std::fstream open(std::ios::openmode open_mode);
};

} // namespace toiletdb

#endif // TDB_FILE_MANAGER_H
