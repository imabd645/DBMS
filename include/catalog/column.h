#pragma once
#include <string>
#include "catalog/type_id.h"

namespace simpledb {

class Column {
public:
    Column(const std::string &column_name, TypeId type)
        : column_name_(column_name), column_type_(type) {}

    std::string GetName() const { return column_name_; }
    TypeId GetType() const { return column_type_; }

private:
    std::string column_name_;
    TypeId column_type_;
};

} // namespace simpledb
