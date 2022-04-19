#pragma once

#include <filesystem>

#include "glad/glad.h"
#include "mailbox.h"

class DynamicShader {
public:
    DynamicShader(void) = default;
    DynamicShader(GRender::Mailbox* mailbox);
    ~DynamicShader(void);

    DynamicShader(const DynamicShader&) = delete;
    DynamicShader& operator=(const DynamicShader&) = delete;
    
    DynamicShader(DynamicShader&&) noexcept;
    DynamicShader& operator=(DynamicShader&&) noexcept;

    void loadShader(const std::filesystem::path& frgPath);
    bool isSuccessful(void);
    bool hasFailed(void);
    void bind(void);

    void setInteger(const std::string&, int);
    void setFloat(const std::string&, float);

private:
    uint32_t createShader(const std::filesystem::path& shaderPath, GLenum shaderType);
    void checkShader(uint32_t id, uint32_t flag);
    void checkProgram(uint32_t id, uint32_t flag);

private:
    bool success = false; // determines if shader was loaded correctly

    uint32_t
        programID = 0,   // id used to bind shader
        vtxID = 0;       // vertex compilation id

    GRender::Mailbox* mail = nullptr;


   
};

