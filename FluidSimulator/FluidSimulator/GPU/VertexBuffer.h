#pragma once
#include "DataType.h"

class VertexBuffer {
public:
    VertexBuffer();
    VertexBuffer(DataType data_type, size_t element_count);
    VertexBuffer(DataType data_type, size_t element_count, const void* initial_data);

    void Bind() const;

    void Draw(size_t count) const;

    // Assumes that the vertex buffer was bound before that
    void SetData(DataType data_type) const;

private:
    unsigned int VBO_ID;
    unsigned int VAO_ID;
    size_t element_count;
};
