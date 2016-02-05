#include <iostream>
#include <fstream>

#include "libasteroid.hpp"

int main(int argc, char const *argv[]) {
    std::cout << "asteroid regurgitator" << std::endl;

    if (argc == 2) {
        std::string in = argv[1];

        std::ifstream inf{in};

        galaxy::asteroid objfile = galaxy::asteroid_belt::read_obj(inf);

        std::cout << "Exported Labels:" << std::endl;
        for (auto&& pair : objfile.exported_labels) {
            std::cout << pair.first << ": 0x" << std::hex << pair.second << std::endl;
        }
        std::cout << std::endl;

        std::cout << "Imported Labels:" << std::endl;
        for (auto&& pair : objfile.imported_labels) {
            std::cout << "0x" << std::hex << pair.first;
            std::cout << ": " << pair.second << std::endl;
        }
        std::cout << std::endl;

        std::cout << "Used_labels:" << std::endl;
        for (auto&& val : objfile.used_labels) {
            std::cout << val << std::endl;
        }
        std::cout << std::endl;

        std::cout << "Object_code:" << std::endl;
        for (auto&& val : objfile.object_code) {
            std::cout << "0x" << std::hex << val << std::endl;
        }
        std::cout << std::endl;

        return 0;
    } else {
        std::cout << "Too many or too few arguments" << std::endl;
        return -1;
    }
}
