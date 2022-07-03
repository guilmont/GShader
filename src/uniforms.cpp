#include "uniforms.h"

#include "mailbox.h"

#include "imgui.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace uniform {


void Uniform::append(const std::string& name, ParentData* data) {
	mData.emplace(name, std::move(data));
}

template<typename TP>
void Uniform::addDataWizard(const Type& tp) {
	const float width = 0.9f * ImGui::GetContentRegionAvail().x;
	const float wSpacing = 0.13f * width;

	const char* label = "##Label:";
	const char* strRange = "##Range:";
	const char* strData = "##Data:";

	// A small test to determine if we are dealing with integers or floats
	bool isInteger = tp >= Type::INT && tp <= Type::IVEC4;

	// We are going to always have 4 values to data, independently of type
	// This should simply code a bit
	static glm::vec<2, TP> range = { 0, 1 };
	static glm::vec<4, TP> data = { 0, 0, 0, 0 };
	static char newName[128] = { 0 };

	//////////////////////////////////////////////////////////
	// Label
	ImGui::Text(label + 2);
	ImGui::SameLine(wSpacing);
	ImGui::SetNextItemWidth(0.8f * width);
	bool enter = ImGui::InputText(label, newName, sizeof(newName), ImGuiInputTextFlags_EnterReturnsTrue);

	///////////////////////////////////////////////////////
	// Setup ranges
	ImGui::Text(strRange + 2);
	ImGui::SameLine(wSpacing);
	ImGui::SetNextItemWidth(0.5f * width);
	if (isInteger) {
		int32_t maxVal = INT_MAX >> 1, minVal = -maxVal;
		ImGui::DragScalarN(strRange, ImGuiDataType_S32, glm::value_ptr(range), 2, 1, &minVal, &maxVal, "%d", 0);
	}
	else {
		float maxVal = FLT_MAX / 2.0f, minVal = -maxVal;
		ImGui::DragScalarN(strRange, ImGuiDataType_Float, glm::value_ptr(range), 2, 1.0f, &minVal, &maxVal, "%.3f", 1);
	}

	///////////////////////////////////////////////////////
	// Actual date
	ImGui::Text(strData + 2);
	ImGui::SameLine(wSpacing);

	// integers
	if (isInteger) {
		int32_t sz = static_cast<int32_t>(tp);
		float val = pow(0.25f * sz, 0.5f);
		ImGui::SetNextItemWidth(val * width);
		ImGui::SliderScalarN(strData, ImGuiDataType_S32, glm::value_ptr(data), sz, &range.x, &range.y, "%d", 0);
	}
	// floats
	else {
		int32_t sz = static_cast<int32_t>(tp) - static_cast<int32_t>(Type::FLOAT) + 1;
		float val = pow(0.25f * sz, 0.5f);
		ImGui::SetNextItemWidth(val * width);
		ImGui::SliderScalarN(strData, ImGuiDataType_Float, glm::value_ptr(data), sz, &range.x, &range.y, "%.3f", 1);
	}

	///////////////////////////////////////////////////////
	// Processing data for new data or closing dialog
	auto reset = [&](void) -> void {
		memset(newName, 0, sizeof(newName));
		range = { 0, 1 };
		data = { 0, 0, 0, 0 };
		addOn = false;
	};

	if (ImGui::Button("Add") || enter) {
		std::string name = "##" + std::string{ newName };
		if (name.size() > 2) {

			// If new data was created, we emplace it in mData
			if (mData.find(name) != mData.end()) {
				GRender::mailbox::CreateWarn("'" + name.substr(2) + "' already exists!");
			}
			else {
				switch (tp) {
				case Type::INT:
				case Type::FLOAT:
					mData.emplace(name, std::make_unique<Data<1, TP>>(name, tp, range, data));
					break;
				case Type::VEC2:
				case Type::IVEC2:
					mData.emplace(name, std::make_unique<Data<2, TP>>(name, tp, range, data));
					break;
				case Type::VEC3:
				case Type::IVEC3:
					mData.emplace(name, std::make_unique<Data<3, TP>>(name, tp, range, data));
					break;
				default:
					mData.emplace(name, std::make_unique<Data<4, TP>>(name, tp, range, data));
					break;
				}

				reset();
			}
		}
	}

	ImGui::SameLine();
	if (ImGui::Button("Close")) {
		reset();
	}
}

