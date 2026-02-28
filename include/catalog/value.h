#pragma once
#include <string>
#include "catalog/type_id.h"

namespace simpledb {

class Value {
public:
    Value() : type_id_(TypeId::INVALID) {}
    explicit Value(int32_t val) : type_id_(TypeId::INTEGER), int_val_(val) {}
    explicit Value(const std::string &val) : type_id_(TypeId::VARCHAR), varchar_val_(val) {}

    TypeId GetTypeId() const { return type_id_; }
    int32_t GetAsInt() const { return int_val_; }
    std::string GetAsString() const { return varchar_val_; }

    // For simplicity, serialize to a string representation
    std::string ToString() const {
        if (type_id_ == TypeId::INTEGER) return std::to_string(int_val_);
        if (type_id_ == TypeId::VARCHAR) return varchar_val_;
        return "NULL";
    }

private:
    TypeId type_id_;
    int32_t int_val_ = 0;
    std::string varchar_val_;
};

} // namespace simpledb
