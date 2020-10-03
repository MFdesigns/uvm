#include <iostream>
#include <filesystem>

void printCLIUsage() {
    std::cout << "usage: uvm <path>\n";
}

int main(int argc, char* argv[]) {
    // Check if minimal CLI arguments are provided
    if (argc < 2) {
        printCLIUsage();
        return -1;
    }

    // Check if target UX file exists
    char* sourcePath = argv[1];
    std::filesystem::path p{sourcePath};
    if (!std::filesystem::exists(p)) {
        std::cout << "Target file '" << p.string() << "' does not exist\n";
        return -1;
    }
}
