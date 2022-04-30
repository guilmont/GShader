#include <fstream>
#include <sstream>

#include "dynamicShader.h"

namespace fs = std::filesystem;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DynamicShader::DynamicShader(GRender::Mailbox* mailbox) : mail(mailbox) {
    const std::string shader =
        "#version 450 core                      \n"
        "layout(location = 0) in vec3 vPos;     \n"
        "layout(location = 2) in vec2 vTexCoord;\n"
        "out vec2 fragCoord;                    \n"
        "void main() {                          \n"
        "    fragCoord = vTexCoord;             \n"
        "    gl_Position = vec4(vPos, 1.0);     \n"
        "}                                      \n";
    
    
    vtxID = createShader(shader, GL_VERTEX_SHADER); // this shader is well tested and should be fine
}

DynamicShader::~DynamicShader(void) {
    glDeleteShader(vtxID);
    glDeleteProgram(programID);
}

DynamicShader::DynamicShader(DynamicShader&& rhs) noexcept {
    std::swap(programID, rhs.programID);
    std::swap(vtxID, rhs.vtxID);
    std::swap(mail, rhs.mail);
}


DynamicShader& DynamicShader::operator=(DynamicShader&& rhs) noexcept {
    if (&rhs != this){
        new(this) DynamicShader(std::move(rhs));
    }
    return *this;
}

void DynamicShader::loadShader(const fs::path& frgPath) {
    GRender::ASSERT(fs::exists(frgPath), "Shader not found! => " + frgPath.string());

    uint32_t frg = createShaderFromFile(frgPath, GL_FRAGMENT_SHADER);
    
    if (hasFailed()) {  // no point to continue
        return;
    }

    // Create program
    programID = glCreateProgram();
    glAttachShader(programID, vtxID);
    glAttachShader(programID, frg);

    // Link shaders to program
    glLinkProgram(programID);

    checkProgram(programID, GL_LINK_STATUS);

    glDeleteShader(frg);
}

bool DynamicShader::hasFailed() {
    return !success;
}

void DynamicShader::bind() {
    glUseProgram(programID);
}

void DynamicShader::setInteger(const char* name, int val) {
    int32_t loc = glGetUniformLocation(programID, name);
    glUniform1i(loc, val);
}

void DynamicShader::setFloat(const char* name, float val) {
    int32_t loc = glGetUniformLocation(programID, name);
    glUniform1f(loc, val);
}

void DynamicShader::setVec2f(const char* name, const float* v) {
    int32_t loc = glGetUniformLocation(programID, name);
    glUniform2f(loc, v[0], v[1]);
}

void DynamicShader::setVec3f(const char* name, const float* v) {
    int32_t loc = glGetUniformLocation(programID, name);
    glUniform3f(loc, v[0], v[1], v[2]);
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

uint32_t DynamicShader::createShaderFromFile(const fs::path& shaderPath, GLenum shaderType) {
    // Importing file into stream
    std::ifstream arq(shaderPath);
    std::stringstream strData;
    strData << arq.rdbuf();
    arq.close();

    return createShader(strData.str(), shaderType);
}

uint32_t DynamicShader::createShader(const std::string& shaderData, GLenum shaderType) {
    // Creating shader from data
    uint32_t shader = glCreateShader(shaderType);
    GRender::ASSERT(shader != 0, "Failed to create shader!");

    GLint length = (GLint) shaderData.size();
    const GLchar* ptr = shaderData.c_str();
    glShaderSource(shader, 1, &ptr, &length);
    glCompileShader(shader);

    checkShader(shader, GL_COMPILE_STATUS);

    return shader;
}

void DynamicShader::checkShader(uint32_t shader, uint32_t flag) {
    int tag = 0;
    glGetShaderiv(shader, flag, &tag);

    if (tag == GL_FALSE) {
        char error[1024] = { 0 }; // arbitrary large size
        glGetShaderInfoLog(shader, sizeof(error), NULL, error);

        success = false;
        mail->createError("Shader compilation error => " + std::string(error));
    } 
    else {
        success = true;
    }
}

void DynamicShader::checkProgram(uint32_t id, uint32_t flag) {
    int tag = 0;
    glGetProgramiv(id, flag, &tag);

    if (tag == GL_FALSE) {
        char error[1024] = { 0 }; // arbitrary large size
        glGetProgramInfoLog(id, sizeof(error), NULL, error);

        success = false;
        mail->createError("Cannot link shader program => " + std::string(error));
    } 
    else {
        success = true;
    }
}