void Uniform::addUniform() {
	ImGui::Begin("New uniform", &addOn);

	const ImVec2 size = { 500.0f, 200.0f };
	ImGui::SetWindowSize(size);

	ImGui::Text("Type:");
	ImGui::SameLine(0.13f * 0.9f * size.x);

	ImGui::SetNextItemWidth(0.3f * size.x);
	const char* items[] = { "NONE", "INT", "IVEC2", "IVEC3", "IVEC4", "FLOAT", "VEC2", "VEC3", "VEC4" };
	static int32_t currItem = 1;
	ImGui::Combo("##comboType", &currItem, items, IM_ARRAYSIZE(items));

	const Type tp = static_cast<Type>(currItem);
	if (tp >= Type::INT && tp <= Type::IVEC4) {
		addDataWizard<int32_t>(tp);
	}
	else if (tp >= Type::FLOAT && tp <= Type::VEC4) {
		addDataWizard<float>(tp);
	}

	ImGui::End();
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

template<typename TP>
void Uniform::showData(ParentData* ptr, std::string& toRemove) {
	float width = ImGui::GetContentRegionAvail().x;
	TP* dt = reinterpret_cast<TP*>(ptr);

	const std::string& name = ptr->name;

	ImGui::PushID(name.c_str());
	char local[128] = { 0 };
	std::copy(name.begin() + 2, name.end(), local);

	ImGui::SetNextItemWidth(0.45f * width);
	if (ImGui::InputText(name.c_str(), local, sizeof(local), ImGuiInputTextFlags_EnterReturnsTrue)) {
		std::string tag = "##" + std::string{ local };
		if (tag.size() > 2) {
			auto it = mData.find(tag);
			if (it == mData.end()) {
				TP* data = new TP(tag, dt->tp, dt->range, dt->data);
				mData.emplace(tag, std::move(data));
				toRemove = name; // We remove the old one
			}
			else {
				GRender::mailbox::CreateWarn("'" + tag.substr(2) + "' already exists!");
			}
		}
	}
	ImGui::SameLine();
	ImGui::SetNextItemWidth(0.45f * width);

	if (dt->tp >= Type::INT && dt->tp <= Type::IVEC4) {
		const int32_t sz = static_cast<int32_t>(dt->tp);
		ImGui::SliderScalarN(dt->name.c_str(), ImGuiDataType_S32, &dt->data.x, sz, &dt->range.x, &dt->range.y, "%d", 0);
	}
	else {
		const int32_t sz = static_cast<int32_t>(dt->tp) - static_cast<int32_t>(Type::FLOAT) + 1;
		ImGui::SliderScalarN(dt->name.c_str(), ImGuiDataType_Float, &dt->data.x, sz, &dt->range.x, &dt->range.y, "%.3f", 1);
	}

	ImGui::SameLine();
	if (ImGui::Button("X", { 0.05f * width, 0.0f })) {
		toRemove = name;
	}
	ImGui::PopID();
}

void Uniform::showUniforms() {
	if (!active) {
		return;
	}

	ImGui::Begin("Uniforms", &active);
	ImGui::SetWindowSize({ 600.0f, 350.0f });
	std::string toRemove; // If we want to rename a color

	float height = ImGui::GetContentRegionAvail().y;

	ImVec2 size = { 0.97f * ImGui::GetWindowWidth(), 0.8f * ImGui::GetWindowHeight() };
	ImGui::BeginChild("child_2", size, true);

	for (auto& [name, data] : mData) {
		switch (data->tp) {
		case Type::INT:
			showData<DataInt>(data.get(), toRemove);
			break;
		case Type::IVEC2:
			showData<DataInt2>(data.get(), toRemove);
			break;
		case Type::IVEC3:
			showData<DataInt3>(data.get(), toRemove);
			break;
		case Type::IVEC4:
			showData<DataInt4>(data.get(), toRemove);
			break;
		case Type::FLOAT:
			showData<DataFloat>(data.get(), toRemove);
			break;
		case Type::VEC2:
			showData<DataFloat2>(data.get(), toRemove);
			break;
		case Type::VEC3:
			showData<DataFloat3>(data.get(), toRemove);
			break;
		default:
			showData<DataFloat4>(data.get(), toRemove);
			break;
		}
	}

	if (!toRemove.empty()) {
		mData.erase(toRemove);
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
		addUniform();
	}
}


/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////


void Uniform::submit(const DynamicShader& shader) {
	for (const auto& [name, data] : mData) {
		switch (data->tp) {
		case Type::INT:
			shader.setInteger(name.substr(2).c_str(), reinterpret_cast<DataInt*>(data.get())->data.x);
			break;
		case Type::IVEC2:
			shader.setVec2i(name.substr(2).c_str(), glm::value_ptr(reinterpret_cast<DataInt2*>(data.get())->data));
			break;
		case Type::IVEC3:
			shader.setVec3i(name.substr(2).c_str(), glm::value_ptr(reinterpret_cast<DataInt3*>(data.get())->data));
			break;
		case Type::IVEC4:
			shader.setVec4i(name.substr(2).c_str(), glm::value_ptr(reinterpret_cast<DataInt4*>(data.get())->data));
			break;
		case Type::FLOAT:
			shader.setFloat(name.substr(2).c_str(), reinterpret_cast<DataFloat*>(data.get())->data.x);
			break;
		case Type::VEC2:
			shader.setVec2f(name.substr(2).c_str(), glm::value_ptr(reinterpret_cast<DataFloat2*>(data.get())->data));
			break;
		case Type::VEC3:
			shader.setVec3f(name.substr(2).c_str(), glm::value_ptr(reinterpret_cast<DataFloat3*>(data.get())->data));
			break;
		default:
			shader.setVec4f(name.substr(2).c_str(), glm::value_ptr(reinterpret_cast<DataFloat4*>(data.get())->data));
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

void Uniform::open() {
	active = true;
}

void Uniform::close() {
	active = false;
}

} // namespcae uniform