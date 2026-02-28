#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cctype>

#include "common/logger.h"
#include "storage/disk_manager.h"
#include "catalog/catalog.h"
#include "catalog/type_id.h"

using namespace simpledb;

static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
}
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
}
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

void ExecuteMakeTable(Catalog &catalog, const std::string &query) {
    size_t start_paren = query.find('(');
    size_t end_paren = query.rfind(')');
    if (start_paren == std::string::npos || end_paren == std::string::npos) {
        LOG_ERROR("Syntax error. Expected: make table <name> (col1 type1, ...)");
        return;
    }

    std::string table_part = query.substr(10, start_paren - 10);
    trim(table_part);

    std::string cols_part = query.substr(start_paren + 1, end_paren - start_paren - 1);
    std::stringstream ss(cols_part);
    std::string col_def;
    std::vector<Column> columns;

    while (std::getline(ss, col_def, ',')) {
        trim(col_def);
        size_t space = col_def.find(' ');
        if (space == std::string::npos) {
            LOG_ERROR("Invalid column definition: " << col_def);
            return;
        }
        std::string cname = col_def.substr(0, space);
        std::string ctype = col_def.substr(space + 1);
        trim(cname); trim(ctype);

        std::string upper_ctype = ctype;
        std::transform(upper_ctype.begin(), upper_ctype.end(), upper_ctype.begin(), ::toupper);

        TypeId t = TypeId::INVALID;
        if (upper_ctype == "INT" || upper_ctype == "INTEGER") {
            t = TypeId::INTEGER;
        } else if (upper_ctype == "VARCHAR" || upper_ctype == "STRING" || 
                   upper_ctype.find("VARCHAR(") == 0 || upper_ctype.find("STRING(") == 0) {
            t = TypeId::VARCHAR;
        } else {
            LOG_ERROR("Unsupported type: " << ctype);
            return;
        }
        columns.emplace_back(cname, t);
    }

    Schema schema(columns);
    if (catalog.CreateTable(table_part, schema)) {
        LOG_INFO("Table '" << table_part << "' created successfully.");
    } else {
        LOG_ERROR("Table '" << table_part << "' already exists.");
    }
}

void ExecuteShowAll(Catalog &catalog, const std::string &query) {
    std::string table_name = query.substr(14);
    trim(table_name);
    if (!table_name.empty() && table_name.back() == ';') table_name.pop_back();

    TableInfo *table = catalog.GetTable(table_name);
    if (!table) {
        LOG_ERROR("Table '" << table_name << "' not found.");
        return;
    }

    const Schema &schema = *table->schema_;
    for (const auto &col : schema.GetColumns()) {
        std::cout << col.GetName() << "\t|\t";
    }
    std::cout << "\n------------------------------------------------------\n";

    for (const auto &tuple : table->tuples_) {
        for (uint32_t i = 0; i < schema.GetColumnCount(); ++i) {
            std::cout << tuple.GetValue(&schema, i).ToString() << "\t|\t";
        }
        std::cout << "\n";
    }
    LOG_INFO(table->tuples_.size() << " rows returned.");
}

void ExecuteRemoveFrom(Catalog &catalog, const std::string &query) {
    size_t where_pos = query.find(" where ");
    if (where_pos == std::string::npos) {
        LOG_ERROR("Syntax error. Expected: remove from <table> where <col> = <val>");
        return;
    }

    std::string table_name = query.substr(12, where_pos - 12);
    trim(table_name);

    std::string cond_part = query.substr(where_pos + 7);
    trim(cond_part);

    size_t eq_pos = cond_part.find('=');
    if (eq_pos == std::string::npos) {
         LOG_ERROR("Syntax error: missing '=' in where clause.");
         return;
    }
    
    std::string col_name = cond_part.substr(0, eq_pos);
    std::string val_str = cond_part.substr(eq_pos + 1);
    trim(col_name); trim(val_str);
    if (!val_str.empty() && val_str.back() == ';') val_str.pop_back();

    TableInfo *table = catalog.GetTable(table_name);
    if (!table) {
        LOG_ERROR("Table not found.");
        return;
    }

    int32_t col_idx = table->schema_->GetColIdx(col_name);
    if (col_idx == -1) {
        LOG_ERROR("Column not found in table.");
        return;
    }

    size_t removed = 0;
    auto it = table->tuples_.begin();
    while (it != table->tuples_.end()) {
        Value v = it->GetValue(table->schema_.get(), col_idx);
        bool match = false;
        if (v.GetTypeId() == TypeId::INTEGER && std::to_string(v.GetAsInt()) == val_str) match = true;
        if (v.GetTypeId() == TypeId::VARCHAR && v.GetAsString() == val_str) match = true;
        if (v.GetTypeId() == TypeId::VARCHAR && val_str.front() == '\'' && val_str.back() == '\'') {
             std::string unquoted = val_str.substr(1, val_str.size() - 2);
             if (v.GetAsString() == unquoted) match = true;
        }

        if (match) {
            it = table->tuples_.erase(it);
            removed++;
        } else {
            ++it;
        }
    }
    LOG_INFO("Removed " << removed << " rows.");
}

