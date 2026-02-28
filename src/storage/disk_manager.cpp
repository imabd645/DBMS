#include "storage/disk_manager.h"
#include "common/logger.h"
#include <filesystem>
#include <cstring>

namespace simpledb {

DiskManager::DiskManager(const std::string &db_file) : file_name_(db_file) {
    db_io_.open(db_file, std::ios::binary | std::ios::in | std::ios::out);
    if (!db_io_.is_open()) {
        db_io_.clear();
        // create file
        db_io_.open(db_file, std::ios::binary | std::ios::trunc | std::ios::out);
        db_io_.close();
        // re-open
        db_io_.open(db_file, std::ios::binary | std::ios::in | std::ios::out);
        if (!db_io_.is_open()) {
            LOG_ERROR("Failed to open db file: " << db_file);
        }
    }
    
    // Determine the next page id
    db_io_.seekp(0, std::ios::end);
    next_page_id_ = db_io_.tellp() / PAGE_SIZE;
}

DiskManager::~DiskManager() {
    if (db_io_.is_open()) {
        db_io_.close();
    }
}

void DiskManager::WritePage(page_id_t page_id, const char *page_data) {
    size_t offset = static_cast<size_t>(page_id) * PAGE_SIZE;
    db_io_.seekp(offset);
    db_io_.write(page_data, PAGE_SIZE);
    if (db_io_.bad()) {
        LOG_ERROR("I/O error while writing to disk");
        return;
    }
    db_io_.flush();
}

void DiskManager::ReadPage(page_id_t page_id, char *page_data) {
    size_t offset = static_cast<size_t>(page_id) * PAGE_SIZE;
    // Check if reading past file
    db_io_.seekg(0, std::ios::end);
    auto file_size = db_io_.tellg();
    if (offset >= static_cast<size_t>(file_size)) {
        LOG_WARN("Read past end of file for page " << page_id);
    }

    db_io_.seekg(offset);
    db_io_.read(page_data, PAGE_SIZE);
    
    // If exact page size is not read (e.g. at end of file)
    size_t read_count = db_io_.gcount();
    if (read_count < PAGE_SIZE) {
        db_io_.clear();
        memset(page_data + read_count, 0, PAGE_SIZE - read_count);
    }
}

page_id_t DiskManager::AllocatePage() {
    return next_page_id_++;
}

int DiskManager::GetNumPages() const {
    return next_page_id_;
}

} // namespace simpledb
