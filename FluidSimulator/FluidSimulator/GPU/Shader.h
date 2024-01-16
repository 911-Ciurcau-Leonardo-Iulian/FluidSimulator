#pragma once

class Shader {
public:
    Shader() { ID = -1; }
    Shader(const char* vertex_path, const char* pixel_path);

    void Use() const;

    void SetBool(const char* name, bool value) const;

    void SetInt(const char* name, int value) const;

    void SetFloat(const char* name, float value) const;

private:
    unsigned int ID;
};
