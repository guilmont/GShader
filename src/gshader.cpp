#include "gshader.h"

GShader::GShader(void) : Application("GShader", 1200, 800, "layout.ini") {
	quad = quad::Quad(1);
	specs.size = { 2.0f, 2.0f };

	*fbuffer = Framebuffer(1200, 800);
	shader.initialize();

	importShader(); // starts default shader: basic.glsl
}


void GShader::importShader() {
	mailbox::Clear();
	elapsedTime = 0.0f;
	modTime = fs::last_write_time(shaderPath);
	shader.loadShader(shaderPath);
}

void GShader::onUserUpdate(float deltaTime) {

	bool ctrl = keyboard::isDown(GRender::Key::LEFT_CONTROL) || keyboard::isDown(GRender::Key::RIGHT_CONTROL);
	bool shft = keyboard::isDown(GRender::Key::LEFT_SHIFT) || keyboard::isDown(GRender::Key::RIGHT_SHIFT);

	if (ctrl && keyboard::isPressed('O')) {
		auto function = [](const fs::path& path, void* ptr) -> void { *reinterpret_cast<fs::path*>(ptr) = path; };
		dialog::OpenFile("Open shader...", { "glsl" }, function, &shaderPath);
	}

	if (shft && keyboard::isPressed('S'))
		view_specs = true;

	if (shft && keyboard::isPressed('C')) {
		colors.open();
	}

	if (shft && keyboard::isPressed('V')) {
		camera.open();
	}

	// Automatic controls for camera
	if (fbuffer.active)
		camera.controls(deltaTime);

	// Update shader if it was modified
	elapsedTime += deltaTime;
	if (fs::last_write_time(shaderPath) != modTime)
		importShader();

	//////////////////////////////////////////////////////////
	// Drawing to framebuffer

	if (shader.hasFailed())
		return;

	// Let's clean up messages
	mailbox::Clear();
	mailbox::Close();

	glm::uvec2 res = fbuffer->getSize();
	float aRatio = float(res.x) / float(res.y);

	fbuffer->bind();

	// Setup shader
	shader.bind();
	shader.setFloat("iTime", elapsedTime);
	shader.setFloat("iRatio", aRatio);

	shader.setVec3f("iCamPos", &camera.position()[0]);
	shader.setFloat("iCamYaw", camera.getYaw());
	shader.setFloat("iCamPitch", camera.getPitch());
	shader.setFloat("iFOV", camera.fieldView());


	float vec[2] = { 0.0f, 0.0f };
	if (fbuffer.active) {
		glm::uvec2 fpos = fbuffer->getPosition();
		glm::uvec2 size = fbuffer->getSize();
		glm::vec2 mpos = mouse::position();
		vec[0] = (mpos.x - fpos.x) / float(size.x);
		vec[1] = 1.0f - (mpos.y - fpos.y) / float(size.y);
	}
	shader.setVec2f("iMouse", vec);

	for (auto& [name, cor] : colors) {
		shader.setVec3f(name.c_str() + 2, &cor[0]);
	}

	// Drawing quad
	quad.draw(specs);
	quad.submit();

	fbuffer->unbind();
}

void GShader::ImGuiLayer(void) {
	if (view_specs) {
		ImGui::Begin("Specs", &view_specs);
		ImGui::Text("FT: %.3f ms", 1000.0f * ImGui::GetIO().DeltaTime);
		ImGui::Text("FPS: %.0f", ImGui::GetIO().Framerate);
		ImGui::Text("Vendor: %s", glGetString(GL_VENDOR));
		ImGui::Text("Graphics card: %s", glGetString(GL_RENDERER));
		ImGui::Text("OpenGL version: %s", glGetString(GL_VERSION));
		ImGui::Text("GLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
		ImGui::End();
	}

	// External utility to edit colors on the fly
	colors.showColors();

	camera.display();

	//////////////////////////////////////////////////////////////////////////////
	// Updating viewport
	ImGui::Begin("Viewport", NULL, ImGuiWindowFlags_NoTitleBar);
	fbuffer.active = ImGui::IsWindowHovered();

	// Check if it needs to resize
	ImVec2 port = ImGui::GetContentRegionAvail();
	ImGui::Image((void *)(uintptr_t)fbuffer->getID(), port, {0.0f, 1.0f}, {1.0f, 0.0f});

	glm::uvec2 view = fbuffer->getSize();
	glm::uvec2 uport{ uint32_t(port.x), uint32_t(port.y) };

	if (uport.x != view.x || uport.y != view.y) {
		*fbuffer = GRender::Framebuffer(uport);
	}

	ImGui::End();
}

void GShader::ImGuiMenuLayer(void) {
	if (ImGui::BeginMenu("File")) {

		if (ImGui::MenuItem("Open shader...", "Ctrl+O")) {
			auto function = [](const fs::path& path, void* ptr) -> void { *reinterpret_cast<fs::path*>(ptr) = path; };
			dialog::OpenFile("Open shader...", { "glsl" }, function, &shaderPath);
		}

		if (ImGui::MenuItem("Exit"))
			closeApp();

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Options")) {
		if (ImGui::MenuItem("Specs...", "Shift+S"))
			view_specs = true;

		if (ImGui::MenuItem("Colors...", "Shift+C")) {
			colors.open();
		}

		if (ImGui::MenuItem("Camera...", "Shift+V")) {
			camera.open();
		}

		ImGui::EndMenu();
	}
}