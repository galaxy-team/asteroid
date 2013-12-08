#include <iostream>
#include <fstream>

#include "libasteroid.hpp"

int main(int argc, char const *argv[])
{
    if (argc == 2){
        std::string in = argv[1];

        std::ifstream inf(in);

        galaxy::asteroid objfile = galaxy::asteroid_belt::read_obj(inf);

        std::cout << "Exported Labels:" << std::endl;
        for (std::pair<std::string, std::uint16_t> pair : objfile.exported_labels) {
            std::cout << pair.first << ": " << pair.second << std::endl;
        }
        std::cout << std::endl;

        std::cout << "Imported Labels:" << std::endl;
        for (std::pair<std::uint16_t, std::string> pair : objfile.imported_labels) {
            std::cout << pair.first << ": " << pair.second << std::endl;
        }
        std::cout << std::endl;

        std::cout << "Used_labels:" << std::endl;
        for (auto val: objfile.used_labels) {
            std::cout << val << std::endl;
        }
        std::cout << std::endl;

        std::cout << "Object_code:" << std::endl;
        for (auto val: objfile.object_code) {
            std::cout << "0x" << std::hex << val << std::endl;
        }
        std::cout << std::endl;

        return 0;

    } else {
        std::cout << "Not enough args" << std::endl;
        return -1;
    }
}
