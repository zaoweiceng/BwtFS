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
    auto bitmap = *reinterpret_cast<size_t*>(binary_data.read(0, sizeof(size_t)).data());
    auto type = *reinterpret_cast<bool*>(binary_data.read(sizeof(size_t), sizeof(bool)).data());
    auto start = *reinterpret_cast<uint16_t*>(binary_data.read(sizeof(size_t) + sizeof(bool), sizeof(uint16_t)).data());
    auto length = *reinterpret_cast<uint16_t*>(binary_data.read(sizeof(size_t) + sizeof(bool) + sizeof(uint16_t), sizeof(uint16_t)).data());
    auto seed = *reinterpret_cast<uint16_t*>(binary_data.read(sizeof(size_t) + sizeof(bool) + 2 * sizeof(uint16_t), sizeof(uint16_t)).data());
    auto level = *reinterpret_cast<uint8_t*>(binary_data.read(sizeof(size_t) + sizeof(bool) + 2 * sizeof(uint16_t) + sizeof(uint16_t), sizeof(uint8_t)).data());
    auto type_enum = (type == 0) ? NodeType::WHITE_NODE : NodeType::BLACK_NODE;
    return entry(bitmap, type_enum, start, length, seed, level);
}

BwtFS::Node::entry_list BwtFS::Node::entry_list::from_binary(Binary& binary_data, int num_entries) {
    entry_list list;
    size_t offset = 0;
    // LOG_DEBUG << "entry_list::from_binary, num_entries: " << num_entries;
    for (size_t i = 0; i < num_entries; i++) {
        auto entry_data = binary_data.read(offset, entry::size());
        Binary entry_binary(entry_data);
        // LOG_DEBUG << "entry_binary size: " << entry_binary.size();
        if (entry_binary.size() == 0) {
            LOG_WARNING << "Entry binary size is 0, " << entry::size() << " bytes expected, current size: " << entry_binary.size();
            throw std::runtime_error("Entry binary size is 0");
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