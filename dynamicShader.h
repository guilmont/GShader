#pragma once

#include "glad/glad.h"
#include "mailbox.h"

class DynamicShader {
public:
    DynamicShader(void) = default;
    ~DynamicShader(void);

    DynamicShader(const DynamicShader&) = delete;
    DynamicShader& operator=(const DynamicShader&) = delete;
    
    DynamicShader(DynamicShader&&) noexcept;
    DynamicShader& operator=(DynamicShader&&) noexcept;


    void initialize(void);
    void loadShader(const std::filesystem::path& frgPath);
    bool hasFailed(void);
    void bind(void);

    void setInteger(const char*, int);
    void setFloat(const char*, float);
    void setVec2f(const char*, const float*);
    void setVec3f(const char*, const float*);


private:
    uint32_t createShaderFromFile(const std::filesystem::path& shaderPath, GLenum shaderType);
    uint32_t createShader(const std::string& shaderData, GLenum shaderType);
    void checkShader(uint32_t id, uint32_t flag);
    void checkProgram(uint32_t id, uint32_t flag);

private:
    bool success = false; // determines if shader was loaded correctly

    uint32_t
        programID = 0,   // id used to bind shader
        vtxID = 0;       // vertex compilation id
};

