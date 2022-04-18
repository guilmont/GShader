#include "gshader.h"

GShader::GShader(void) : Application("GShader", 1200, 800, "layout.ini") {
	quad = quad::Quad(1);
	specs.size = { 2.0f, 2.0f };

	fbuffer = Framebuffer(1200, 800);
	loadShader();
}


void GShader::loadShader() {
	elapsedTime = 0.0f;
	modTime = fs::last_write_time(shaderPath);
	shader = Shader("../assets/vtxShader.glsl", shaderPath);
}

void GShader::onUserUpdate(float deltaTime) {

	bool ctrl = keyboard.isDown(GRender::Key::LEFT_CONTROL) || keyboard.isDown(GRender::Key::RIGHT_CONTROL);

	if (ctrl && keyboard.isPressed('S'))
		view_specs = true;

	if (ctrl && keyboard.isPressed('O')) {
		auto function = [](const fs::path& path, void* ptr) -> void { *reinterpret_cast<fs::path*>(ptr) = path; };
		dialog.openFile("Open shader...", { "glsl" }, function, &shaderPath);
	}

	// Update shader if it was modified
	elapsedTime += deltaTime;
	if (fs::last_write_time(shaderPath) != modTime)
		loadShader();

	//////////////////////////////////////////////////////////
	// Drawing to framebuffer

	glm::uvec2 res = fbuffer.getSize();
	float aRatio = float(res.x) / float(res.y);

	fbuffer.bind();

	// Setup shader
	shader.bind();
	shader.setFloat("iTime", elapsedTime);
	shader.setFloat("iRatio", aRatio);

	// Drawing quad
	quad.draw(specs);
	quad.submit();

	fbuffer.unbind();
}

void GShader::ImGuiLayer(void) {
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

//////////////////////////////////////////////////////////////////////////////
	/// Updating viewport
	ImGui::Begin("Viewport", NULL, ImGuiWindowFlags_NoTitleBar);

	// Check if it needs to resize
	ImVec2 port = ImGui::GetContentRegionAvail();
	ImGui::Image((void *)(uintptr_t)fbuffer.getID(), port, {0.0f, 1.0f}, {1.0f, 0.0f});

	glm::uvec2 view = fbuffer.getSize();
	glm::uvec2 uport{ uint32_t(port.x), uint32_t(port.y) };

	if (uport.x != view.x || uport.y != view.y) {
		fbuffer = GRender::Framebuffer(uport);
	}

	ImGui::End();
}

void GShader::ImGuiMenuLayer(void) {
	if (ImGui::BeginMenu("File")) {

		if (ImGui::MenuItem("Open shader...", "Ctrl+O")) {
			auto function = [](const fs::path& path, void* ptr) -> void { *reinterpret_cast<fs::path*>(ptr) = path; };
			dialog.openFile("Open shader...", { "glsl" }, function, &shaderPath);
		}

		if (ImGui::MenuItem("Exit"))
			closeApp();

		ImGui::EndMenu();
	}


	if (ImGui::BeginMenu("About"))
	{
		if (ImGui::MenuItem("Specs", "Ctrl+H"))
			view_specs = true;

		if (ImGui::MenuItem("View mailbox"))
			mailbox.setActive();
		
		ImGui::EndMenu();
	}
}
