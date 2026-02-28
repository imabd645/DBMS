#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include "catalog/schema.h"
#include "catalog/tuple.h"

namespace simpledb {

struct TableInfo {
    std::string name_;
    std::unique_ptr<Schema> schema_;
    std::vector<Tuple> tuples_; // In-memory storage for simplicity in this demo

    TableInfo(std::string name, std::unique_ptr<Schema> schema)
        : name_(std::move(name)), schema_(std::move(schema)) {}
};

class Catalog {
public:
    Catalog() = default;

    TableInfo *CreateTable(const std::string &table_name, const Schema &schema) {
        if (tables_.find(table_name) != tables_.end()) {
            return nullptr;
        }
        auto table_info = std::make_unique<TableInfo>(table_name, std::make_unique<Schema>(schema));
        TableInfo *ptr = table_info.get();
        tables_[table_name] = std::move(table_info);
        return ptr;
    }

    TableInfo *GetTable(const std::string &table_name) {
        if (tables_.find(table_name) == tables_.end()) {
            return nullptr;
        }
        return tables_[table_name].get();
    }

private:
    std::unordered_map<std::string, std::unique_ptr<TableInfo>> tables_;
};

} // namespace simpledb
