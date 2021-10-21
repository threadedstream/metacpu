#pragma once

// C-includes
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstdint>
#include <cstring>


// Cpp-includes
#include <string>

constexpr uint16_t preamble_size = 12;
constexpr uint8_t memory_bank_size = 0xFF;


constexpr const char expected_preamble[preamble_size + 1] = "metasm v_1_0";

enum {
    FAILED_TO_INIT_STREAM,
    FAILED_TO_READ_PREAMBLE,
    FAILED_TO_READ_CONTENTS,
    FAILED_TO_ALLOCATE_MEMORY,
    MALFORMED_PREAMBLE,
};

static const char *errors[] = {
        "failed to initialize stream\n",
        "failed to read preamble\n",
        "failed to read contents from a file\n",
        "failed to allocate memory\n",
        "malformed preamble\n",
};

static bool loadMachineCodeIntoMemory(const char *path, uint16_t **buffer_ptr, int32_t *size) {
    FILE *stream = fopen(path, "rb");
    if (!stream) {
        fputs(errors[FAILED_TO_INIT_STREAM], stderr);
        return false;
    }

    fseek(stream, 0, SEEK_END);
    const auto file_size = ftell(stream);
    fseek(stream, 0, SEEK_SET);

    char preamble[preamble_size + 1];
    if (!fread(preamble, 1, preamble_size + 1, stream)) {
        fputs(errors[FAILED_TO_READ_PREAMBLE], stderr);
        return false;
    }

    if (strcmp(preamble, expected_preamble)) {
        fputs(errors[MALFORMED_PREAMBLE], stderr);
        return false;
    }

    const auto bin_data_size = file_size - preamble_size;
    *buffer_ptr = static_cast<uint16_t *>(calloc(bin_data_size, sizeof(uint16_t)));
    if (!(*buffer_ptr)) {
        fputs(errors[FAILED_TO_ALLOCATE_MEMORY], stderr);
        return false;
    }

    if (!fread(*buffer_ptr, sizeof(uint16_t), bin_data_size, stream)) {
        fputs(errors[FAILED_TO_READ_CONTENTS], stderr);
        return false;
    }

    *size = bin_data_size;

    fclose(stream);

    return true;
}


struct vm {
    vm() : acc{0}, pc{0} {}

    uint16_t acc;
    uint16_t *memory;
    uint8_t pc;
    bool zf;
};


class Interpreter final {
public:

    Interpreter() = default;

    bool initializeVm(const std::string &path);

    void simulate();

    inline void destroyVm() {
        free(vm_->memory);
        vm_->memory = nullptr;
        free(vm_);
        vm_ = nullptr;
    }

    ~Interpreter() { destroyVm(); }

private:
    void addi(uint8_t value);

    void add(uint8_t addr);

    void subi(uint8_t value);

    void sub(uint8_t addr);

    void clac();

    void bnz(uint8_t addr);

    void bz(uint8_t addr);

    void ucb(uint8_t addr);

    void str(uint8_t addr);

private:
    vm *vm_;
};