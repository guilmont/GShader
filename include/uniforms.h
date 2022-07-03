#pragma once

#include "dynamicShader.h"

#include <map>
#include <memory>
#include <string>

namespace uniform {

enum class Type : int32_t {
	NONE,
	// integer types
	INT, IVEC2, IVEC3, IVEC4,
	// float types
	FLOAT, VEC2, VEC3, VEC4,
	// total numner of types
	TOTAL
};

///////////////////////////////////////////////////////////////////////////////

struct ParentData {
	ParentData(const std::string& _name, Type _tp)
		: name(_name), tp(_tp) {}

	std::string name;
	Type tp;
};

template<size_t N, typename TP>
struct Data : public ParentData {
	Data(const std::string& name, Type _tp, glm::vec<2, TP> _range, glm::vec<N, TP> _data)
		: ParentData(name, _tp), range(_range), data(_data) {}

	glm::vec<2, TP> range;
	glm::vec<N, TP> data;
};

// Some helpers for commonly used specializations
using DataInt = Data<1, int32_t>;
using DataInt2 = Data<2, int32_t>;
using DataInt3 = Data<3, int32_t>;
using DataInt4 = Data<4, int32_t>;

using DataFloat = Data<1, float>;
using DataFloat2 = Data<2, float>;
using DataFloat3 = Data<3, float>;
using DataFloat4 = Data<4, float>;

///////////////////////////////////////////////////////////////////////////////

class Uniform {
public:
	void append(const std::string& name, ParentData* ptr);

	void addUniform(void);
	void showUniforms(void);
	void submit(const DynamicShader& shader);

	void open();
	void close();

public:
	auto begin() const {
		return mData.begin();
	}
	auto end() const {
		return mData.end();
	}

private:
	template<typename TP>
	void addDataWizard(const Type&);

	template <typename TP>
	void showData(ParentData* ptr, std::string& toRemove);

private:
	bool addOn = false;
	bool active = false;
	std::map<std::string, std::unique_ptr<ParentData>> mData;
};

} // namespace uniform
