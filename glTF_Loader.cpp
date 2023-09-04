#include "glTF_Loader.hpp"
#include <array>
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

    // Create a Document object 
    // to hold the JSON data
    Document modelInfo;

    // Parse the JSON data
    modelInfo.Parse(json.c_str());

    // Check for parse errors
    if (modelInfo.HasParseError()) {
        std::cerr << "Error parsing JSON: "
            << modelInfo.GetParseError() << " \n ";
        return;
    }
    const auto& meshesArr = modelInfo["meshes"].GetArray();
    
    for (auto& mesh : meshesArr) {
        //const auto& meshObj = mesh.GetObject();
        const auto& primitives = mesh.GetObject()["primitives"].GetArray();
        for (auto& primitive : primitives) {
            const auto& primitiveObj = primitive.GetObject();
            const auto & attributes = primitiveObj["attributes"].GetObject();
            for (Value::ConstMemberIterator itr = attributes.MemberBegin();
                itr != attributes.MemberEnd(); ++itr)
            {
                std::cout << itr->name.GetString() << ": " << itr->value.GetUint() << "\n";
            }
            if (primitiveObj.HasMember("indices")) {
                const auto& indices = primitiveObj["indices"].GetUint();
                std::cout << "indices: " << indices << "\n";
            }
        }
    }

    const auto& accessorsArr = modelInfo["accessors"].GetArray();
    const auto& buffersArr = modelInfo["buffers"].GetArray();
    std::string currentPath(splitFilename(filepath) + "\\");
    for (auto& bufferInfo : buffersArr) {
        const auto& bufferInfoObj = bufferInfo.GetObject();
        const auto& bufferSize = bufferInfoObj["byteLength"].GetUint();
        const auto& bufferFilePath = currentPath + bufferInfoObj["uri"].GetString();


        std::cout << "buffer at " << bufferFilePath << ", with size = " << bufferSize << "\n";
        std::ifstream file(bufferFilePath, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!" + std::string(bufferFilePath));
        }

        const int firstSize = 288;
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
        file.close();
    }
  
    

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
