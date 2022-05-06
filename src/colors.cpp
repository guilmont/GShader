#include "colors.h"

#include "imgui.h"

void Colors::addColor() {
	ImGui::Begin("New color", &addOn);
	ImGui::SetWindowSize({ 500.0f, 140.0f });

	const char* label = "##Label:";
	const char* color = "##Color:";

	ImGui::Text(label + 2);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(0.95f * ImGui::GetContentRegionAvail().x);
	bool enter = ImGui::InputText(label, newColorName, sizeof(newColorName), ImGuiInputTextFlags_EnterReturnsTrue);

	ImGui::Text(color + 2);
	ImGui::SameLine();
	ImGui::ColorEdit3(color, &newColor[0], ImGuiColorEditFlags_Float);

	ImGui::Dummy({ 0.0f, 10.0f });

	auto reset = [&](void) -> void {
		addOn = false;
		memset(newColorName, 0, sizeof(newColorName));
		newColor = { 1.0f, 1.0f, 1.0f };
	};

	if (ImGui::Button("Add") || enter) {
		std::string name(newColorName);
		if (!name.empty()) {
			mColors["##" + std::string{ newColorName }] = newColor;
			reset();
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Close")) {
		reset();
	}

	ImGui::End();
}

void Colors::showColors() {
	if (!active) {
		return;
	}

	ImGui::Begin("Colors", &active);
	ImGui::SetWindowSize({ 600.0f, 350.0f });

	std::string toRemove; // If we want to rename a color

	float height = ImGui::GetContentRegionAvail().y;

	ImVec2 size = { 0.97f * ImGui::GetWindowWidth(), 0.8f * ImGui::GetWindowHeight() };
	ImGui::BeginChild("child_2", size, true);

	for (auto& [name, color] : mColors) {
		char local[128] = { 0 };
		std::copy(name.begin()+2, name.end(), local);

		ImGui::SetNextItemWidth(0.45f * size.x);
		if (ImGui::InputText(name.c_str(), local, sizeof(local), ImGuiInputTextFlags_EnterReturnsTrue)) {
			std::string tag(local);
			if (!tag.empty()) {
				mColors["##" + std::string{ local }] = color;
				toRemove = name; // We remove the old color
			}
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(0.45f * size.x);
		ImGui::ColorEdit3(name.c_str(), &color[0], ImGuiColorEditFlags_Float);
		ImGui::SameLine();
		if (ImGui::Button("X", {0.05f * size.x, 0.0f})) {
			toRemove = name;
		}
	}

	if (!toRemove.empty()) {
		mColors.erase(toRemove);
	}

	ImGui::EndChild();

	if (ImGui::Button("New")) {
		addOn = true;
	}

	ImGui::SameLine();
	if (ImGui::Button("Close")) {
		active = false;
	}

	ImGui::End();

	// In case we need to add new colors
	if (addOn) {
		addColor();
	}
}

void Colors::open() {
	active = true;
}

void Colors::close() {
	active = false;
}
