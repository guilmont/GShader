#include "gshader.h"

GShader::GShader(void) : Application("GShader", 1200, 800, "layout.ini") {
	quad = quad::Quad(1);
	specs.size = { 2.0f, 2.0f };

	*fbuffer = Framebuffer(1200, 800);
	shader.initialize();

	// starts default shader
	importShader("../examples/basic.glsl"); 
}


void GShader::importShader(const fs::path& shaderpath) {
	mailbox::Clear();
	mailbox::Close();
	elapsedTime = 0.0f;
    currentShader = shaderpath;
	shader.loadShader(shaderpath);
}

void GShader::onUserUpdate(float deltaTime) {

	bool ctrl = keyboard::isDown(GRender::Key::LEFT_CONTROL) || keyboard::isDown(GRender::Key::RIGHT_CONTROL);
	bool alt = keyboard::isDown(GRender::Key::LEFT_ALT) || keyboard::isDown(GRender::Key::RIGHT_ALT);

	if (ctrl && keyboard::isPressed('O')) {
		auto function = [](const fs::path& path, void* ptr) -> void { 
			reinterpret_cast<GShader*>(ptr)->importShader(path);			
		};
		dialog::OpenFile("Open shader...", { "glsl" }, function, this);
	}

	if (fbuffer.active && keyboard::isPressed('S'))
		view_specs = alt ? false : true;

	if (fbuffer.active && keyboard::isPressed('C')) {
		alt ? colors.close() : colors.open();
	}

	if (fbuffer.active && keyboard::isPressed('V')) {
		alt ? camera.close() : camera.open();
	}

	// Automatic controls for camera
	if (fbuffer.active)
		camera.controls(deltaTime);

	// Update shader if it was modified
	elapsedTime += deltaTime;
	if (shader.wasUpdated())
		importShader(currentShader);

	//////////////////////////////////////////////////////////
	// Drawing to framebuffer

	if (shader.hasFailed())
		return;

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
			auto function = [](const fs::path& path, void* ptr) -> void { 
				reinterpret_cast<GShader*>(ptr)->importShader(path);			
			};
			dialog::OpenFile("Open shader...", { "glsl" }, function, this);
		}

		if (ImGui::MenuItem("Exit"))
			closeApp();

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Options")) {
		if (ImGui::MenuItem("Specs...", "S"))
			view_specs = true;

		if (ImGui::MenuItem("Colors...", "C")) {
			colors.open();
		}

		if (ImGui::MenuItem("Camera...", "V")) {
			camera.open();
		}

		ImGui::EndMenu();
	}
}
