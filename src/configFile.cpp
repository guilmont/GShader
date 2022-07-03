#include "configFile.h"
#include "mailbox.h"

#include "colors.h"
#include "camera.h"
#include "uniforms.h"

#include <fstream>

namespace fs = std::filesystem;

ConfigFile::ConfigFile(const fs::path& configpath) : configpath(configpath) {
}

void ConfigFile::load() {
    GRender::ASSERT(fs::exists(configpath), "'" + configpath.string() + "' doesn't exist!");

    data.clear();

    std::ifstream arq(configpath);
    arq >> data;
    arq.close();
}

void ConfigFile::save() {
    std::ofstream arq(configpath);
    arq << data.dump(4);
    arq.close();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Shader path
template<>
void ConfigFile::insert(const fs::path& relShaderPath) {
    std::string var = relShaderPath.string();
    std::replace(var.begin(), var.end(), '\\','/');
    data["relativePath"] = var;
}

template<>
fs::path ConfigFile::get() {
    json& relPath = data["relativePath"];
    if (relPath.is_null()) {
        GRender::mailbox::CreateWarn("Configuration file set to null!");
        return fs::path();
    }
    else {
        return configpath.parent_path() / relPath.get<fs::path>();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
// Colors 
template<>
void ConfigFile::insert(const Colors& colors) {
    json& vec = data["colors"];
    for (const auto& [name, cor] : colors) {
        vec[name.c_str() + 2] = {cor.r, cor.g, cor.b};
    }
}

template<>
Colors ConfigFile::get() {
    json& aux = data["colors"];
    if (aux.is_null()) { 
        return Colors(); 
    }

    Colors colors;
    for (const auto& [name, cor] : aux.items()) {
        colors.append(name, {cor[0].get<float>(), 
                             cor[1].get<float>(),
                             cor[2].get<float>()});
    }

    return colors;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Camera
template<>
void ConfigFile::insert(const GRender::Camera& cam) {
    json& aux = data["camera"];

    const glm::vec3& pos = cam.getDefaultPosition();
    aux["position"] = {pos.x, pos.y, pos.z};
    aux["pitch"] = cam.getDefaultPitch();
    aux["yaw"] = cam.getYaw();
    aux["fov"] = cam.getFOV();
}

template<>
GRender::Camera ConfigFile::get() {
    json& aux = data["camera"];
    if (aux.is_null()) { return GRender::Camera(); }

    json& _pos = aux["position"];
    glm::vec3 pos = {_pos[0].get<float>(),
                     _pos[1].get<float>(),
                     _pos[2].get<float>()};


    GRender::Camera cam;
    cam.setDefaultPosition(pos);
    cam.setPosition(pos);

    float pitch = aux["pitch"].get<float>();
    cam.setDefaultPitch(pitch);
    cam.setPitch(pitch);

    float yaw = aux["yaw"].get<float>();
    cam.setDefaultYaw(yaw);
    cam.setYaw(yaw);

    cam.setFOV(aux["fov"].get<float>());

    return cam;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Uniforms

template<>
void ConfigFile::insert(const uniform::Uniform& unif) {
    using namespace uniform;
    
    json& vec = data["uniforms"];
    for (const auto& [tag, data] : unif) {
        json& var = vec[tag.substr(2)];
        var["type"] = static_cast<int32_t>(data->tp);

        switch (data->tp) {
        case Type::INT: {
            const DataInt* ptr = reinterpret_cast<const DataInt*>(data.get());
            var["range"] = { ptr->range.x, ptr->range.y };
            var["data"] = ptr->data.x;
            break;
        }
        case Type::IVEC2: {
            const DataInt2* ptr = reinterpret_cast<const DataInt2*>(data.get());
            var["range"] = { ptr->range.x, ptr->range.y };
            var["data"] = { ptr->data.x, ptr->data.y };
            break;
        }
        case Type::IVEC3: {
            const DataInt3* ptr = reinterpret_cast<const DataInt3*>(data.get());
            var["range"] = { ptr->range.x, ptr->range.y };
            var["data"] = { ptr->data.x, ptr->data.y, ptr->data.z };
            break;
        }
        case Type::IVEC4: {
            const DataInt4* ptr = reinterpret_cast<const DataInt4*>(data.get());
            var["range"] = { ptr->range.x, ptr->range.y };
            var["data"] = { ptr->data.x, ptr->data.y, ptr->data.z, ptr->data.w };
            break;
        }
        case Type::FLOAT: {
            const DataFloat* ptr = reinterpret_cast<const DataFloat*>(data.get());
            var["range"] = { ptr->range.x, ptr->range.y };
            var["data"] = ptr->data.x;
            break;
        }
        case Type::VEC2: {
            const DataFloat2* ptr = reinterpret_cast<const DataFloat2*>(data.get());
            var["range"] = { ptr->range.x, ptr->range.y };
            var["data"] = { ptr->data.x, ptr->data.y };
            break;
        }
        case Type::VEC3: {
            const DataFloat3* ptr = reinterpret_cast<const DataFloat3*>(data.get());
            var["range"] = { ptr->range.x, ptr->range.y };
            var["data"] = { ptr->data.x, ptr->data.y, ptr->data.z };
            break;
        }
        default: {
            const DataFloat4* ptr = reinterpret_cast<const DataFloat4*>(data.get());
            var["range"] = { ptr->range.x, ptr->range.y };
            var["data"] = { ptr->data.x, ptr->data.y, ptr->data.z, ptr->data.w };
            break;
        }
        } // switch
    }
}

template<>
uniform::Uniform ConfigFile::get() {
    using namespace uniform;

    json& aux = data["uniforms"];
    if (aux.is_null()) {
        return Uniform();
    }
  
    Uniform unif;
    for (const auto& [name, var] : aux.items()) {
        // Type and tag
        const std::string tag = "##" + name;
        Type tp = static_cast<Type>(var["type"].get<int32_t>());

        ///////////////////////////////
        // Getting range data
        glm::ivec2 irg;
        glm::vec2 frg;
        json& range = var["range"];
        if (tp >= Type::INT && tp <= Type::IVEC4) {
            irg = { range[0].get<int32_t>(), range[1].get<int32_t>() };
        }
        else {
            frg = { range[0].get<float>(), range[1].get<float>() };
        }

        /////////////////////////////
        //Getting values
        json& val = var["data"];
        switch (tp) {
        case Type::INT: {
            glm::ivec1 ptr(val.get<int32_t>());
            unif.append(tag, new DataInt(tag, tp, irg, ptr));
            break;
        }
        case Type::IVEC2: {
            glm::ivec2 ptr = { val[0].get<int32_t>(), val[1].get<int32_t>() };
            unif.append(tag, new DataInt2(tag, tp, irg, ptr));
            break;
        }
        case Type::IVEC3: {
            glm::ivec3 ptr = { val[0].get<int32_t>(), val[1].get<int32_t>(), val[2].get<int32_t>() };
            unif.append(tag, new DataInt3(tag, tp, irg, ptr));
            break;
        }
        case Type::IVEC4: {
            glm::ivec4 ptr = { val[0].get<int32_t>(), val[1].get<int32_t>(),
                               val[2].get<int32_t>(), val[3].get<int32_t>() };
            unif.append(tag, new DataInt4(tag, tp, irg, ptr));
            break;
        }
        case Type::FLOAT: {
            glm::vec1 ptr(val.get<float>());
            unif.append(tag, new DataFloat(tag, tp, frg, ptr));
            break;
        }
        case Type::VEC2: {
            glm::vec2 ptr = { val[0].get<float>(), val[1].get<float>() };
            unif.append(tag, new DataFloat2(tag, tp, frg, ptr));
            break;
        }
        case Type::VEC3: {
            glm::vec3 ptr = { val[0].get<float>(), val[1].get<float>(), val[2].get<float>() };
            unif.append(tag, new DataFloat3(tag, tp, frg, ptr));
            break;
        }
        default: {
            glm::vec4 ptr = { val[0].get<float>(), val[1].get<float>(),
                               val[2].get<float>(), val[3].get<float>() };
            unif.append(tag, new DataFloat4(tag, tp, frg, ptr));
            break;
        }
        } // switch
    }   
    
    return unif;
}