#include "VertexBuffer.h"
#include "glad.h"

unsigned int GetVertexDataTypeElemCount(VertexDataType type)
{
#define CASE(count) case VertexDataType::Float##count:\
                    case VertexDataType::Int##count:\
                    case VertexDataType::Uint##count:\
                    case VertexDataType::Norm##count:\
                    case VertexDataType::UNorm##count:

    switch (type) {
        CASE()
            return 1;
        CASE(2)
            return 2;
        CASE(3)
            return 3;
        CASE(4)
            return 4;
    }

    return -1;

#undef CASE

}

unsigned int GetVertexDataTypeNativeType(VertexDataType type)
{
#define CASE(type)  case VertexDataType::type:\
                    case VertexDataType::type##2:\
                    case VertexDataType::type##3:\
                    case VertexDataType::type##4:

    switch (type) {
        CASE(Float)
            return GL_FLOAT;
        CASE(Int)
        CASE(Norm)
            return GL_INT;
        CASE(Uint)
        CASE(UNorm)
            return GL_UNSIGNED_INT;
    }

    return -1;

#undef CASE
}

bool IsVertexDataTypeNormalized(VertexDataType type)
{
    switch (type) {
    case VertexDataType::Norm:
    case VertexDataType::Norm2:
    case VertexDataType::Norm3:
    case VertexDataType::Norm4:
    case VertexDataType::UNorm:
    case VertexDataType::UNorm2:
    case VertexDataType::UNorm3:
    case VertexDataType::UNorm4:
        return true;
    }

    return false;
}

size_t GetVertexDataTypeByteSize(VertexDataType type)
{
    // At the moment, all types are either floats or 32bit integers
    return GetVertexDataTypeElemCount(type) * sizeof(float);
}

VertexBuffer::VertexBuffer() : VAO_ID(-1), VBO_ID(-1), element_count(-1) {}

VertexBuffer::VertexBuffer(VertexDataType data_type, size_t element_count) : element_count(element_count)
{
    glGenVertexArrays(1, &VAO_ID);
    glBindVertexArray(VAO_ID);

    glGenBuffers(1, &VBO_ID);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_ID);
    glBufferData(GL_ARRAY_BUFFER, GetVertexDataTypeByteSize(data_type) * element_count, nullptr, GL_DYNAMIC_DRAW);
    SetDataType(data_type);
    
}

VertexBuffer::VertexBuffer(VertexDataType data_type, size_t element_count, const void* initial_data) : element_count(element_count)
{
    glGenVertexArrays(1, &VAO_ID);
    glBindVertexArray(VAO_ID);

    glGenBuffers(1, &VBO_ID);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_ID);
    glBufferData(GL_ARRAY_BUFFER, GetVertexDataTypeByteSize(data_type) * element_count, initial_data, GL_STATIC_DRAW);
    SetDataType(data_type);
}

void VertexBuffer::Bind() const
{
    glBindVertexArray(VAO_ID);
}

void VertexBuffer::Draw(size_t count) const
{
    glDrawArrays(GL_TRIANGLES, 0, element_count * count);
}

void VertexBuffer::SetDataType(VertexDataType data_type) const
{
    glVertexAttribPointer(
        0,
        GetVertexDataTypeElemCount(data_type),
        GetVertexDataTypeNativeType(data_type),
        IsVertexDataTypeNormalized(data_type),
        GetVertexDataTypeByteSize(data_type),
        (void*)0
    );
}
