#include "glTF_Loader.hpp"
#include "glTF_Loader.hpp"
#include "glTF_Loader.hpp"
#include "glTF_Loader.hpp"
#include <array>
#include <vector>
#include <filesystem>

std::string splitFilename(const std::string& filePath)
{
	//returns path to the folder where file is located
	size_t found;
	found = filePath.find_last_of("/\\");
	std::string filePathFolder(filePath.substr(0, found).c_str());
	return filePathFolder;
}
GLTF_Loader::GLTF_Loader(const std::string& filepath)
{	
	std::cout << filepath;
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

	readNodes(nodes);
	readMeshes(meshes);


	if (modelInfo.HasMember("animations")) {
		readAnimations();
	}

	if (modelInfo.HasMember("skins")) {
			readSkins(skins);
	}

	readAccessors(accessors);
	readBufferViews(bufferViews);
	readBufferInfos(bufferInfos, filepath);
	file.close();

	writeBigBuffers();
}

void GLTF_Loader::readNodes(std::vector<NodeAttributes>& nodes) {

	const auto& nodesArr = modelInfo["nodes"].GetArray();
	for (const auto& nodeIter : nodesArr) {
		const auto& nodeObject = nodeIter.GetObject();


		NodeAttributes nodeAttributes{};
		nodeAttributes.name = nodeObject["name"].GetString();

		if (nodeObject.HasMember("children")) {
			const auto& childrenAttr = nodeObject["children"].GetArray();
			int i = 0;
			for (const auto& childrenId : childrenAttr) {
				{
					nodeAttributes.children.push_back(childrenId.GetInt());
					i++;
				}
			}
		}

		if (nodeObject.HasMember("rotation")) {
			const auto& rotationAttr = nodeObject["rotation"].GetArray();
			int i = 0;
			for (auto& rotationValue : rotationAttr) {
				{
					nodeAttributes.rotation[i] = rotationValue.GetFloat();
					i++;
				}
			}
		}

		if (nodeObject.HasMember("translation")) {
			const auto& translationAttr = nodeObject["translation"].GetArray();
			int i = 0;
			for (auto& translationValue : translationAttr) {
				{
					nodeAttributes.translation[i] = translationValue.GetFloat();
					i++;
				}
			}
		}

		if (nodeObject.HasMember("scale")) {
			const auto& scaleAttr = nodeObject["scale"].GetArray();
			int i = 0;
			for (auto& scaleValue : scaleAttr) {
				{
					nodeAttributes.scale[i] = scaleValue.GetFloat();
					i++;
				}
			}
		}

		if (nodeObject.HasMember("mesh")) {
			nodeAttributes.meshId = nodeObject["mesh"].GetInt();
		}

		if (nodeObject.HasMember("skin")) {
			nodeAttributes.skinId = nodeObject["skin"].GetInt();
		}
		nodes.push_back(nodeAttributes);
	}
}

