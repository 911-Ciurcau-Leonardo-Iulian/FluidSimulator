#include "ComputeShader.h"
#include "glad.h"
#include <fstream>
#include <iostream>

#define MAX_SHADER_SIZE 1024 * 1024

static void CheckCompileErrors(unsigned int shader, bool program_type)
{
    int success;
    char infoLog[1024];
    if (program_type)
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR\n" << infoLog << "\n-------------------------------------------------------\n";
            abort();
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type:\n" << infoLog << "\n ------------------------------------------------------- ";
            abort();
        }
    }
}

ComputeShader::ComputeShader(const char* path) {
    char* file_allocation = (char*)malloc(sizeof(char) * MAX_SHADER_SIZE);
    std::ifstream file_stream(path);
    file_stream.read(file_allocation, MAX_SHADER_SIZE);

    if (file_stream.good()) {
        size_t read_count = file_stream.gcount();
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
}

void ComputeShader::Bind() const
{
    glUseProgram(program_id);
}

void ComputeShader::CreateUniformBlock(const char* name, size_t byte_size)
{
    UniformBlock uniform_block;
    if (byte_size > sizeof(uniform_block.embedded_data)) {
        abort();
    }
    uniform_block.name = name;
    uniform_blocks.push_back(std::move(uniform_block));
}

void* ComputeShader::GetUniformBlockData(const char* name)
{
    for (size_t index = 0; index < uniform_blocks.size(); index++) {
        if (uniform_blocks[index].name == name) {
            return uniform_blocks[index].embedded_data;
        }
    }
    return nullptr;
}
