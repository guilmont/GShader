#include "GRender.h"

using namespace GRender;
namespace fs = std::filesystem;

class GShader : public Application
{
public:
	GShader(void);
	~GShader(void) = default;

	void onUserUpdate(float deltaTime) override;
	void ImGuiLayer(void) override;
	void ImGuiMenuLayer(void) override;

private:
	float elapsedTime = 0.0f;
	fs::path shaderPath = "../assets/basic.glsl";
	fs::file_time_type modTime;  // used to reload shader if if was modified
	
	void loadShader();

private:
	bool view_specs = false;

	quad::Specs specs;
	quad::Quad quad;

	Shader shader;
	Framebuffer fbuffer;
	
};

GRender::Application* GRender::createApplication() {
	return new GShader;
}
