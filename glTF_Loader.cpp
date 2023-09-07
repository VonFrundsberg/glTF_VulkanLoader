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
namespace gltf {

    
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

        const int bufferInfosSize = bufferInfos.size();
        const int bufferViewsSize = bufferViews.size();
        const int accessorsSize = accessors.size();

        for (int bufferIndex = 0; bufferIndex < bufferInfosSize; bufferIndex++) {
            const auto& bufferInfo = bufferInfos[bufferIndex];

            std::ifstream binaryFile(bufferInfo.uri, std::ios::binary);
            if (!binaryFile.is_open()) {
                throw std::runtime_error("failed to open file!" + std::string(bufferInfo.uri));
            }
            for (int accessorIndex = 0; accessorIndex < accessorsSize; accessorIndex++) {
                auto& accessor = accessors[accessorIndex];
                auto& bufferView = bufferViews[accessor.bufferView];
                int bufferSize = bufferView.byteLength;
                int byteOffset = bufferView.byteOffset;

                binaryFile.seekg(byteOffset);

                switch (accessor.type) {
                case SCALAR: {
                    std::cout << "scalars:" << "\n";
                    std::vector<unsigned short> scalars(bufferSize / sizeof(unsigned short));
                    binaryFile.read(reinterpret_cast<char*>(scalars.data()), bufferView.byteLength);
                    int i = 0;
                    for (auto& obj : scalars) {
                        i++;
                        std::cout << obj << ", ";
                    }
                    std::cout << int(i) << "\n";
                    break;
                }
                case VEC2: {
                    std::cout << "vec2s" << "\n";
                    std::vector<float> vec2s(bufferSize / sizeof(float));
                    binaryFile.read(reinterpret_cast<char*>(vec2s.data()), bufferSize);
                    int i = 0;
                    for (auto& obj : vec2s) {
                        i++;
                        std::cout << obj << ", ";
                    }
                    std::cout << int(i) << "\n";
                    break;
                }
                case VEC3: {
                    std::vector<float> vec3s(bufferSize / sizeof(float));
                    std::cout << "vec3s:" << "\n";
                    binaryFile.read(reinterpret_cast<char*>(vec3s.data()), bufferSize);
                    int i = 0;
                    for (auto& obj : vec3s) {
                        i++;
                            std::cout << obj << ", ";
                    }
                    std::cout << int(i) << "\n";
                    break;
                }
                }

            }
        }
        

        

        
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
                this->setType(accessorInfoObj["type"].GetString()) };
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
                bufferViewObj["target"].GetInt()};
            bufferViews.push_back(bufferView);
        }
    }

    void GLTF_Loader::writeBufferInfosVector(
        std::vector<BufferInfo>& bufferInfos,
        const rapidjson::Document& modelInfo, const std::string & filepath)
    {
        const auto& buffersArr = modelInfo["buffers"].GetArray();
        std::string pathToBin(splitFilename(filepath) + "\\");
        for (auto& bufferInfo : buffersArr) {
            const auto& bufferInfoObj = bufferInfo.GetObject();
            BufferInfo bufferInfo{
                bufferInfoObj["byteLength"].GetInt(),
                pathToBin + bufferInfoObj["uri"].GetString()};
            bufferInfos.push_back(bufferInfo);
        }
    }
}