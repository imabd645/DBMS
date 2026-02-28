#pragma once
#include <vector>
#include "catalog/value.h"
#include "catalog/schema.h"

namespace simpledb {

class Tuple {
public:
    Tuple() = default;
    explicit Tuple(std::vector<Value> values) : values_(std::move(values)) {}

    Value GetValue(const Schema *schema, uint32_t column_idx) const {
        return values_[column_idx];
    }

    const std::vector<Value> &GetValues() const { return values_; }

    void SetValue(uint32_t column_idx, const Value &value) {
        if (column_idx < values_.size()) {
            values_[column_idx] = value;
        }
    }

private:
    std::vector<Value> values_;
};

} // namespace simpledb
