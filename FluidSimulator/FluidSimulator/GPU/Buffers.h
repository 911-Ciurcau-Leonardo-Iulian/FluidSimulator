#pragma once

class StructuredBuffer {
public:
    StructuredBuffer() = default;
    StructuredBuffer(size_t element_byte_size, size_t element_count);
    StructuredBuffer(size_t element_byte_size, size_t element_count, const void* data);

    void Bind(unsigned int index);

private:
    unsigned int id;
};
