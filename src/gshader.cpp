#include "gshader.h"

GRender::Application* GRender::createApplication(int argc, char** argv) {
	namespace fs = std::filesystem;

	fs::path currDir = fs::current_path();
	
	// Setup program to use executable path as reference
	std::filesystem::path exe = fs::path{ __argv[0] }.parent_path();
	if (fs::exists(exe))
		fs::current_path(exe);

	if (argc == 1)
		return new GShader("../examples/basic.glsl");
	else {
		if (!fs::exists(argv[1])) {
			return new GShader(currDir / fs::path{ argv[1] });
		} else {
			return new GShader(argv[1]);
		}
	}
}


GShader::GShader(const fs::path& filepath) : Application("GShader", 1200, 800, "layout.ini") {
	quad = quad::Quad(1);
	specs.size = { 2.0f, 2.0f };

	*fbuffer = Framebuffer(1200, 800);
	shader.initialize();

	if (!fs::exists(filepath)) {
		importShader("../examples/basic.glsl");
		mailbox::CreateError("File doesn't exist: " + filepath.string());
	} else {
		std::string ext = filepath.extension().string();
		if (ext == ".glsl")
			importShader(filepath);
		else if (ext == ".json")
			loadConfig(filepath);
		else {
			importShader("../examples/basic.glsl");
			mailbox::CreateError("File extension not supported: " + filepath.filename().string());
		}
	}
}



void GShader::onUserUpdate(float deltaTime) {
	bool ctrl = keyboard::isDown(GRender::Key::LEFT_CONTROL) || keyboard::isDown(GRender::Key::RIGHT_CONTROL);
	bool alt = keyboard::isDown(GRender::Key::LEFT_ALT) || keyboard::isDown(GRender::Key::RIGHT_ALT);

    ///////////////////////////////////////////////////////
    // IO
    if (ctrl && keyboard::isPressed('O')) {
		auto function = [](const fs::path& path, void* ptr) -> void {
			GShader* gs = reinterpret_cast<GShader*>(ptr);
			if (path.extension() == ".json")
				gs->loadConfig(path);
			else
				gs->importShader(path);
		};
		dialog::OpenFile("Open shader...", { "json", "glsl" }, function, this);
	}

    if (ctrl && keyboard::isPressed('L')) {
        auto function = [](const fs::path& path, void* ptr) -> void {
            reinterpret_cast<GShader*>(ptr)->loadConfig(path);
        };
        dialog::OpenFile("Load configurations...", {"json"}, function, this);
    }

    if (ctrl && keyboard::isPressed('S')) {
        auto function = [](const fs::path& path, void* ptr) -> void {
            reinterpret_cast<GShader*>(ptr)->saveConfig(path);
        };
        dialog::SaveFile("Save configurations...", {"json"}, function, this);
    }

    ///////////////////////////////////////////////////////
    // Property windows
	
    if (fbuffer.active && keyboard::isPressed('I'))
		view_specs = alt ? false : true;

	if (fbuffer.active && keyboard::isPressed('C'))
		alt ? colors.close() : colors.open();

	if (fbuffer.active && keyboard::isPressed('U'))
		alt ? uniforms.close() : uniforms.open();

	if (fbuffer.active && keyboard::isPressed('V'))
		alt ? camera.close() : camera.open();

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

	shader.setVec3f("iCamPos", glm::value_ptr(camera.getPosition()));
	shader.setFloat("iCamYaw", camera.getYaw());
	shader.setFloat("iCamPitch", camera.getPitch());
	shader.setFloat("iFOV", camera.getFOV());


	float vec[2] = { 0.0f, 0.0f };
	if (fbuffer.active) {
		glm::uvec2 fpos = fbuffer->getPosition();
		glm::uvec2 size = fbuffer->getSize();
		glm::vec2 mpos = mouse::position();
		vec[0] = (mpos.x - fpos.x) / float(size.x);
		vec[1] = 1.0f - (mpos.y - fpos.y) / float(size.y);
	}
	shader.setVec2f("iMouse", vec);

	// Submit data to shader
	colors.submit(shader);
	uniforms.submit(shader);

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
	uniforms.showUniforms();
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
				GShader* gs = reinterpret_cast<GShader*>(ptr);
				if (path.extension() == ".json")
					gs->loadConfig(path);
				else 
					gs->importShader(path);
			};
			dialog::OpenFile("Open shader...", { "json", "glsl" }, function, this);
		}

        if (ImGui::MenuItem("Save configurations...", "Ctrl+S")) {
            auto function = [](const fs::path& path, void* ptr) -> void {
                reinterpret_cast<GShader*>(ptr)->saveConfig(path);
            };
            dialog::SaveFile("Save configurations...", {"json"}, function, this);
        }

		if (ImGui::MenuItem("Exit"))
			closeApp();

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Options")) {
		if (ImGui::MenuItem("Specs...", "I"))
			view_specs = true;

		if (ImGui::MenuItem("Colors...", "C")) {
			colors.open();
		}

		if (ImGui::MenuItem("Uniforms...", "U")) {
			uniforms.open();
		}

		if (ImGui::MenuItem("Camera...", "V")) {
			camera.open();
		}

		ImGui::EndMenu();
	}
}

void GShader::importShader(const fs::path& shaderpath) {
	mailbox::Clear();
	mailbox::Close();
	elapsedTime = 0.0f;
    
	// In case we simply saved the glsl file, we don't need to reset anything
	if (shaderpath != currentShader) {
		currentShader = shaderpath;
		colors = Colors();
		camera = GRender::Camera();
	}

	shader.loadShader(shaderpath);
	setAppTitle("GShader :: " + shaderpath.filename().string());
}

void GShader::loadConfig(const fs::path& configpath) {
	//GRender::ASSERT(fs::exists(configpath), "'" + configpath.string() + "' doesn't exist!");
	
	ConfigFile config(configpath);
	config.load();
	
	fs::path shaderpath = config.get<fs::path>();
	if (shaderpath.empty())
		return;

	currentShader = shaderpath;
	colors = config.get<Colors>();
	uniforms = config.get<uniform::Uniform>();
	camera = config.get<GRender::Camera>();

	importShader(currentShader);
}

void GShader::saveConfig(const fs::path& configpath) {
	ConfigFile config(configpath);
    fs::path relPath = fs::relative(currentShader, configpath.parent_path());
	
    config.insert(relPath);
	config.insert(colors);
	config.insert(camera);
	config.insert(uniforms);
	config.save();
}
