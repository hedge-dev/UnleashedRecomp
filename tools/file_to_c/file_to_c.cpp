/*
    MIT License
    
    Copyright (c) 2024 RT64 Contributors
    
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
    
    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.
    
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include <filesystem>
#include <fstream>
#include <cstdio>
#include <vector>

std::vector<char> read_file(const char* path) {
    std::ifstream input_file{path, std::ios::binary};
    std::vector<char> ret{};

    if (!input_file.good()) {
        return ret;
    }

    // Get the length of the file
    input_file.seekg(0, std::ios::end);
    ret.resize(input_file.tellg());
    
    // Read the file contents into the vector
    input_file.seekg(0, std::ios::beg);
    input_file.read(ret.data(), ret.size());

    return ret;
}

void create_parent_if_needed(const char* path) {
    std::filesystem::path parent_path = std::filesystem::path{path}.parent_path();
    if (!parent_path.empty()) {
        std::filesystem::create_directories(parent_path);
    }
}

int main(int argc, const char** argv) {
    if (argc != 6) {
        printf("Usage: %s [input file] [array name] [array type] [output C file] [output C header]\n", argv[0]);
        return EXIT_SUCCESS;
    }

    const char* input_path = argv[1];
    const char* array_name = argv[2];
    const char* array_type = argv[3];
    const char* output_c_path = argv[4];
    const char* output_h_path = argv[5];

    // Read the input file's contents
    std::vector<char> contents = read_file(input_path);

    if (contents.empty()) {
        fprintf(stderr, "Failed to open file %s! (Or it's empty)\n", input_path);
        return EXIT_FAILURE;
    }

    // Create the output directories if they don't exist
    create_parent_if_needed(output_c_path);
    create_parent_if_needed(output_h_path);

    // Write the C file with the array
    {
        std::ofstream output_c_file{output_c_path};
        output_c_file << "extern " << array_type << " " << array_name << "[" << contents.size() << "];\n";
        output_c_file << array_type << " " << array_name << "[" << contents.size() << "] = {";

        for (char x : contents) {
            output_c_file << (int)x << ", ";
        }

        output_c_file << "};\n";
    }

    // Write the header file with the extern array
    {
        std::ofstream output_h_file{output_h_path};
        output_h_file <<
            "#ifdef __cplusplus\n"
            "  extern \"C\" {\n"
            "#endif\n"
            "extern " << array_type << " " << array_name << "[" << contents.size() << "];\n"
            "#ifdef __cplusplus\n"
            "  }\n"
            "#endif\n";
    }
}
