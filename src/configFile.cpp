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
    data["relativePath"] = relShaderPath.string();
}

template<>
void ConfigFile::get(fs::path& relShaderPath) {
    bool check = data.contains("relativePath");
    GRender::ASSERT(check, "Configuration file doesn't contain a relative path!");
    if (!check) return;

    json& relPath = data["relativePath"];
    if (!relPath.is_null()) 
        relShaderPath = relPath.get<fs::path>();
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
void ConfigFile::get(Colors& colors) {
    bool check = data.contains("colors");
    GRender::ASSERT(check, "Configuration file doesn't contain color data!");
    if (!check) return;
    
    json& aux = data["colors"];
    if (aux.is_null()) return;

    for (const auto& [name, cor] : aux.items()) {
        colors.append(name, {cor[0].get<float>(), 
                             cor[1].get<float>(),
                             cor[2].get<float>()});
    }
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
void ConfigFile::get(GRender::Camera& cam) {
    bool check = data.contains("camera");
    GRender::ASSERT(check, "Configuration file doesn't contain camera data!");
    if(!check) return;

    json& aux = data["camera"];
    if (aux.is_null()) return;

    json& _pos = aux["position"];
    glm::vec3 pos = {_pos[0].get<float>(),
                     _pos[1].get<float>(),
                     _pos[2].get<float>()};

    cam.setDefaultPosition(pos);
    cam.setPosition(pos);

    float pitch = aux["pitch"].get<float>();
    cam.setDefaultPitch(pitch);
    cam.setPitch(pitch);

    float yaw = aux["yaw"].get<float>();
    cam.setDefaultYaw(yaw);
    cam.setYaw(yaw);

    cam.setFOV(aux["fov"].get<float>());
}


