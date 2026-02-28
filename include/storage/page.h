#pragma once
#include <cstring>
#include "common/config.h"

namespace simpledb {

class Page {
public:
    Page() { ResetMemory(); }
    ~Page() = default;

    inline char *GetData() { return data_; }
    inline page_id_t GetPageId() const { return page_id_; }
    inline void SetPageId(page_id_t page_id) { page_id_ = page_id; }
    inline void ResetMemory() { memset(data_, 0, PAGE_SIZE); }

private:
    char data_[PAGE_SIZE]{};
    page_id_t page_id_ = INVALID_PAGE_ID;
};

} // namespace simpledb
