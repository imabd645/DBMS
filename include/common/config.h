#pragma once
#include <cstdint>

namespace simpledb {

// Define fixed page size
constexpr uint32_t PAGE_SIZE = 4096;

// Define INVALID_PAGE_ID
constexpr int32_t INVALID_PAGE_ID = -1;

// Define type for Page ID
using page_id_t = int32_t;

// Define type for Frame ID (used in buffer pool)
using frame_id_t = int32_t;

} // namespace simpledb
