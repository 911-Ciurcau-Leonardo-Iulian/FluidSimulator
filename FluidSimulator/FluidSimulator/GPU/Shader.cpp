#include "Shader.h"
#include "glad.h"
#include <iostream>
#include <stdio.h>

Shader::Shader(const char* vertex_path, const char* pixel_path)
{
    int success;
    char info_log[2048];

    // The vertex shader
    FILE* vertex_file = fopen(vertex_path, "rt");
    bool has_vertex_shader = false;
    unsigned int vertex = -1;
    if (vertex_file != NULL) {
        char shader_file[1024 * 32];
        size_t read_size = fread(shader_file, sizeof(char), sizeof(shader_file), vertex_file);

        if (read_size > 0) {
            const char* shader_data = shader_file;
            int int_read_size = read_size;

            vertex = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertex, 1, &shader_data, &int_read_size);
            glCompileShader(vertex);
            glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(vertex, sizeof(info_log), NULL, info_log);
                std::cout << "Vertex Shader Compilation error:\n" << info_log << "\n";
                glDeleteShader(vertex);
            }
            else {
                has_vertex_shader = true;
            }
        }
        fclose(vertex_file);
    }

    // The pixel shader
    FILE* pixel_file = fopen(pixel_path, "rt");
    if (pixel_file != NULL && has_vertex_shader) {
        char shader_file[1024 * 32];
        size_t read_size = fread(shader_file, sizeof(char), sizeof(shader_file), pixel_file);

        if (read_size > 0) {
            const char* shader_data = shader_file;
            int int_read_size = read_size;

            unsigned int pixel = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(pixel, 1, &shader_data, &int_read_size);
            glCompileShader(pixel);
            glGetShaderiv(pixel, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(pixel, sizeof(info_log), NULL, info_log);
                std::cout << "Pixel Shader Compilation error:\n" << info_log << "\n";
            }
            else {
                // The entire shader program
                ID = glCreateProgram();
                glAttachShader(ID, vertex);
                glAttachShader(ID, pixel);
                glLinkProgram(ID);
                glGetProgramiv(ID, GL_LINK_STATUS, &success);
                if (!success)
                {
                    glGetProgramInfoLog(ID, sizeof(info_log), NULL, info_log);
                    std::cout << "Shader Program Linking error:\n" << info_log << "\n";
                }
            }

            glDeleteShader(vertex);
            glDeleteShader(pixel);
        }
    }
}

void Shader::Use() const
{
    glUseProgram(ID);
}

void Shader::SetBool(const char* name, bool value) const
{
    SetInt(name, (int)value);
}

void Shader::SetInt(const char* name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name), value);
}

void Shader::SetFloat(const char* name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name), value);
}

void Shader::SetTexture(const char* name, unsigned int slot) const
{
    glUniform1i(glGetUniformLocation(ID, name), slot);
}
