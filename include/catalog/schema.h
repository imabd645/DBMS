#pragma once
#include <vector>
#include "catalog/column.h"

namespace simpledb {

class Schema {
public:
    explicit Schema(const std::vector<Column> &columns) : columns_(columns) {}

    const std::vector<Column> &GetColumns() const { return columns_; }
    const Column &GetColumn(uint32_t col_idx) const { return columns_[col_idx]; }
    uint32_t GetColumnCount() const { return static_cast<uint32_t>(columns_.size()); }

    int32_t GetColIdx(const std::string &col_name) const {
        for (uint32_t i = 0; i < columns_.size(); ++i) {
            if (columns_[i].GetName() == col_name) return i;
        }
        return -1;
    }

private:
    std::vector<Column> columns_;
};

} // namespace simpledb
