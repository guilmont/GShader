#include "dynamicShader.h"

namespace fs = std::filesystem;

DynamicShader::~DynamicShader(void) {
    glDeleteShader(vtxID);
    glDeleteProgram(programID);
}

DynamicShader::DynamicShader(DynamicShader&& rhs) noexcept {
    std::swap(programID, rhs.programID);
    std::swap(vtxID, rhs.vtxID);
}


DynamicShader& DynamicShader::operator=(DynamicShader&& rhs) noexcept {
    if (&rhs != this){
        new(this) DynamicShader(std::move(rhs));
    }
    return *this;
}

void DynamicShader::initialize() {
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

void DynamicShader::loadShader(const fs::path& frgPath) {
    GRender::ASSERT(vtxID > 0, "DynamicShader was not initialized!");
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


bool DynamicShader::wasUpdated() {
    // if any file was touched since last check
    for (auto& [filepath, data] : fileMap) {
        if (data.modTime != fs::last_write_time(filepath))
            return true;
    }
    return false;
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

bool DynamicShader::recurseFiles(const fs::path& shadername) {
    fs::path shaderpath = location / shadername;

    fs::path fLocation(location); // storing location, in case we need to move to another folder
    location  = shaderpath.parent_path();

    // the first file is always guaranteed to exist when using the dialog box
    if (!fs::exists(shaderpath)) {
        GRender::mailbox::CreateError( "\"" + shadername.string() + "\" doesn't exist!");
        return false;
    }

    // We also keep a variable with moditication time for automatic update
    fileMap[shaderpath.string()].modTime = fs::last_write_time(shaderpath);

    int32_t lineZero = numLines+1; // coding lines usually start at 1

    // reading file line by line and checking if it is a include line or normal program
    std::string line;
    std::ifstream arq(shaderpath);
    while(std::getline(arq, line)) {
        size_t pos = line.find("#include");
        
        // We have an include file
        if (pos < line.size()) {
            // finding the quotes for the header name
            size_t qt1 = line.find('\"', pos) + 1;
            size_t qt2 = line.find('\"', qt1);

            fs::path newPath = line.substr(qt1, qt2 - qt1);
            
            if (qt1 == std::string::npos || qt2 == std::string::npos) {
                std::string error = shadername.string() + " => " + std::to_string(numLines - lineZero+2)
                                  + "(9) Header file not found: '" + line + "'";

                GRender::mailbox::CreateError(error);
                return false;
            }

            // If the header was already included, no need to do another time
            if (fileMap.find((location / newPath).string()) != fileMap.end()) {
                program += "\n";
                // numLines++;
            } 
            // passing header to be recursed
            else if (!recurseFiles(newPath)) {
                return false;
            }

            // setup line zero for normal program
            lineZero = numLines;
        }
        // normal program
        else {
            program += line + "\n";
            numLines++;
        }
    }
    arq.close();
    
    // Restoring original location
    location = fLocation;

    // Getting range of lines in final program for this file
    fileMap[shaderpath.string()].range = {lineZero, numLines};

    return true;
}

uint32_t DynamicShader::createShaderFromFile(const fs::path& shaderPath, GLenum shaderType) {
    numLines = 0;
    program.clear();
    fileMap.clear();
    location = shaderPath.parent_path();

    success = recurseFiles(shaderPath.filename());
    if (!success)
        return 0;

    return createShader(program, shaderType);
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
        std::string errorMessage = "Shader compilation error\n";

        // remapping error to correct file and line
        std::string line;
        std::stringstream err(error);
        while(std::getline(err, line)) {
            size_t pos = line.find_first_of('(');
            int32_t num = atoi(line.substr(2, pos - 2).c_str());

            for (auto& [name, data] : fileMap) {
                if (num >= data.range.first && num <= data.range.second) {
                    size_t size = location.string().size()+1;
                    errorMessage += name.substr(size) + " => " + std::to_string(num - data.range.first);
                    errorMessage += line.substr(pos) + "\n";
                    break;
                }
            }
        }

        GRender::mailbox::CreateError(errorMessage);
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
        GRender::mailbox::CreateError("Cannot link shader program => " + std::string(error));
    } 
    else {
        success = true;
    }
}

