#include "GRender.h"
#include "dynamicShader.h"
#include "colors.h"

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

class GShader : public Application
{
public:
	GShader(void);
	~GShader(void) = default;

	void onUserUpdate(float deltaTime) override;
	void ImGuiLayer(void) override;
	void ImGuiMenuLayer(void) override;

	void importShader(const fs::path& shaderpath);

private:
	fs::path currentShader;
	float elapsedTime = 0.0f;

	bool view_specs = false;

	quad::Specs specs;
	quad::Quad quad;

	Colors colors;
	Camera camera;
	DynamicShader shader;

	Ref<Framebuffer> fbuffer;

};

GRender::Application* GRender::createApplication() {
	return new GShader;
}
