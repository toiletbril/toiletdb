#ifndef TDB_TABLE_H
#define TDB_TABLE_H

#include <optional>
#include <memory>
#include <string>
#include <list>
#include <span>

#include "common.hpp"
#include "file_manager.hpp"
#include "types.hpp"

namespace toiletdb {

class Table
{
public:
    ~Table() = default;

    virtual void init() = 0;

    virtual void serialize() const = 0;
    virtual void serialize(std::string_view file_path) const = 0;

    virtual void deserialize() = 0;
    virtual void deserialize(std::string_view file_path) = 0;

    virtual std::optional<size_t> search(size_t id) const = 0;
    virtual std::optional<std::vector<size_t>> search(size_t column_pos, std::string_view query) const = 0;

    virtual std::optional<const void *> get(size_t column_pos, size_t row_pos) = 0;
    virtual std::optional<void *> get_mut(size_t column_pos, size_t row_pos) = 0;

    virtual std::optional<std::list<const void *>> get_row(size_t pos) const = 0;
    virtual std::optional<std::list<void *>> get_row_mut(size_t pos) = 0;

    virtual int add_row(std::span<const void *> &row) = 0;

    virtual void erase_row(size_t pos) = 0;
    virtual void erase_row_id(size_t id) = 0;

    virtual void clear_table() = 0;

    virtual size_t column_count() const = 0;
    virtual size_t row_count() const = 0;

    virtual size_t get_id_column_pos() const = 0;
    virtual const std::string_view get_column_name(size_t pos) const = 0;
    virtual const std::list<std::string_view> column_names() const = 0;
    virtual size_t get_column_pos(std::string_view name) const = 0;

    virtual const std::list<const Column_Info &> get_types() const = 0;
    virtual const Column_Info &get_column_type(size_t pos) const = 0;

protected:
    std::unique_ptr<File_Manager> file_manager;
};

class Memory_Table : public Table
{
public:
    Memory_Table(std::string_view file_path);

    void init() override;

    void serialize() const override;
    void serialize(std::string_view file_path) const override;

    void deserialize() override;
    void deserialize(std::string_view file_path) override;

    std::optional<size_t> search(size_t id) const override;
    std::optional<std::vector<size_t>> search(size_t column_pos, std::string_view query) const override;

    std::optional<const void *> get(size_t column_pos, size_t row_pos) override;
    std::optional<void *> get_mut(size_t column_pos, size_t row_pos) override;

    std::optional<std::list<const void *>> get_row(size_t pos) const override;
    std::optional<std::list<void *>> get_row_mut(size_t pos) override;

    int add_row(std::span<const void *> &row) override;

    void erase_row(size_t pos) override;
    void erase_row_id(size_t id) override;

    void clear_table() override;

    size_t column_count() const override;
    size_t row_count() const override;

    size_t get_id_column_pos() const override;
    const std::string_view get_column_name(size_t pos) const override;
    const std::list<std::string_view> column_names() const override;
    size_t get_column_pos(std::string_view name) const override;

    const std::list<const Column_Info &> get_types() const override;
    const Column_Info &get_column_type(size_t pos) const override;

private:
    std::unique_ptr<File_Manager> file_manager;
    std::unique_ptr<std::vector<Column_Base *>> columns;
};

} // namespace toiletdb

#endif // TDB_TABLE_H
