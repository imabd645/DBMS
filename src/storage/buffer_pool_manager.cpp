#include "storage/buffer_pool_manager.h"
#include "common/logger.h"

namespace simpledb {

BufferPoolManager::BufferPoolManager(size_t pool_size, DiskManager *disk_manager)
    : pool_size_(pool_size), disk_manager_(disk_manager) {
    pages_ = new Page[pool_size_];
    frame_info_ = new FrameInfo[pool_size_];

    for (size_t i = 0; i < pool_size_; ++i) {
        free_list_.push_back(static_cast<frame_id_t>(i));
    }
}

BufferPoolManager::~BufferPoolManager() {
    for (auto const& [page_id, frame_id] : page_table_) {
        if (frame_info_[frame_id].is_dirty) {
            FlushPage(page_id);
        }
    }
    delete[] pages_;
    delete[] frame_info_;
}

bool BufferPoolManager::FindVictim(frame_id_t *frame_id) {
    if (!free_list_.empty()) {
        *frame_id = free_list_.front();
        free_list_.pop_front();
        return true;
    }

    for (auto it = eviction_list_.begin(); it != eviction_list_.end(); ++it) {
        frame_id_t victim_frame_id = *it;
        if (frame_info_[victim_frame_id].pin_count == 0) {
            *frame_id = victim_frame_id;
            eviction_list_.erase(it);

            page_id_t victim_page_id = pages_[victim_frame_id].GetPageId();
            if (frame_info_[victim_frame_id].is_dirty) {
                disk_manager_->WritePage(victim_page_id, pages_[victim_frame_id].GetData());
            }

            page_table_.erase(victim_page_id);
            frame_info_[victim_frame_id].is_dirty = false;
            return true;
        }
    }
    return false; // All pages pinned
}

Page *BufferPoolManager::FetchPage(page_id_t page_id) {
    if (page_table_.count(page_id) > 0) {
        frame_id_t frame_id = page_table_[page_id];
        frame_info_[frame_id].pin_count++;
        return &pages_[frame_id];
    }

    frame_id_t frame_id;
    if (!FindVictim(&frame_id)) {
        return nullptr;
    }

    pages_[frame_id].ResetMemory();
    disk_manager_->ReadPage(page_id, pages_[frame_id].GetData());
    pages_[frame_id].SetPageId(page_id);
    
    page_table_[page_id] = frame_id;
    frame_info_[frame_id].pin_count = 1;
    frame_info_[frame_id].is_dirty = false;
    eviction_list_.push_back(frame_id);

    return &pages_[frame_id];
}

bool BufferPoolManager::UnpinPage(page_id_t page_id, bool is_dirty) {
    if (page_table_.count(page_id) == 0) return false;

    frame_id_t frame_id = page_table_[page_id];
    if (frame_info_[frame_id].pin_count <= 0) return false;

    frame_info_[frame_id].pin_count--;
    if (is_dirty) {
        frame_info_[frame_id].is_dirty = true;
    }

    return true;
}

bool BufferPoolManager::FlushPage(page_id_t page_id) {
    if (page_table_.count(page_id) == 0) return false;

    frame_id_t frame_id = page_table_[page_id];
    disk_manager_->WritePage(page_id, pages_[frame_id].GetData());
    frame_info_[frame_id].is_dirty = false;

    return true;
}

Page *BufferPoolManager::NewPage(page_id_t *page_id) {
    frame_id_t frame_id;
    if (!FindVictim(&frame_id)) return nullptr;

    *page_id = disk_manager_->AllocatePage();
    pages_[frame_id].ResetMemory();
    pages_[frame_id].SetPageId(*page_id);

    page_table_[*page_id] = frame_id;
    frame_info_[frame_id].pin_count = 1;
    frame_info_[frame_id].is_dirty = false;
    eviction_list_.push_back(frame_id);

    return &pages_[frame_id];
}

} // namespace simpledb
