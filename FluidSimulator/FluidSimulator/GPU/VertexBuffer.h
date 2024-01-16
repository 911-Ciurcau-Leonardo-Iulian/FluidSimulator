#pragma once

enum class VertexDataType {
    Float,
    Float2,
    Float3,
    Float4,
    Uint,
    Uint2,
    Uint3,
    Uint4,
    Int,
    Int2,
    Int3,
    Int4,
    UNorm,
    UNorm2,
    UNorm3,
    UNorm4,
    Norm,
    Norm2,
    Norm3,
    Norm4
};

unsigned int GetVertexDataTypeElemCount(VertexDataType type);

unsigned int GetVertexDataTypeNativeType(VertexDataType type);

bool IsVertexDataTypeNormalized(VertexDataType type);

size_t GetVertexDataTypeByteSize(VertexDataType type);

class VertexBuffer {
public:
    VertexBuffer();
    VertexBuffer(VertexDataType data_type, size_t element_count);
    VertexBuffer(VertexDataType data_type, size_t element_count, const void* initial_data);

    void Bind() const;

    void Draw(size_t count) const;

    // Assumes that the vertex buffer was bound before that
    void SetDataType(VertexDataType data_type) const;

private:
    unsigned int VBO_ID;
    unsigned int VAO_ID;
    size_t element_count;
};
