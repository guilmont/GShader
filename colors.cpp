#include "colors.h"

#include "imgui.h"

void Colors::addColor() {
	ImGui::Begin("New color", &addOn);
	ImGui::SetWindowSize({ 400.0f, 120.0f });
	const char* label = "##Label:";
	const char* color = "##Color:";

	ImGui::Text(label + 2);
	ImGui::SameLine();
	bool enter = ImGui::InputText(label, newColorName, sizeof(newColorName), ImGuiInputTextFlags_EnterReturnsTrue);

	ImGui::Text(color + 2);
	ImGui::SameLine();
	ImGui::ColorEdit3(color, &newColor[0], ImGuiColorEditFlags_Float);


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
	std::string toRemove; // If we want to rename a color

	for (auto& [name, color] : mColors) {
		char local[128] = { 0 };
		std::copy(name.begin()+2, name.end(), local);

		ImGui::SetNextItemWidth(float(sizeof(local)) + 20.0f);
		if (ImGui::InputText(name.c_str(), local, sizeof(local), ImGuiInputTextFlags_EnterReturnsTrue)) {
			std::string tag(local);
			if (!tag.empty()) {
				mColors["##" + std::string{ local }] = color;
				toRemove = name; // We remove the old color
			}
		}
		ImGui::SameLine();
		ImGui::ColorEdit3(name.c_str(), &color[0], ImGuiColorEditFlags_Float);
		ImGui::SameLine();
		if (ImGui::Button("X", {30.0f, 0.0f})) {
			toRemove = name;
		}
	}

	if (!toRemove.empty()) {
		mColors.erase(toRemove);
	}

	if (ImGui::Button("New")) {
		addOn = true;
	}

	ImGui::End();

	// In case we need to add new colors
	if (addOn) {
		addColor();
	}

	auto it = mColors.begin();
}

void Colors::open() {
	active = true;
}

void Colors::close() {
	active = false;
}
