#pragma once

class StructuredBuffer {
public:
    StructuredBuffer() = default;
    StructuredBuffer(size_t element_byte_size, size_t element_count);
    StructuredBuffer(size_t element_byte_size, size_t element_count, const void* data);

    void Bind(unsigned int index) const;

    void SetNewDataSize(size_t element_byte_size, size_t element_count) const;

    void SetNewData(size_t element_byte_size, size_t element_count, const void* data) const;

private:
    unsigned int id;
};
