#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

class Colors {
public:
	Colors() = default;
	~Colors() = default;

	void addColor();
	void showColors();

	void open();
	void close();

public:
	auto begin() const {
		return mColors.begin();
	}
	auto end() const {
		return mColors.end();
	}

private:
	bool active = false;
	std::unordered_map<std::string, glm::vec3> mColors;

private:
	// specs for adding new color
	bool addOn = false;
	char newColorName[128] = { 0 };
	glm::vec3 newColor = { 1.0f, 1.0f, 1.0f };
};