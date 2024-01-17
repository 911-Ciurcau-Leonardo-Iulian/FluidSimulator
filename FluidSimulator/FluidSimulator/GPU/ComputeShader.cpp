#include "ComputeShader.h"
#include "glad.h"
#include <fstream>
#include <iostream>

#define MAX_SHADER_SIZE 1024 * 1024

static void CheckCompileErrors(unsigned int id, bool is_program_type)
{
    int success;
    char info_log[1024];
    if (!is_program_type)
    {
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(id, sizeof(info_log), NULL, info_log);
            std::cout << "Shader compilation error\n" << info_log << "\n-------------------------------------------------------\n";
            abort();
        }
    }
    else
    {
        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(id, sizeof(info_log), NULL, info_log);
            std::cout << "Program link error\n" << info_log << "\n -------------------------------------------------------\n";
            abort();
        }
    }
}

ComputeShader::ComputeShader(const char* path, unsigned int _group_size_x, unsigned int _group_size_y, unsigned int _group_size_z) {
    char* file_allocation = (char*)malloc(sizeof(char) * MAX_SHADER_SIZE);
    std::ifstream file_stream(path);
    file_stream.read(file_allocation, MAX_SHADER_SIZE);

    size_t read_count = file_stream.gcount();
    if (read_count != -1) {
        unsigned int shader_id = glCreateShader(GL_COMPUTE_SHADER);
        int shader_size = read_count;
        glShaderSource(shader_id, 1, &file_allocation, &shader_size);
        glCompileShader(shader_id);
        CheckCompileErrors(shader_id, false);

        program_id = glCreateProgram();
        glAttachShader(program_id, shader_id);
        glLinkProgram(program_id);
        CheckCompileErrors(program_id, true);
        glDeleteShader(shader_id);
    }
    else {
        abort();
    }
    group_size_x = _group_size_x;
    group_size_y = _group_size_y;
    group_size_z = _group_size_z;
}

void ComputeShader::BindUniformBlock(size_t index, unsigned int bind_index)
{
    if (uniform_blocks[index].dirty) {
        glBindBuffer(GL_UNIFORM_BUFFER, uniform_blocks[index].ID);
        glBufferData(GL_UNIFORM_BUFFER, uniform_blocks[index].byte_size, uniform_blocks[index].embedded_data, GL_DYNAMIC_DRAW);
        //glMemoryBarrier(GL_UNIFORM_BARRIER_BIT);
        uniform_blocks[index].dirty = false;
    }
    glBindBufferBase(GL_UNIFORM_BUFFER, bind_index != -1 ? bind_index : index, uniform_blocks[index].ID);
}

void ComputeShader::BindAllUniformBlocks()
{
    for (size_t index = 0; index < uniform_blocks.size(); index++) {
        BindUniformBlock(index);
    }
}

void ComputeShader::Bind(bool bind_uniform_blocks)
{
    glUseProgram(program_id);
    if (bind_uniform_blocks) {
        BindAllUniformBlocks();
    }
}

void ComputeShader::BindAndDispatch(unsigned int dimension_x, unsigned int dimension_y, unsigned int dimension_z, bool bind_uniform_blocks) {
    Bind(bind_uniform_blocks);
    Dispatch(dimension_x, dimension_y, dimension_z);
}

void ComputeShader::Dispatch(unsigned int dimension_x, unsigned int dimension_y, unsigned int dimension_z) const {
    int group_count_x = ceilf(dimension_x / (float)group_size_x);
    int group_count_y = ceilf(dimension_y / (float)group_size_y);
    int group_count_z = ceilf(dimension_z / (float)group_size_z);
    glDispatchCompute(group_count_x, group_count_y, group_count_z);
    // This will ensure that the compute shader has finished before other commands are issued
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void ComputeShader::CreateUniformBlock(const char* name, size_t byte_size)
{
    UniformBlock uniform_block;
    if (byte_size > sizeof(uniform_block.embedded_data)) {
        abort();
    }
    uniform_block.name = name;
    uniform_block.dirty = false;
    uniform_block.byte_size = byte_size;
    uniform_blocks.push_back(std::move(uniform_block));
    size_t last_index = uniform_blocks.size() - 1;

    glGenBuffers(1, &uniform_blocks[last_index].ID);
    glBindBuffer(GL_UNIFORM_BUFFER, uniform_blocks[last_index].ID);
    glBufferData(GL_UNIFORM_BUFFER, byte_size, uniform_blocks[last_index].embedded_data, GL_DYNAMIC_DRAW);
}

size_t ComputeShader::GetUniformBlockIndex(const char* name) const
{
    for (size_t index = 0; index < uniform_blocks.size(); index++) {
        if (uniform_blocks[index].name == name) {
            return index;
        }
    }
    return -1;
}

void* ComputeShader::GetUniformBlockData(const char* name)
{
    size_t index = GetUniformBlockIndex(name);
    if (index == -1) {
        return nullptr;
    }
    return uniform_blocks[index].embedded_data;
}

void ComputeShader::SetUniformBlockDirty(const char* name)
{
    size_t index = GetUniformBlockIndex(name);
    if (index == -1) {
        abort();
    }
    uniform_blocks[index].dirty = true;
}
