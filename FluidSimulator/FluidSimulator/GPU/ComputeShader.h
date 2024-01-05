#pragma once
#include <vector>
#include <string>

struct UniformBlock {
    std::string name;
    size_t embedded_data[128];
};

class ComputeShader {
public:
    ComputeShader() = default;
    ComputeShader(const char* path, unsigned int group_size_x, unsigned int group_size_y, unsigned int group_size_z);

    void Bind() const;

    void BindAndDispatch(unsigned int dimension_x, unsigned int dimension_y, unsigned int dimension_z) const;

    void Dispatch(unsigned int dimension_x, unsigned int dimension_y, unsigned int dimension_z) const;

    void CreateUniformBlock(const char* name, size_t byte_size);

    void* GetUniformBlockData(const char* name);

    template<typename T>
    void SetUniformBlock(const char* name, const T* data) {
        void* uniform_data = GetUniformBlockData(name);
        memcpy(uniform_data, data, sizeof(*data));
    }


private:
    std::vector<UniformBlock> uniform_blocks;
    unsigned int program_id;
    unsigned int group_size_x;
    unsigned int group_size_y;
    unsigned int group_size_z;
};
