#pragma once
#include <vector>
#include <string>

struct UniformBlock {
    std::string name;
    size_t embedded_data[128];
    unsigned int ID;
    unsigned int attribute_location;
    unsigned int byte_size;
    bool dirty;
};

class ComputeShader {
public:
    ComputeShader() : program_id(-1) {}
    ComputeShader(const char* path, unsigned int group_size_x, unsigned int group_size_y, unsigned int group_size_z);

    // If the bind index is left at -1, it will assume that it will be bound at the same index as in the array
    void BindUniformBlock(size_t index, unsigned int bind_index = -1);

    void BindAllUniformBlocks();

    void Bind(bool bind_uniform_blocks = true);

    void BindAndDispatch(unsigned int dimension_x, unsigned int dimension_y, unsigned int dimension_z, bool bind_uniform_blocks = true);

    void Dispatch(unsigned int dimension_x, unsigned int dimension_y, unsigned int dimension_z) const;

    void CreateUniformBlock(const char* name, size_t byte_size);

    size_t GetUniformBlockIndex(const char* name) const;

    void* GetUniformBlockData(const char* name);

    template<typename T>
    void SetUniformBlock(const char* name, const T* data) {
        size_t uniform_block_index = GetUniformBlockIndex(name);
        void* uniform_data = uniform_blocks[uniform_block_index].embedded_data;
        memcpy(uniform_data, data, sizeof(*data));
        uniform_blocks[uniform_block_index].dirty = true;
    }

    void SetUniformBlockDirty(const char* name);

    void SetBool(const char* name, bool value) const;

    void SetInt(const char* name, int value) const;

    void SetFloat(const char* name, float value) const;


private:
    std::vector<UniformBlock> uniform_blocks;
    unsigned int program_id;
    unsigned int group_size_x;
    unsigned int group_size_y;
    unsigned int group_size_z;
};
