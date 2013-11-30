#ifndef LIBASTEROID_HPP
#define LIBASTEROID_HPP

#include <cstdint>

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>


namespace galaxy {
    struct asteroid {
        /**
         * this is a map indexed by strings, each representing a label.
         *
         * The labels are mapped to their *declaration points* in the
         * outputted DASM-16 code.
         */
        std::unordered_map<std::string, std::uint16_t> exported_labels;

        /**
         * this is a set of locations in DASM-16 assembly code.
         *
         * In the actual DASM-16 code, any positions where non-imported labels
         * are used will be set to the right value for that asm file
         * and will be added to this set, so they can be moved later
         */
        std::unordered_set<std::uint16_t> used_labels;

        /**
         * this is a map indexed by integers, each representing a location
         * in DASM-16 code.
         *
         * These are positions mapped to the labels *used in those positions*.
         * In the actual DASM-16 code, any positions where imported labels
         * are used will be set to their offset and will be added to this map.
         *
         * are used will be set to their offeset and will be added to this map.
         *
         * For example, SET PC, [foo_bar+1], where foo_bar is some imported label
         * will be changed to SET PC, [1] (with no short literal optimisation), and
         * the linker will be responsible for adding foo_bar to that 1.
         */
        std::unordered_map<std::uint16_t, std::string> imported_labels;

        /// the machine code.
        std::vector<std::uint16_t> object_code;
    };

    namespace asteroid_belt {
        template<typename T>
        std::uint16_t read_uint16_t(T& in);

        template<typename T>
        std::string read_char_string(T& in);

        template<typename T>
        int write_obj(galaxy::asteroid& object, T& outf);

        template<typename T>
        asteroid read_obj(T& in);
    }
}

template<typename T>
int galaxy::asteroid_belt::write_obj(galaxy::asteroid& object, T& outf) {
    std::uint16_t size;
    // write out object_file.exported_labels
    size = object.exported_labels.size();
    (&outf)->write(reinterpret_cast<char*>(&size), sizeof(std::uint16_t));
    for (std::pair<std::string, std::uint16_t> pair : object.exported_labels) {
        const char *s = pair.first.c_str();
        (&outf)->write(s, pair.first.size()+1);
        (&outf)->write(reinterpret_cast<char*>(&pair.second), sizeof(std::uint16_t));
    }

    // write out object_file.imported_labels
    size = object.imported_labels.size();
    (&outf)->write(reinterpret_cast<char*>(&size), sizeof(std::uint16_t));
    for (std::pair<std::uint16_t, std::string> pair : object.imported_labels) {
        const char *s = pair.second.c_str();
        (&outf)->write(s, pair.second.size()+1);
        (&outf)->write(reinterpret_cast<char*>(&pair.first), sizeof(std::uint16_t));
    }

    // write out object_file.used_labels
    size = object.used_labels.size();
    (&outf)->write(reinterpret_cast<char*>(&size), sizeof(std::uint16_t));
    for (std::uint16_t address : object.used_labels) {
        (&outf)->write(reinterpret_cast<char*>(&address), sizeof address);
    }

    // write out object_file.object_code
    size = object.object_code.size();
    (&outf)->write(reinterpret_cast<char*>(&size), sizeof(std::uint16_t));
    for (std::uint16_t byte : object.object_code) {
        (&outf)->write(reinterpret_cast<char*>(&byte), sizeof byte);
    }

    return 0;
}

template<typename T>
std::uint16_t galaxy::asteroid_belt::read_uint16_t(T& in) {
    const std::streamsize uint16_t_size = sizeof(std::uint16_t);
    char* buffer = new char[uint16_t_size];

    in.read(buffer, uint16_t_size);
    std::uint16_t* size = reinterpret_cast<std::uint16_t*>(buffer);

    return *size;
}

template<typename T>
std::string galaxy::asteroid_belt::read_char_string(T& in) {
    char* buffer = new char[1];
    std::string actual_out;

    // char*'s are NULL terminated, so read chars 'till we find a NULL char
    while (in.peek() != 0) {
        in.read(buffer, 1);
        actual_out += buffer[0];
    }
    // one byte of padding is added for god knows why
    in.read(buffer, 1);

    return actual_out;
}

template<typename T>
galaxy::asteroid galaxy::asteroid_belt::read_obj(T& in) {
    galaxy::asteroid object;
    std::uint16_t size;

    // read in the exported_labels
    size = read_uint16_t(in);
    for (int i = 0; i < size; i++) {
        std::pair<std::string, std::uint16_t> pair;
        pair.first = read_char_string(in);
        pair.second = read_uint16_t(in);
        object.exported_labels.insert(pair);
    }

    // read in the imported_labels
    size = read_uint16_t(in);
    for (int i = 0; i < size; i++) {
        std::pair<std::uint16_t, std::string> pair;
        pair.first = read_uint16_t(in);
        pair.second = read_char_string(in);
        object.imported_labels.insert(pair);
    }

    // read in the used_labels
    size = read_uint16_t(in);
    for (int i = 0; i < size; i++) {
        object.used_labels.insert(
            read_uint16_t(in)
        );
    }

    // read in the object_code
    size = read_uint16_t(in);
    for (int i = 0; i < size; i++) {
        object.object_code.push_back(
            read_uint16_t(in)
        );
    }

    return object;
}


#endif /* LIBASTEROID_HPP */
