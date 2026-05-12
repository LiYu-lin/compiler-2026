#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _WIN32
#include <process.h>
#endif

#ifndef SYSY_COMPILER_PATH
#error "SYSY_COMPILER_PATH must be defined"
#endif

namespace {

std::string trimQuotes(std::string value) {
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
        return value.substr(1, value.size() - 2);
    }
    return value;
}

std::string readFile(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::in | std::ios::binary);
    if (!input) {
        throw std::runtime_error("Failed to read file: " + path.string());
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

}

int main() {
    try {
        auto compilerPath = std::filesystem::path(trimQuotes(SYSY_COMPILER_PATH));
        compilerPath.make_preferred();

        auto tempDir = std::filesystem::temp_directory_path() / "sysy_driver_test";
        tempDir.make_preferred();
        std::filesystem::create_directories(tempDir);

        auto inputPath = tempDir / "sample.sy";
        auto outputPath = tempDir / "sample.s";
        inputPath.make_preferred();
        outputPath.make_preferred();

        {
            std::ofstream input(inputPath);
            input
                << "int add(int x, int y) { return x + y; }\n"
                << "int main() {\n"
                << "  int a = 5;\n"
                << "  if (a >= 10 && a < 20) {\n"
                << "    a = add(a, 1);\n"
                << "  } else {\n"
                << "    a = add(a, 2);\n"
                << "  }\n"
                << "  return a;\n"
                << "}\n";
        }

#ifdef _WIN32
        const std::string compiler = compilerPath.string();
        const std::string input = inputPath.string();
        const std::string output = outputPath.string();
        std::vector<const char*> argv = {
            compiler.c_str(),
            input.c_str(),
            "-S",
            "-o",
            output.c_str(),
            "-O1",
            nullptr
        };
        const int exitCode = _spawnv(_P_WAIT, compiler.c_str(), argv.data());
#else
        const std::string command =
            "\"" + compilerPath.string() + "\" \"" +
            inputPath.string() + "\" -S -o \"" + outputPath.string() + "\" -O1";
        const int exitCode = std::system(command.c_str());
#endif
        if (exitCode != 0) {
            throw std::runtime_error("compiler exited with code " + std::to_string(exitCode));
        }

        const std::string asmOutput = readFile(outputPath);
        if (asmOutput.find(".globl main") == std::string::npos) {
            throw std::runtime_error("Driver output missing main symbol");
        }
        if (asmOutput.find("ret") == std::string::npos) {
            throw std::runtime_error("Driver output missing return instruction");
        }
        if (asmOutput.find("call add") == std::string::npos) {
            throw std::runtime_error("Driver output missing lowered function call");
        }
        if (asmOutput.find("bne") == std::string::npos) {
            throw std::runtime_error("Driver output missing conditional branch lowering");
        }
        if (asmOutput.find("unknown ") != std::string::npos) {
            throw std::runtime_error("Driver output contains unresolved pseudo instructions");
        }

        std::cout << "Compiler driver test passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Compiler driver test failed: " << e.what() << std::endl;
        return 1;
    }
}