void ExecuteChangeTable(Catalog &catalog, const std::string &query) {
    size_t set_pos = query.find(" set ");
    if (set_pos == std::string::npos) {
        LOG_ERROR("Syntax error. Expected: change <table> set <col> = <val>");
        return;
    }

    std::string table_name = query.substr(7, set_pos - 7);
    trim(table_name);

    std::string assgn_part = query.substr(set_pos + 5);
    size_t eq_pos = assgn_part.find('=');
    if (eq_pos == std::string::npos) {
        LOG_ERROR("Syntax error: missing '=' in set clause.");
        return;
    }

    std::string col_name = assgn_part.substr(0, eq_pos);
    std::string val_str = assgn_part.substr(eq_pos + 1);
    trim(col_name); trim(val_str);
    if (!val_str.empty() && val_str.back() == ';') val_str.pop_back();

    TableInfo *table = catalog.GetTable(table_name);
    if (!table) {
        LOG_ERROR("Table not found.");
        return;
    }

    int32_t col_idx = table->schema_->GetColIdx(col_name);
    if (col_idx == -1) {
         LOG_ERROR("Column not found.");
         return;
    }

    TypeId t = table->schema_->GetColumn(col_idx).GetType();
    Value new_val;
    if (t == TypeId::INTEGER) {
        new_val = Value(std::stoi(val_str));
    } else {
        if (val_str.front() == '\'' && val_str.back() == '\'') {
            val_str = val_str.substr(1, val_str.size() - 2);
        }
        new_val = Value(val_str);
    }

    for (auto &tuple : table->tuples_) {
        tuple.SetValue(col_idx, new_val);
    }
    LOG_INFO("Updated " << table->tuples_.size() << " rows.");
}

void ExecuteInsertInto(Catalog &catalog, const std::string &query) {
    size_t val_pos = query.find(" values ");
    if (val_pos == std::string::npos) {
        LOG_ERROR("Syntax error. Expected: insert into <table> values (v1, v2)");
        return;
    }

    std::string table_name = query.substr(12, val_pos - 12);
    trim(table_name);

    TableInfo *table = catalog.GetTable(table_name);
    if (!table) {
        LOG_ERROR("Table not found.");
        return;
    }

    size_t start_paren = query.find('(', val_pos);
    size_t end_paren = query.find(')', start_paren);
    if (start_paren == std::string::npos || end_paren == std::string::npos) {
        LOG_ERROR("Syntax error in values list.");
        return;
    }

    std::string vals_part = query.substr(start_paren + 1, end_paren - start_paren - 1);
    std::stringstream ss(vals_part);
    std::string val_tok;
    std::vector<Value> row_values;

    uint32_t col_idx = 0;
    while (std::getline(ss, val_tok, ',')) {
        trim(val_tok);
        if (col_idx >= table->schema_->GetColumnCount()) {
            LOG_ERROR("Too many values provided.");
            return;
        }

        TypeId expected_type = table->schema_->GetColumn(col_idx).GetType();
        if (expected_type == TypeId::INTEGER) {
            row_values.emplace_back(std::stoi(val_tok));
        } else {
            if (val_tok.front() == '\'' && val_tok.back() == '\'') {
                val_tok = val_tok.substr(1, val_tok.size() - 2);
            }
            row_values.emplace_back(val_tok);
        }
        col_idx++;
    }

    if (row_values.size() != table->schema_->GetColumnCount()) {
        LOG_ERROR("Not enough values provided.");
        return;
    }

    table->tuples_.emplace_back(row_values);
    LOG_INFO("1 row inserted.");
}

int main() {
    LOG_INFO("SimpleDBMS starting... Type 'exit' to quit.");
    
    // Initialize storage subsystem to fulfill architecture requirements
    DiskManager disk_manager("data.db");
    Catalog catalog;

    std::string query;
    while (true) {
        std::cout << "dbms> ";
        if (!std::getline(std::cin, query)) break;
        
        trim(query);
        if (query == "exit" || query == "quit") break;
        if (query.empty()) continue;

        if (query.rfind("make table", 0) == 0) {
            ExecuteMakeTable(catalog, query);
        } else if (query.rfind("show all from", 0) == 0) {
            ExecuteShowAll(catalog, query);
        } else if (query.rfind("remove from", 0) == 0) {
            ExecuteRemoveFrom(catalog, query);
        } else if (query.rfind("change", 0) == 0) {
            ExecuteChangeTable(catalog, query);
        } else if (query.rfind("insert into", 0) == 0) {
            ExecuteInsertInto(catalog, query);
        } else {
            LOG_ERROR("Unknown command: " << query);
        }
    }

    LOG_INFO("DBMS shutdown.");
    return 0;
}
