#include "glTF_Loader.hpp"
#include <array>
#include <vector>
#include <filesystem>

std::string splitFilename(const std::string& str)
{
	size_t found;
	found = str.find_last_of("/\\");
	std::string result(str.substr(0, found).c_str());
	return result;
	//std::cout << " file: " << str.substr(found + 1) << "\n";
}
GLTF_Loader::GLTF_Loader(const std::string& filepath)
{
	// Open the file
	std::ifstream file(filepath);

	// Read the entire file into a string
	std::string json((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());

	// Parse the JSON data
	modelInfo.Parse(json.c_str());

	// Check for parse errors
	if (modelInfo.HasParseError()) {
		std::cerr << "Error parsing JSON: "
			<< modelInfo.GetParseError() << " \n ";
		return;
	}

	writeMeshesMap(meshes, modelInfo);
	writeAccessorsVector(accessors, modelInfo);
	writeBufferViewsVector(bufferViews, modelInfo);
	writeBufferInfosVector(bufferInfos, modelInfo, filepath);
	file.close();

	writeBigBuffers();
}

void GLTF_Loader::writeMeshesMap(
	std::unordered_map<std::string, std::unordered_map<std::string, int>>& meshes,
	const rapidjson::Document& modelInfo) {

	const auto& meshesArr = modelInfo["meshes"].GetArray();

	for (auto& mesh : meshesArr) {
		const auto& meshName = mesh.GetObject()["name"].GetString();
		const auto& primitivesArr = mesh.GetObject()["primitives"].GetArray();

		//just setting init capacity to some mostly guaranteed value, in this case 4
		std::unordered_map<std::string, int> primitives;
		primitives.reserve(4);

		for (auto& primitive : primitivesArr) {
			const auto& primitiveObj = primitive.GetObject();

			const auto& attributes = primitiveObj["attributes"].GetObject();
			for (Value::ConstMemberIterator itr = attributes.MemberBegin();
				itr != attributes.MemberEnd(); ++itr)
			{
				primitives.emplace(itr->name.GetString(), itr->value.GetInt());
				//std::cout << itr->name.GetString() << ": " << itr->value.GetUint() << "\n";
			}

			if (primitiveObj.HasMember("indices")) {
				//const auto& indices = primitiveObj["indices"].GetUint();
				//std::cout << "indices: " << indices << "\n";
				primitives.emplace("indices", primitiveObj["indices"].GetInt());
			}
		}
		meshes.emplace(meshName, primitives);
	}
}
void GLTF_Loader::writeAccessorsVector(std::vector<Accessor>& accessors, const rapidjson::Document& modelInfo)
{
	const auto& accessorsArr = modelInfo["accessors"].GetArray();
	for (auto& accessorInfo : accessorsArr) {
		const auto& accessorInfoObj = accessorInfo.GetObject();
		Accessor accessor{
			accessorInfoObj["bufferView"].GetInt(),
			accessorInfoObj["componentType"].GetInt(),
			accessorInfoObj["count"].GetInt(),
			this->convertStringToIntType(accessorInfoObj["type"].GetString()) };
		accessors.push_back(accessor);
	}
}
void GLTF_Loader::writeBufferViewsVector(std::vector<BufferView>& bufferViews, const rapidjson::Document& modelInfo)
{
	const auto& buffeViewsArr = modelInfo["bufferViews"].GetArray();
	for (auto& bufferView : buffeViewsArr) {
		const auto& bufferViewObj = bufferView.GetObject();
		BufferView bufferView{
			bufferViewObj["buffer"].GetInt(),
			bufferViewObj["byteLength"].GetInt(),
			bufferViewObj["byteOffset"].GetInt(),
			0 };
		if (bufferViewObj.HasMember("target")) {
			bufferView.target = bufferViewObj["target"].GetInt();
		}
		bufferViews.push_back(bufferView);
	}
}
void GLTF_Loader::writeBufferInfosVector(
	std::vector<BufferInfo>& bufferInfos,
	const rapidjson::Document& modelInfo, const std::string& filepath)
{
	const auto& buffersArr = modelInfo["buffers"].GetArray();
	std::string pathToBin(splitFilename(filepath) + "\\");
	for (auto& bufferInfo : buffersArr) {
		const auto& bufferInfoObj = bufferInfo.GetObject();
		BufferInfo bufferInfo{
			bufferInfoObj["byteLength"].GetInt(),
			pathToBin + bufferInfoObj["uri"].GetString() };
		bufferInfos.push_back(bufferInfo);
	}
}

void GLTF_Loader::writeBigBuffers() {
	const int bufferInfosSize = bufferInfos.size();
	bigBuffers.reserve(bufferInfosSize);
	for (int bufferIndex = 0; bufferIndex < bufferInfosSize; bufferIndex++) {
		const auto& bufferInfo = bufferInfos[bufferIndex];

		std::ifstream binaryFile(bufferInfo.uri, std::ios::binary);
		if (!binaryFile.is_open()) {
			throw std::runtime_error("failed to open file!" + std::string(bufferInfo.uri));
		}
		std::vector<char> bigBuffer(bufferInfo.byteLength);
		binaryFile.read(bigBuffer.data(), bufferInfo.byteLength);
		bigBuffers.push_back(bigBuffer);
		binaryFile.close();
	}
}
void GLTF_Loader::writeBuffers()
{
	const int bufferInfosSize = bufferInfos.size();
	const int bufferViewsSize = bufferViews.size();
	const int accessorsSize = accessors.size();

	for (int bufferIndex = 0; bufferIndex < bufferInfosSize; bufferIndex++) {
		const auto& bufferInfo = bufferInfos[bufferIndex];

		std::ifstream binaryFile(bufferInfo.uri, std::ios::binary);
		if (!binaryFile.is_open()) {
			throw std::runtime_error("failed to open file!" + std::string(bufferInfo.uri));
		}
		std::vector<char> bigBuffer(bufferInfo.byteLength);
		binaryFile.read(bigBuffer.data(), bufferInfo.byteLength);
		binaryFile.close();
		for (int accessorIndex = 0; accessorIndex < accessorsSize; accessorIndex++) {
			const auto& accessor = accessors[accessorIndex];
			const auto& bufferView = bufferViews[accessor.bufferView];
			int bufferSize = bufferView.byteLength;
			int byteOffset = bufferView.byteOffset;

			std::vector<char> buffer(bufferSize);
			std::memcpy(buffer.data(), bigBuffer.data() + byteOffset, bufferSize);
			buffers.push_back(buffer);
		}
	}
}

std::variant<
	std::vector<unsigned short>,
	std::vector<signed char>,
	std::vector<unsigned char>,
	std::vector<short>,
	std::vector<unsigned int>,
	std::vector<float>> GLTF_Loader::getDataAuto(
		const std::string& objectName,
		const std::string& attributeName)
{
	const int accessorIndex = this->meshes[objectName][attributeName];
	const auto& accessor = accessors[accessorIndex];
	const auto& bufferView = bufferViews[accessor.bufferView];
	const int bufferSize = bufferView.byteLength;
	const int byteOffset = bufferView.byteOffset;
	switch (accessor.componentType) {
	case UNSIGNED_SHORT: {
		std::cout << "scalars:" << "\n";
		std::vector<unsigned short> scalars(bufferSize / sizeof(unsigned short));
		std::memcpy(scalars.data(), (bigBuffers[bufferView.bufferId]).data() + byteOffset, bufferSize);
		return scalars;
	}
	case SIGNED_BYTE: {
		std::cout << "chars:" << "\n";
		std::vector<signed char> chars(bufferSize);
		std::memcpy(chars.data(), (bigBuffers[bufferView.bufferId]).data() + byteOffset, bufferSize);
		return chars;
	}
	case UNSIGNED_BYTE: {
		std::cout << "unsigned chars:" << "\n";
		std::vector<unsigned char> chars(bufferSize / sizeof(unsigned char));
		std::memcpy(chars.data(), (bigBuffers[bufferView.bufferId]).data() + byteOffset, bufferSize);
		return chars;
	}
	case SIGNED_SHORT: {
		std::cout << "signed shorts:" << "\n";
		std::vector<short> shorts(bufferSize / sizeof(short));
		std::memcpy(shorts.data(), (bigBuffers[bufferView.bufferId]).data() + byteOffset, bufferSize);
		return shorts;
	}
	case UNSIGNED_INT: {
		std::cout << "unsigned ints" << "\n";
		std::vector<unsigned int> uints(bufferSize / sizeof(unsigned int));
		std::memcpy(uints.data(), (bigBuffers[bufferView.bufferId]).data() + byteOffset, bufferSize);
		return uints;
	}
	case FLOAT: {
		std::cout << "floats" << "\n";
		std::vector<float> floats(bufferSize / sizeof(float));
		std::memcpy(floats.data(), (bigBuffers[bufferView.bufferId]).data() + byteOffset, bufferSize);
		return floats;
	}
	}
}


void GLTF_Loader::convertBuffers() {

	const int bufferInfosSize = bufferInfos.size();
	const int bufferViewsSize = bufferViews.size();
	const int accessorsSize = accessors.size();

	for (int accessorIndex = 0; accessorIndex < accessorsSize; accessorIndex++) {
		const auto& accessor = accessors[accessorIndex];
		const auto& bufferView = bufferViews[accessor.bufferView];
		int bufferSize = bufferView.byteLength;
		int byteOffset = bufferView.byteOffset;

		switch (accessor.componentType) {
		case UNSIGNED_SHORT: {
			std::cout << "scalars:" << "\n";
			std::vector<unsigned short> scalars(bufferSize / sizeof(unsigned short));
			std::memcpy(scalars.data(), buffers[accessorIndex].data(), bufferSize);
			int i = 0;
			for (auto& obj : scalars) {
				i++;
				std::cout << obj << ", ";
			}
			std::cout << int(i) << "\n";
			break;
		}
		case SIGNED_BYTE: {
			std::cout << "chars:" << "\n";
			std::vector<signed char> chars(bufferSize);
			std::memcpy(chars.data(), buffers[accessorIndex].data(), bufferSize);
			int i = 0;
			for (auto& obj : chars) {
				i++;
				std::cout << obj << ", ";
			}
			std::cout << int(i) << "\n";
			break;
		}
		case UNSIGNED_BYTE: {
			std::cout << "unsigned chars:" << "\n";
			std::vector<unsigned char> chars(bufferSize / sizeof(unsigned char));
			std::memcpy(chars.data(), buffers[accessorIndex].data(), bufferSize);
			int i = 0;
			for (auto& obj : chars) {
				i++;
				std::cout << static_cast<unsigned>(obj) << ", ";
			}
			std::cout << int(i) << "\n";
			break;
		}
		case SIGNED_SHORT: {
			std::cout << "signed shorts:" << "\n";
			std::vector<short> shorts(bufferSize / sizeof(short));
			std::memcpy(shorts.data(), buffers[accessorIndex].data(), bufferSize);
			int i = 0;
			for (auto& obj : shorts) {
				i++;
				std::cout << obj << ", ";
			}
			std::cout << int(i) << "\n";
			break;
		}
		case UNSIGNED_INT: {
			std::cout << "unsigned ints" << "\n";
			std::vector<unsigned int> uints(bufferSize / sizeof(unsigned int));
			std::memcpy(uints.data(), buffers[accessorIndex].data(), bufferSize);
			int i = 0;
			for (auto& obj : uints) {
				i++;
				std::cout << obj << ", ";
			}
			std::cout << int(i) << "\n";
			break;
		}
		case FLOAT: {
			std::cout << "floats" << "\n";
			std::vector<float> floats(bufferSize / sizeof(float));
			std::memcpy(floats.data(), buffers[accessorIndex].data(), bufferSize);
			int i = 0;
			for (auto& obj : floats) {
				i++;
				std::cout << obj << ", ";
			}
			std::cout << int(i) << "\n";
			break;
		}
		}
	}
}