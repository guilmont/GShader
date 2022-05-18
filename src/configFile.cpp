#include "configFile.h"

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


