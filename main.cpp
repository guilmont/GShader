#include "GRender.h"


class Tracer : public GRender::Application
{
public:
	Tracer(void);
	~Tracer(void) = default;

	void onUserUpdate(float deltaTime) override;
	void ImGuiLayer(void) override;
	void ImGuiMenuLayer(void) override;

private:
	bool
		view_specs = false,
		view_imguidemo = false,
		viewport_hover = false;

	GRender::TextureRGBA texture;
	GRender::Shader shader;

};

GRender::Application* GRender::createApplication() {
	return new Tracer;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Tracer::Tracer(void) : Application("Tracer", 1200, 800, "layout.ini") {
	texture = GRender::TextureRGBA("../assets/ultron.jpg");

}

void Tracer::onUserUpdate(float deltaTime) {
	bool ctrl = keyboard.isDown(GRender::Key::LEFT_CONTROL) || keyboard.isDown(GRender::Key::RIGHT_CONTROL);

	if (ctrl && keyboard.isPressed('H'))
		view_specs = true;

	if (ctrl && keyboard.isPressed('I'))
		view_imguidemo = true;

	
}

void Tracer::ImGuiLayer(void) {
	if (view_specs) {
		ImGui::Begin("Specs", &view_specs);
		ImGui::Text("FT: %.3f ms", 1000.0 * double(ImGui::GetIO().DeltaTime));
		ImGui::Text("FPS: %.0f", ImGui::GetIO().Framerate);
		ImGui::Text("Vendor: %s", glad_glGetString(GL_VENDOR));
		ImGui::Text("Graphics card: %s", glad_glGetString(GL_RENDERER));
		ImGui::Text("OpenGL version: %s", glad_glGetString(GL_VERSION));
		ImGui::Text("GLSL version: %s", glad_glGetString(GL_SHADING_LANGUAGE_VERSION));
		ImGui::End();
	}

	if (view_imguidemo)
		ImGui::ShowDemoWindow(&view_imguidemo);


	//////////////////////////////////////////////////////////////////////////////
	/// Updating viewport
	ImGui::Begin("Viewport", NULL, ImGuiWindowFlags_NoTitleBar);
	viewport_hover = ImGui::IsWindowHovered();

	// Check if it needs to resize
	glm::uvec2 size = texture.getSize();
	float aRatio = float(size.x) / float(size.y);
	ImVec2 port = ImGui::GetContentRegionAvail();
	port.x = port.y * aRatio;
	ImGui::Image((void *)(uintptr_t)texture.getID(), port, {0.0f, 1.0f}, {1.0f, 0.0f});
	
	ImGui::End();
}

void Tracer::ImGuiMenuLayer(void) {
	if (ImGui::BeginMenu("File")) {

		if (ImGui::MenuItem("Exit"))
			closeApp();

		ImGui::EndMenu();
	}


	if (ImGui::BeginMenu("About"))
	{
		if (ImGui::MenuItem("Specs", "Ctrl+H"))
			view_specs = true;

		if (ImGui::MenuItem("ImGui Demo", "Ctrl+D"))
			view_imguidemo = true;

		if (ImGui::MenuItem("View mailbox"))
			mailbox.setActive();
		
		ImGui::EndMenu();
	}
}
