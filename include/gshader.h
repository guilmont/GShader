#include "GRender/application.h"
#include "dynamicShader.h"
#include "colors.h"
#include "uniforms.h"

#include "configFile.h"

using namespace GRender;
namespace fs = std::filesystem;

template <typename TP>
class Ref {
public:
	bool active = false;
	TP* operator->() { return &var; }
	TP& operator*() { return var; }

private:
	TP var;
};

class GShader : public Application {
	using Uniform = uniform::Uniform;
	using QuadSpecs = quad::Specs;
	using Quad = quad::Quad;

public:
	GShader(const fs::path& filepath);
	~GShader(void) = default;

	void onUserUpdate(float deltaTime) override;
	void ImGuiLayer(void) override;
	void ImGuiMenuLayer(void) override;

	void importShader(const fs::path& shaderpath);

	void loadConfig(const fs::path& configpath);
	void saveConfig(const fs::path& configpath);

private:
	fs::path currentShader;
	float elapsedTime = 0.0f;

	bool view_specs = false;
	bool ctrlPlay = true;
	bool ctrlReset = false;
	bool ctrlStep = false;

	Quad quad;
	QuadSpecs specs;

	Uniform uniforms;
	Colors colors;
	Camera camera;
	DynamicShader shader;

	Ref<Framebuffer> fbuffer;
};
