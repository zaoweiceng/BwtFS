#include "node/entry.h"
#include "util/log.h"


BwtFS::Node::Binary BwtFS::Node::entry::to_binary() {
    Binary binary_data;
    bool tp = (type == NodeType::WHITE_NODE) ? 0 : 1;
    binary_data.append(sizeof(size_t), reinterpret_cast<std::byte*>(&bitmap));
    // LOG_INFO << "bitmap:  " << binary_data.to_hex_string();
    binary_data.append(sizeof(bool), reinterpret_cast<std::byte*>(&tp));
    // LOG_INFO << "type:   " << binary_data.to_hex_string();
    binary_data.append(sizeof(uint16_t), reinterpret_cast<std::byte*>(&start));
    // LOG_INFO << "start:  " << binary_data.to_hex_string();
    binary_data.append(sizeof(uint16_t), reinterpret_cast<std::byte*>(&length));
    // LOG_INFO << "length: " << binary_data.to_hex_string();
    binary_data.append(sizeof(uint16_t), reinterpret_cast<std::byte*>(&seed));
    // LOG_INFO << "seed:   " << binary_data.to_hex_string();
    binary_data.append(sizeof(uint8_t), reinterpret_cast<std::byte*>(&level));
    // LOG_INFO << "level:  " << binary_data.to_hex_string();
    return binary_data;
}

BwtFS::Node::entry BwtFS::Node::entry::from_binary(Binary& binary_data) {
    // Store data in local variables to avoid dangling pointer
    auto bitmap_data = binary_data.read(0, sizeof(size_t));
    auto type_data = binary_data.read(sizeof(size_t), sizeof(bool));
    auto start_data = binary_data.read(sizeof(size_t) + sizeof(bool), sizeof(uint16_t));
    auto length_data = binary_data.read(sizeof(size_t) + sizeof(bool) + sizeof(uint16_t), sizeof(uint16_t));
    auto seed_data = binary_data.read(sizeof(size_t) + sizeof(bool) + 2 * sizeof(uint16_t), sizeof(uint16_t));
    auto level_data = binary_data.read(sizeof(size_t) + sizeof(bool) + 2 * sizeof(uint16_t) + sizeof(uint16_t), sizeof(uint8_t));

    // Now safely read the values
    auto bitmap = *reinterpret_cast<size_t*>(bitmap_data.data());
    auto type = *reinterpret_cast<bool*>(type_data.data());
    auto start = *reinterpret_cast<uint16_t*>(start_data.data());
    auto length = *reinterpret_cast<uint16_t*>(length_data.data());
    auto seed = *reinterpret_cast<uint16_t*>(seed_data.data());
    auto level = *reinterpret_cast<uint8_t*>(level_data.data());

    auto type_enum = (type == 0) ? NodeType::WHITE_NODE : NodeType::BLACK_NODE;

    LOG_DEBUG << "Deserializing entry: bitmap=" << bitmap << ", type=" << int(type)
              << ", start=" << start << ", length=" << length
              << ", seed=" << seed << ", level=" << int(level);

    return entry(bitmap, type_enum, start, length, seed, level);
}

BwtFS::Node::entry_list BwtFS::Node::entry_list::from_binary(Binary& binary_data, int num_entries) {
    entry_list list;
    size_t offset = 0;

    // Validate num_entries parameter
    if (num_entries < 0) {
        LOG_ERROR << "Invalid num_entries: " << num_entries << " (must be non-negative)";
        throw std::runtime_error("Invalid num_entries value");
    }

    // Sanity check for extremely large values
    const int MAX_REASONABLE_ENTRIES = 256; // Based on block size
    if (num_entries > MAX_REASONABLE_ENTRIES) {
        LOG_WARNING << "Suspiciously large num_entries: " << num_entries
                   << ", limiting to " << MAX_REASONABLE_ENTRIES;
        num_entries = MAX_REASONABLE_ENTRIES;
    }

    for (size_t i = 0; i < num_entries; i++) {
        // Check if we have enough data to read another entry
        if (offset + entry::size() > binary_data.size()) {
            LOG_WARNING << "Not enough data to read entry " << i
                       << ", offset: " << offset
                       << ", entry size: " << entry::size()
                       << ", binary_data size: " << binary_data.size();
            break;  // Stop reading entries if we don't have enough data
        }
        auto entry_data = binary_data.read(offset, entry::size());
        Binary entry_binary(entry_data);
        LOG_DEBUG << "entry_binary size: " << entry_binary.size();
        if (entry_binary.size() == 0) {
            LOG_WARNING << "Entry binary size is 0, " << entry::size() << " bytes expected, current size: " << entry_binary.size();
            throw std::runtime_error("Entry binary size is 0");
            // continue;
        }
        list.add_entry(std::move(entry::from_binary(entry_binary)));
        offset += entry::size();
    }
    return list;
}

BwtFS::Node::Binary BwtFS::Node::entry_list::to_binary() {
    Binary binary_data;
    for (auto& e : entries) {
        binary_data += std::move(e.to_binary());
    }
    return binary_data;
}