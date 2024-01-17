#include "VertexBuffer.h"
#include "glad.h"

VertexBuffer::VertexBuffer() : VAO_ID(-1), VBO_ID(-1), element_count(-1) {}

VertexBuffer::VertexBuffer(DataType data_type, size_t element_count) : element_count(element_count)
{
    glGenVertexArrays(1, &VAO_ID);
    glBindVertexArray(VAO_ID);

    glGenBuffers(1, &VBO_ID);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_ID);
    glBufferData(GL_ARRAY_BUFFER, GetDataTypeByteSize(data_type) * element_count, nullptr, GL_DYNAMIC_DRAW);
    SetData(data_type);
    
}

VertexBuffer::VertexBuffer(DataType data_type, size_t element_count, const void* initial_data) : element_count(element_count)
{
    glGenVertexArrays(1, &VAO_ID);
    glBindVertexArray(VAO_ID);

    glGenBuffers(1, &VBO_ID);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_ID);
    glBufferData(GL_ARRAY_BUFFER, GetDataTypeByteSize(data_type) * element_count, initial_data, GL_STATIC_DRAW);
    SetData(data_type);
}

void VertexBuffer::Bind() const
{
    glBindVertexArray(VAO_ID);
}

void VertexBuffer::Draw(size_t count) const
{
    if (count == 1) {
        glDrawArrays(GL_TRIANGLES, 0, element_count);
    }
    else {
        glDrawArraysInstanced(GL_TRIANGLES, 0, element_count, count);
    }
}

void VertexBuffer::SetData(DataType data_type) const
{
    glVertexAttribPointer(
        0,
        GetDataTypeElemCount(data_type),
        GetDataTypeNativeType(data_type),
        IsDataTypeNormalized(data_type),
        GetDataTypeByteSize(data_type),
        (void*)0
    );
    glEnableVertexAttribArray(0);
}