void GLTF_Loader::readMeshes(std::unordered_map<std::string, MeshAttributes>& meshes) {

	const auto& meshesArr = modelInfo["meshes"].GetArray();

	for (const auto& mesh : meshesArr) {
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
			}

			if (primitiveObj.HasMember("indices")) {
				primitives.emplace("indices", primitiveObj["indices"].GetInt());
			}
		}
		meshes.emplace(meshName, MeshAttributes(primitives));
	}
}
void GLTF_Loader::readAccessors(std::vector<Accessor>& accessors)
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
void GLTF_Loader::readBufferViews(std::vector<BufferView>& bufferViews)
{
	const auto& buffeViewsArr = modelInfo["bufferViews"].GetArray();
	for (auto& bufferViewIter : buffeViewsArr) {
		const auto& bufferViewObj = bufferViewIter.GetObject();
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
void GLTF_Loader::readBufferInfos(std::vector<BufferInfo>& bufferInfos, const std::string& filepath)
{
	const auto& buffersArr = modelInfo["buffers"].GetArray();
	std::string pathToBin(splitFilename(filepath) + "\\");
	for (auto& bufferInfoIter : buffersArr) {
		const auto& bufferInfoObj = bufferInfoIter.GetObject();
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
void GLTF_Loader::readAnimationSamplers(std::vector<AnimationSampler>& animationSamplers,
	const rapidjson::Value& samplersObj) {
	const auto& samplersArr = samplersObj.GetArray();
	for (const auto& sampler : samplersArr) {
		const auto & samplerObj = sampler.GetObject();
		AnimationSampler animSampler{ samplerObj["input"].GetInt(),
			samplerObj["output"].GetInt(),
			samplerObj["interpolation"].GetString() };
		animationSamplers.push_back(animSampler);
	}

}
void GLTF_Loader::readAnimationChannels(std::vector<AnimationChannel>& animationChannels,
	const rapidjson::Value& cnannelsObj) {
	const auto& channelsArr = cnannelsObj.GetArray();
	for (const auto& channel : channelsArr) {
		const auto& channelObj = channel.GetObject();
		const auto& targetAttributes = channelObj["target"].GetObject();
		AnimationChannel animChannel{ channelObj["sampler"].GetInt(),
			targetAttributes["node"].GetInt(),
			targetAttributes["path"].GetString() };
		animationChannels.push_back(animChannel);
	}

}

void GLTF_Loader::readAnimations()
{
	const auto& animationsArr = modelInfo["animations"].GetArray();

	for (const auto& animation : animationsArr) {
		const auto& animationObject = animation.GetObject();
		const auto& animationName = animationObject["name"].GetString();

		std::vector<AnimationSampler> animationSamplers;
		std::vector<AnimationChannel> animationChannels;
		readAnimationSamplers(animationSamplers, animationObject["samplers"]);
		readAnimationChannels(animationChannels, animationObject["channels"]);

		animations.emplace(animationName, Animation{ animationSamplers, animationChannels });
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

void GLTF_Loader::readSkins(std::unordered_map<std::string, SkinAttributes>& skins)
{
	const auto& skinsArr = modelInfo["skins"].GetArray();
	for (const auto& skin : skinsArr) {
		const auto& skinName = skin.GetObject()["name"].GetString();
		SkinAttributes skinAttributes{};
		skinAttributes.inverseMatrixId = skin.GetObject()["inverseBindMatrices"].GetInt();
		std::vector<int> jointVector{};
		const auto& jointIdArr = skin.GetObject()["joints"].GetArray();
		for (const auto& jointId : jointIdArr){
			jointVector.push_back(jointId.GetInt());
		}
		skinAttributes.joints = jointVector;
		skins.emplace(skinName, skinAttributes);
	}
}

void GLTF_Loader::printNodeInfos(const bool showTRS)
{
	for (const auto& obj : this->nodes) {
		std::cout << "name: " << obj.name << "\n";
		std::cout << "mesh: " << obj.meshId << ", " << "skin: " << obj.skinId << "\n";
		std::cout << "children: " << "\n";
		for (const int& childrenId : obj.children) {
			std::cout << childrenId << ", ";
		}
		std::cout << "\n";
		if (showTRS) {
			std::cout << "rotation: " << "\n";
			for (const float& rotationId : obj.rotation) {
				std::cout << rotationId << ", ";
			}
			std::cout << "\n";

			std::cout << "translation: " << "\n";
			for (const float& translationId : obj.translation) {
				std::cout << translationId << ", ";
			}
			std::cout << "\n";

			std::cout << "scale: " << "\n";
			for (const float& scaleId : obj.scale) {
				std::cout << scaleId << ", ";
			}
			std::cout << "\n";
		}
	}
	std::cout << "\n";
}

void GLTF_Loader::printSkinInfos()
{
	for (const auto& obj : this->skins) {
		std::cout << "name: " << obj.first << "\n";
		std::cout << "inverseMatrixId: " << obj.second.inverseMatrixId << "\n";
		std::cout << "joints: " << "\n";
		for (const int& jointId : obj.second.joints) {
			std::cout << jointId << ", ";
		}
		std::cout << "\n";
	}
}
void GLTF_Loader::printMeshData(const std::string& objectName, const std::string& attributeName) {
	const int accessorIndex = meshes[objectName].attributes[attributeName];
	const auto& accessor = accessors[accessorIndex];
	const auto& bufferView = bufferViews[accessor.bufferView];
	const int bufferSize = bufferView.byteLength;
	const int byteOffset = bufferView.byteOffset;

	switch (accessor.componentType) {
	case UNSIGNED_SHORT: {
		std::vector<unsigned short> scalars(bufferSize / sizeof(unsigned short));
		std::memcpy(scalars.data(), buffers[accessorIndex].data(), bufferSize);
		int i = 0;
		for (auto& obj : scalars) {
			i++;
			std::cout << obj << ", ";
		}
		std::cout << int(i) << " ushorts" << "\n";
		break;
	}
	case SIGNED_BYTE: {
		std::vector<signed char> chars(bufferSize);
		std::memcpy(chars.data(), buffers[accessorIndex].data(), bufferSize);
		int i = 0;
		for (auto& obj : chars) {
			i++;
			std::cout << obj << ", ";
		}
		std::cout << int(i) << " chars" << "\n";
		break;
	}
	case UNSIGNED_BYTE: {
		std::vector<unsigned char> chars(bufferSize / sizeof(unsigned char));
		std::memcpy(chars.data(), (bigBuffers[bufferView.bufferId]).data() + byteOffset, bufferSize);
		int i = 0;
		for (auto& obj : chars) {
			i++;
			std::cout << static_cast<unsigned>(obj) << ", ";
		}
		std::cout << int(i) << " uchars" << "\n";
		break;
	}
	case SIGNED_SHORT: {
		std::vector<short> shorts(bufferSize / sizeof(short));
		std::memcpy(shorts.data(), (bigBuffers[bufferView.bufferId]).data() + byteOffset, bufferSize);
		int i = 0;
		for (auto& obj : shorts) {
			i++;
			std::cout << obj << ", ";
		}
		std::cout << int(i) << " objects" << "\n";
		break;
	}
	case UNSIGNED_INT: {
		std::vector<unsigned int> uints(bufferSize / sizeof(unsigned int));
		std::memcpy(uints.data(), (bigBuffers[bufferView.bufferId]).data() + byteOffset, bufferSize);
		int i = 0;
		for (auto& obj : uints) {
			i++;
			std::cout << obj << ", ";
		}
		std::cout << int(i) << " uints" << "\n";
		break;
	}
	case FLOAT: {
		std::vector<float> floats(bufferSize / sizeof(float));
		std::memcpy(floats.data(), (bigBuffers[bufferView.bufferId]).data() + byteOffset, bufferSize);
		int i = 0;
		for (auto& obj : floats) {
			i++;
			std::cout << obj << ", ";
		}
		std::cout << int(i) << " floats" << "\n";
		break;
	}
	}

}

void GLTF_Loader::printInverseSkinMatrix(const std::string& objectName)
{
	const int accessorIndex = this->skins[objectName].inverseMatrixId;
	const auto& accessor = accessors[accessorIndex];
	const auto& bufferView = bufferViews[accessor.bufferView];
	const int bufferSize = bufferView.byteLength;
	const int byteOffset = bufferView.byteOffset;

	std::vector<float> floats(bufferSize / sizeof(float));
	std::memcpy(floats.data(), (bigBuffers[bufferView.bufferId]).data() + byteOffset, bufferSize);
	int i = 0;
	for (auto& obj : floats) {
		i++;
		if (i % 16 > 0) {
			std::cout << obj << ", ";
		}
		else if(i > 0) {
			std::cout << obj << "}, \n {";
		}
		else if (i == 0) {
			std::cout << "{" << obj << ", ";
		}
		if (i % 4 == 0 && i != 0) std::cout << "\n";
		
	}
	std::cout << "} \n";
	std::cout << int(i) << " floats amount" << "\n";
}
