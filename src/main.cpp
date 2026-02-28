#include <iostream>
#include <string>

#include "common/logger.h"
#include "storage/disk_manager.h"

int main() {
    LOG_INFO("SimpleDBMS starting...");
    
    simpledb::DiskManager disk_manager("data.db");
    LOG_INFO("Initialized storage with " << disk_manager.GetNumPages() << " pages.");

    std::string query;
    while (true) {
        std::cout << "dbms> ";
        if (!std::getline(std::cin, query)) break;
        
        if (query == "exit" || query == "quit") {
            break;
        }

        if (query.empty()) {
            continue;
        }

        LOG_INFO("Evaluating: " << query);
        // TODO: parse and execute custom syntax
    }

    LOG_INFO("DBMS shutdown.");
    return 0;
}
