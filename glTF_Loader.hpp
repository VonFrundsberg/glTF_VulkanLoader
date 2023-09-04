#include "rapidjson/document.h"
#include <iostream>
#include <fstream>
#include <unordered_map>

using namespace rapidjson;

namespace gltf {
    constexpr unsigned int hash(const char* s, int off = 0) {
        return !s[off] ? 5381 : (hash(s, off + 1) * 33) ^ s[off];
    }
    struct  Accessor {
        // types ar efrom GLTF_Loader class
        //const int SCALAR{ 0 };
        //const int VEC2{ 1 };
        //const int VEC3{ 2 };
        int bufferView;
        int componentType;
        int count;
        int type;
    };

    struct BufferView {
        int bufferId;
        int byteLength;
        int byteOffset;
        int target;
    };

    struct BufferInfo {
        int byteLength;
        std::string uri;
    };

    class GLTF_Loader {
    public:

        GLTF_Loader(const std::string& filename);
    private:
            const int SCALAR { 0 };
            const int VEC2   { 1 };
            const int VEC3   { 2 };

        Document modelInfo;
        std::unordered_map<std::string, std::unordered_map<std::string, int>> meshes;
        std::vector<Accessor> accessors;
        std::vector<BufferView> bufferViews;
        std::vector<BufferInfo> bufferInfos;

        void writeMeshesMap(
            std::unordered_map<std::string, std::unordered_map<std::string, int>>& meshes,
            const rapidjson::Document& modelInfo);
        void writeAccessorsVector(
            std::vector<Accessor>& accessors,
            const rapidjson::Document& modelInfo);
        void writeBufferViewsVector(
            std::vector<BufferView>& bufferViews,
            const rapidjson::Document& modelInfo);
        void writeBufferInfosVector(
            std::vector<BufferInfo>& bufferInfos,
            const rapidjson::Document& modelInfo, const std::string& filepath);
        
        int setType(const std::string& stringType) {
            switch (hash(stringType.c_str())) {
            case hash("SCALAR"):
                return 0;
            case hash("VEC2"):
                return 1;
            case hash("VEC3"):
                return 2;
            }
        }
    };
}