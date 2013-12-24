#ifndef LIBASTEROID_HPP
#define LIBASTEROID_HPP

#include <cstdint>

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <cstdint>

#define LIBASTEROID_MAGIC_STRING "GALAXYOBJ"
#define LIBASTEROID_VERSION_STRING "v0.1"

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
        asteroid read_obj(T& in);

        template<typename T>
        void write_char_string(T& outf, std::string str);
        template<typename T>
        void write_uint16_t(T& outf, std::uint16_t i);
        template<typename T>
        void write_obj(galaxy::asteroid& object, T& outf);

        template<typename T>
        void objectfile_format_check(T& in);

        class invalid_object_file : std::exception {
        private:
            std::string message;
        public:
            invalid_object_file(std::string message) : message(message) {};
            virtual ~invalid_object_file() {};
            virtual const char* what() const noexcept {
                return message.c_str();
            }
        };
    }
}

template<typename T>
void galaxy::asteroid_belt::write_obj(galaxy::asteroid& object, T& outf) {
    write_char_string(outf, LIBASTEROID_MAGIC_STRING);
    write_char_string(outf, LIBASTEROID_VERSION_STRING);

    // write out object_file.exported_labels
    write_uint16_t(outf, object.exported_labels.size());
    for (std::pair<std::string, std::uint16_t> pair : object.exported_labels) {
        write_char_string(outf, pair.first);
        write_uint16_t(outf, pair.second);
    }

    // write out object_file.imported_labels
    write_uint16_t(outf, object.imported_labels.size());
    for (std::pair<std::uint16_t, std::string> pair : object.imported_labels) {
        write_char_string(outf, pair.second);
        write_uint16_t(outf, pair.first);
    }

    // write out object_file.used_labels
    write_uint16_t(outf, object.used_labels.size());
    for (std::uint16_t address : object.used_labels) {
        write_uint16_t(outf, address);
    }

    // write out object_file.object_code
    write_uint16_t(outf, object.object_code.size());
    for (std::uint16_t byte : object.object_code) {
        write_uint16_t(outf, byte);
    }
}

template<typename T>
void galaxy::asteroid_belt::objectfile_format_check(T& in) {
    std::string magic_string_check = read_char_string(in);
    if (magic_string_check != LIBASTEROID_MAGIC_STRING) {
        throw galaxy::asteroid_belt::invalid_object_file(
            "This file does not seem to be a Jupiter object file"
        );
    }

    std::string version_check = read_char_string(in);
    if (version_check != LIBASTEROID_VERSION_STRING) {
        std::string message = "This file was generated with version ";
        message += LIBASTEROID_VERSION_STRING;
        message += " of libasteroid/jupiter";
        throw galaxy::asteroid_belt::InvalidObjectFile(
            message
        );
    }
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
void galaxy::asteroid_belt::write_uint16_t(T& outf, std::uint16_t i) {
    (&outf)->write(reinterpret_cast<char*>(&i), sizeof(std::uint16_t));
}


template<typename T>
std::string galaxy::asteroid_belt::read_char_string(T& in) {
    std::string out;

    std::getline(in, out, '\0');

    return out;
}

template<typename T>
void galaxy::asteroid_belt::write_char_string(T& outf, std::string str) {
    const char *s = str.c_str();
    (&outf)->write(s, str.size()+1);
}


template<typename T>
galaxy::asteroid galaxy::asteroid_belt::read_obj(T& in) {
    galaxy::asteroid_belt::objectfile_format_check(in);

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
