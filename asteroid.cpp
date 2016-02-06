#include "libasteroid.hpp"

namespace detail {
    std::uint16_t network_to_host(std::uint16_t word)  { return ntohs(word); }
    std::uint32_t network_to_host(std::uint32_t dword) { return ntohl(dword); }
    std::uint16_t host_to_network(std::uint16_t word)  { return htons(word); }
    std::uint32_t host_to_network(std::uint32_t dword) { return htonl(dword); }
}

void galaxy::asteroid_belt::write_obj(std::ostream& os, galaxy::asteroid const& obj) {
    write_char_string(os, ASTEROID_MAGIC_STRING);
    write_char_string(os, ASTEROID_VERSION_STRING);

    write_uint16_t(os, obj.exported_labels.size());
    for (auto pair : obj.exported_labels) {
        write_char_string(os, pair.first);
        write_uint16_t(os, pair.second);
    }

    write_uint16_t(os, obj.imported_labels.size());
    for (auto pair : obj.imported_labels) {
        write_char_string(os, pair.second);
        write_uint16_t(os, pair.first);
    }

    write_uint16_t(os, obj.used_labels.size());
    for (std::uint16_t address : obj.used_labels) {
        write_uint16_t(os, address);
    }

    write_uint16_t(os, obj.object_code.size());
    for (std::uint16_t byte : obj.object_code) {
        write_uint16_t(os, byte);
    }
}

int galaxy::asteroid_belt::objectfile_format_check(std::istream& is) {
    // check that it is actually an asteroid object file
    std::string magic_string_check = read_char_string(is);
    if (magic_string_check != ASTEROID_MAGIC_STRING) {
        throw galaxy::asteroid_belt::invalid_object_file(
            "This file is not a valid asteroid object file"
        );
    }

    // check that it is a supported version
    std::string version_str = read_char_string(is);
    int version = std::atoi(version_str.c_str());
    if (version < ASTEROID_MIN_VERSION) {
        throw galaxy::asteroid_belt::invalid_object_file(
            "This version of asteroid does not support such an old version of the asteroid file format."
        );
    }
    if (version > ASTEROID_MAX_VERSION) {
        throw galaxy::asteroid_belt::invalid_object_file(
            "This version of asteroid does not support such a new version of the asteroid file format."
        );
    }

    return version;
}

std::uint16_t galaxy::asteroid_belt::read_uint16_t(std::istream& is) {
    std::uint16_t byte;
    is.read(reinterpret_cast<char*>(&byte), sizeof(std::uint16_t));
    return detail::network_to_host(byte);
}

void galaxy::asteroid_belt::write_uint16_t(std::ostream& os, std::uint16_t n) {
    auto const fixed = detail::host_to_network(n);
    os.write(reinterpret_cast<char const*>(&fixed), sizeof(std::uint16_t));
}


std::string galaxy::asteroid_belt::read_char_string(std::istream& is) {
    auto const length = read_uint16_t(is);

    std::string str;
    str.reserve(length);

    for (int i = 0; i < length; i++) {
        auto const byte = read_uint16_t(is);
        if ((byte >> 8) != 0) {
            throw galaxy::asteroid_belt::invalid_object_file("invalid byte");
        }
        str.push_back(static_cast<char>(byte));
    }

    return str;
}

void galaxy::asteroid_belt::write_char_string(std::ostream& os, std::string str) {
    if (str.size() > std::numeric_limits<std::uint16_t>::max()) {
        throw galaxy::asteroid_belt::invalid_object_file("string too long");
    }
    write_uint16_t(os, static_cast<std::uint16_t>(str.size()));

    for (auto c : str) {
        write_uint16_t(os, static_cast<std::uint16_t>(c));
    }
}


galaxy::asteroid galaxy::asteroid_belt::read_obj(std::istream& is) {
    int version = galaxy::asteroid_belt::objectfile_format_check(is);

    if (version != 1) {
        throw galaxy::asteroid_belt::invalid_object_file("invalid version: " + std::to_string(version));
    }

    // VERSION 0001
    galaxy::asteroid object;
    std::uint16_t size;

    size = read_uint16_t(is);
    for (int i = 0; i < size; i++) {
        std::pair<std::string, std::uint16_t> pair;
        pair.first = read_char_string(is);
        pair.second = read_uint16_t(is);
        object.exported_labels.insert(pair);
    }

    size = read_uint16_t(is);
    for (int i = 0; i < size; i++) {
        std::pair<std::uint16_t, std::string> pair;
        pair.first = read_uint16_t(is);
        pair.second = read_char_string(is);
        object.imported_labels.insert(pair);
    }

    size = read_uint16_t(is);
    for (int i = 0; i < size; i++) {
        object.used_labels.insert(
            read_uint16_t(is)
        );
    }

    size = read_uint16_t(is);
    for (int i = 0; i < size; i++) {
        object.object_code.push_back(
            read_uint16_t(is)
        );
    }

    return object;
}

