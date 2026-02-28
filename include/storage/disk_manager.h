#pragma once
#include <string>
#include <fstream>
#include "common/config.h"

namespace simpledb {

class DiskManager {
public:
    DiskManager(const std::string &db_file);
    ~DiskManager();

    // Write a page to the database file.
    void WritePage(page_id_t page_id, const char *page_data);
    
    // Read a page from the database file.
    void ReadPage(page_id_t page_id, char *page_data);
    
    // Allocate a new page and return its page_id.
    page_id_t AllocatePage();

    // Size of the db file
    int GetNumPages() const;

private:
    std::fstream db_io_;
    std::string file_name_;
    page_id_t next_page_id_;
};

} // namespace simpledb
