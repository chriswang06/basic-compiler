#include <iostream>
#include <fstream>
#include <sstream>
#include <optional>
#include <vector>

#if defined(_WIN32) || defined(__CYGWIN__)
    #define OS "win"
    // Windows (x86 or x64)
#elif defined(__linux__)
    #define OS "linux"
    #include "./generation.hpp"


    // Linux
#elif defined(__APPLE__) && defined(__MACH__)
    // Mac OS
    #define OS "mac"
    #include "generation-mac.hpp"


#elif defined(unix) || defined(__unix__) || defined(__unix)
    // Unix like OS
    #define OS "unix"

#else
    #error Unknown environment!

#endif

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Incorrect usage. Correct usage is..." << std::endl;
        std::cout << "hydro <input.hy>" << std::endl;
        return EXIT_FAILURE;

    }

    std::string contents;
    {
        std::stringstream contents_stream;
        std::fstream input(argv[1], std::ios::in);
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    }
    Tokenizer tokenizer(std::move(contents));
    std::vector<Token> tokens = tokenizer.tokenize();

    Parser parser(std::move(tokens));
    std::optional<NodeProgram> prog = parser.parse_prog();

    if (!prog.has_value()) {
        std::cerr << "Invalid Program" << std::endl;
        exit(EXIT_FAILURE);
    }
    Generator generator(prog.value());
    {
        std::fstream file("out.asm", std::ios::out);
        file << generator.gen_prog();
    }

    if (OS == "linux") {
        system("nasm -felf64 out.asm");
        system("ld -o out out.o");
    }
   else if (OS == "mac") {
        system("as -arch arm64 -o out.o out.asm");
        // system("as -arch arm64 -o out.o out.asm && clang -o out out.o && ./out");
        system("clang++ -o out out.o");
        // system("ld -o out out.o");
    } else {
        std::cerr << "Unknown OS" << std::endl;
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}