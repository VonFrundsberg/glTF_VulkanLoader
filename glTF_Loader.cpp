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

        
            //std::cout << "buffer at " << bufferFilePath << ", with size = " << bufferSize << "\n";
            //std::ifstream file(bufferFilePath, std::ios::binary);
            //if (!file.is_open()) {
            //    throw std::runtime_error("failed to open file!" + std::string(bufferFilePath));
            //}
            /*const int firstSize = 288;
            std::array<float, firstSize / sizeof(float)> buffer;
            std::cout << "positions:" << "\n";
            file.read(reinterpret_cast<char*>(buffer.data()), firstSize);
            int i = 0;
            std::cout << "{";
            for (auto& obj : buffer) {
                i++;
                if (!(i % 3)) {
                    std::cout << obj << "}, {";
                }
                else {
                    std::cout << obj << ", ";
                }

            }
            std::cout << int(i) << "\n";


            int secondSize = 288;
            std::vector<float> buffer2(secondSize / sizeof(float));
            std::cout << "normals:" << "\n";
            file.read(reinterpret_cast<char*>(buffer2.data()), secondSize);
            i = 0;
            for (auto& obj : buffer2) {
                i++;
                std::cout << obj << ", ";
            }
            std::cout << int(i) << "\n";


            std::cout << "UV:" << "\n";
            int thirdSize = 192;
            std::vector<float> buffer3(thirdSize / sizeof(float));

            file.read(reinterpret_cast<char*>(buffer3.data()), thirdSize);
            i = 0;
            for (auto& obj : buffer3) {
                i++;
                std::cout << obj << ", ";
            }
            std::cout << int(i) << "\n";


            std::cout << "indices:" << "\n";
            int fourthSize = 72;
            std::vector<unsigned short> buffer4(fourthSize / sizeof(unsigned short));
            file.read(reinterpret_cast<char*>(buffer4.data()), fourthSize);
            i = 0;
            for (auto& obj : buffer4) {
                i++;
                std::cout << obj << ", ";
            }
            std::cout << int(i) << "\n";
            file.close();*/
        //}



        //for (auto& mesh : meshesArr) {
        //    //const auto& meshObj = mesh.GetObject();
        //    const auto& primitives = mesh.GetObject()["primitives"].GetArray();
        //    for (auto& primitive : primitives) {
        //        const auto& primitiveObj = primitive.GetObject();
        //        const auto& attributes = primitiveObj["attributes"].GetObject();
        //        for (Value::ConstMemberIterator itr = attributes.MemberBegin();
        //            itr != attributes.MemberEnd(); ++itr)
        //        {
        //            std::cout << itr->name.GetString() << ": " << itr->value.GetUint() << "\n";
        //        }
        //        if (primitiveObj.HasMember("indices")) {
        //            const auto& indices = primitiveObj["indices"].GetUint();
        //            std::cout << "indices: " << indices << "\n";
        //        }
        //    }
        //}
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