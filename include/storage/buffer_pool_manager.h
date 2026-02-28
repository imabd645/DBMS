#pragma once
#include <unordered_map>
#include <list>
#include "storage/disk_manager.h"
#include "storage/page.h"

namespace simpledb {

class BufferPoolManager {
public:
    BufferPoolManager(size_t pool_size, DiskManager *disk_manager);
    ~BufferPoolManager();

    // Fetch a page; returns nullptr if it cannot be fetched
    Page *FetchPage(page_id_t page_id);

    // Unpin a page, indicating if it was modified (dirty)
    bool UnpinPage(page_id_t page_id, bool is_dirty);

    // Flush a page to disk
    bool FlushPage(page_id_t page_id);

    // Create a new page in the buffer pool
    Page *NewPage(page_id_t *page_id);

private:
    bool FindVictim(frame_id_t *frame_id);

    size_t pool_size_;
    DiskManager *disk_manager_;
    Page *pages_;

    std::unordered_map<page_id_t, frame_id_t> page_table_;
    std::list<frame_id_t> free_list_;
    std::list<frame_id_t> eviction_list_;
    
    struct FrameInfo {
        int pin_count = 0;
        bool is_dirty = false;
    };
    FrameInfo *frame_info_;
};

} // namespace simpledb
