#include "gshader.h"

#include "GRender/entryPoint.h"
namespace mouse = mouse;
namespace keyboard = keyboard;

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
	bool ctrl = keyboard::IsDown(Key::LEFT_CONTROL) || keyboard::IsDown(Key::RIGHT_CONTROL);
	bool alt = keyboard::IsDown(Key::LEFT_ALT) || keyboard::IsDown(Key::RIGHT_ALT);
	bool shift = keyboard::IsDown(Key::LEFT_SHIFT) || keyboard::IsDown(Key::RIGHT_SHIFT);

    ///////////////////////////////////////////////////////
    // IO
    if (ctrl && keyboard::IsPressed('O')) {
		auto function = [](const fs::path& path, void* ptr) -> void {
			GShader* gs = reinterpret_cast<GShader*>(ptr);
			if (path.extension() == ".json")
				gs->loadConfig(path);
			else
				gs->importShader(path);
		};
		dialog::OpenFile("Open shader...", { "json", "glsl" }, function, this);
	}

    if (ctrl && keyboard::IsPressed('L')) {
        auto function = [](const fs::path& path, void* ptr) -> void {
            reinterpret_cast<GShader*>(ptr)->loadConfig(path);
        };
        dialog::OpenFile("Load configurations...", {"json"}, function, this);
    }

    if (ctrl && keyboard::IsPressed('S')) {
        auto function = [](const fs::path& path, void* ptr) -> void {
            reinterpret_cast<GShader*>(ptr)->saveConfig(path);
        };
        dialog::SaveFile("Save configurations...", {"json"}, function, this);
    }

	///////////////////////////////////////////////////////
	// Control
	if (!shift && keyboard::IsPressed(Key::SPACE)) { ctrlPlay = !ctrlPlay; }

	if (shift && keyboard::IsPressed(Key::SPACE)) {
		ctrlStep = true;
		ctrlPlay = false;
	}

	if (ctrlReset || (shift && keyboard::IsPressed('R'))) {
		ctrlReset = false;

		elapsedTime = 0.0f;
		ctrlPlay = false;
		ctrlStep = true;
	}

    ///////////////////////////////////////////////////////
    // Property windows
	
    if (fbuffer.active && keyboard::IsPressed('I'))
		view_specs = alt ? false : true;

	if (fbuffer.active && keyboard::IsPressed('C'))
		alt ? colors.close() : colors.open();

	if (fbuffer.active && keyboard::IsPressed('U'))
		alt ? uniforms.close() : uniforms.open();

	if (fbuffer.active && keyboard::IsPressed('V'))
		alt ? camera.close() : camera.open();

	// Automatic controls for camera
	if (fbuffer.active && ctrlPlay)
		camera.controls(deltaTime);

	// Update shader if it was modified
	elapsedTime += deltaTime;
	if (shader.wasUpdated())
		importShader(currentShader);

	//////////////////////////////////////////////////////////
	// Drawing to framebuffer

	if (shader.hasFailed() || (!ctrlPlay && !ctrlStep))
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
		glm::vec2 mpos = mouse::Position();
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

	// Resetting step controller, so no more updates are made
	ctrlStep = false;
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

	{
		// Hidden feature to help development
		static bool view_demo = false;
		if (keyboard::IsDown(Key::SPACE) && keyboard::IsPressed('D'))
			view_demo = true;

		if (view_demo)
			ImGui::ShowDemoWindow(&view_demo);
	}

	//////////////////////////////////////////////////////////////////////////////
	// Updating viewport
	ImGui::Begin("Viewport", NULL, ImGuiWindowFlags_NoTitleBar);
	fbuffer.active = ImGui::IsWindowHovered();

	// Check if it needs to resize
	ImVec2 port = ImGui::GetContentRegionAvail();
	ImGui::Image((void *)(uintptr_t)fbuffer->getID(), port, {0.0f, 1.0f}, {1.0f, 0.0f});

	glm::uvec2 view = fbuffer->getSize();
	glm::uvec2 uport = { port.x, port.y };

	if (uport.x != view.x || uport.y != view.y) {
		*fbuffer = Framebuffer(uport);
		ImVec2 ps = ImGui::GetWindowPos();
		fbuffer->setPosition(ps.x, ps.y);
	}

	ImGui::End();

	// Creating a small play/reset widget visible only when buffer is hovered
	// We cannot simply user fbuffer.active, otherwise the widget would disappear when mouse hovers this controls
	glm::uvec2 p0 = fbuffer->getPosition();
	glm::uvec2 pf = fbuffer->getSize() + p0;
	p0.y = (p0.y + pf.y) / 2; // To avoid the menu bars to interfere

	glm::uvec2 mpos = { mouse::Position().x, mouse::Position().y };
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.8f);
	if (mpos.x > p0.x && mpos.x < pf.x && mpos.y > p0.y && mpos.y < pf.y) {
		ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_NoDecoration);
		
		ImVec2 size = { 160.0f, 40.0f };
		ImVec2 loc = { 0.5f * (p0.x + pf.x - size.x), pf.y - 1.5f * size.y };

		ImGui::SetWindowSize(size);
		ImGui::SetWindowPos(loc);

		if (ImGui::Button(ctrlPlay ? "Pause" : "Play", {50.0f,0.0f})) { ctrlPlay = !ctrlPlay; }
		ImGui::SameLine();
		if (ImGui::Button("Step")) { 
			ctrlStep = true;
			ctrlPlay = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset")) { ctrlReset = true; }

		ImGui::End();
	}
	ImGui::PopStyleVar();
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
	ctrlPlay = true;
	ctrlStep = false;
    
	// In case we simply saved the glsl file, we don't need to reset anything
	if (shaderpath != currentShader) {
		currentShader = shaderpath;
		colors = Colors();
		camera = Camera();
	}

	shader.loadShader(shaderpath);
	setAppTitle("GShader :: " + shaderpath.filename().string());
}

void GShader::loadConfig(const fs::path& configpath) {
	//ASSERT(fs::exists(configpath), "'" + configpath.string() + "' doesn't exist!");
	
	ConfigFile config(configpath);
	config.load();
	
	fs::path shaderpath = config.get<fs::path>();
	if (shaderpath.empty())
		return;

	currentShader = shaderpath;
	colors = config.get<Colors>();
	uniforms = config.get<uniform::Uniform>();
	camera = config.get<Camera>();

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
