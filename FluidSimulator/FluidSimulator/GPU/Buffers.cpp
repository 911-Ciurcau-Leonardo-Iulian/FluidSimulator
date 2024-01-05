#include "Buffers.h"
#include "glad.h"

StructuredBuffer::StructuredBuffer(size_t element_byte_size, size_t element_count)
{
    glGenBuffers(1, &id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, element_byte_size * element_count, nullptr, GL_DYNAMIC_DRAW);
}

StructuredBuffer::StructuredBuffer(size_t element_byte_size, size_t element_count, const void* data)
{
    glGenBuffers(1, &id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, element_byte_size * element_count, data, GL_DYNAMIC_DRAW);
}

void StructuredBuffer::Bind(unsigned int index)
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, id);
}
