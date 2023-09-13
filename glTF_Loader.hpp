#pragma once
#include "rapidjson/document.h"
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <variant>

using namespace rapidjson;

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
        template <typename VectorType>
        void getData(std::vector<VectorType>& dstVector, const std::string& objectName, const std::string& attributeName) {
            const int accessorIndex = this->meshes[objectName][attributeName];
            const auto& accessor = accessors[accessorIndex];
            const auto& bufferView = bufferViews[accessor.bufferView];
            const int bufferSize = bufferView.byteLength;
            const int byteOffset = bufferView.byteOffset;

            dstVector.resize(bufferSize / sizeof(VectorType));
            std::memcpy(dstVector.data(), (bigBuffers[bufferView.bufferId]).data() + byteOffset, bufferSize);
        }
        std::unordered_map<std::string, std::unordered_map<std::string, int>> meshes;
    private:
        static const int SCALAR{ 0 };
        static const int VEC2{ 1 };
        static const int VEC3{ 2 };
        static const int VEC4{ 3 };

        static const int SIGNED_BYTE{ 5120 };
        static const int UNSIGNED_BYTE{ 5121 };
        static const int SIGNED_SHORT{ 5122 };
        static const int UNSIGNED_SHORT{ 5123 };
        static const int UNSIGNED_INT{ 5125 };
        static const int FLOAT{ 5126 };

        Document modelInfo;
        std::vector<Accessor> accessors;
        std::vector<BufferView> bufferViews;
        std::vector<BufferInfo> bufferInfos;
        std::vector<std::vector<char>> buffers;
        std::vector<std::vector<char>> bigBuffers;

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

        void writeBuffers();
        void writeBigBuffers();


        std::variant<
            std::vector<unsigned short>,
            std::vector<signed char>,
            std::vector<unsigned char>,
            std::vector<short>,
            std::vector<unsigned int>,
            std::vector<float>>   getDataAuto(const std::string& objectName, const std::string& attributeName);


        void convertBuffers();
        
        int convertStringToIntType(const std::string& stringType) {
            switch (hash(stringType.c_str())) {
            case hash("SCALAR"):
                return SCALAR;
            case hash("VEC2"):
                return VEC2;
            case hash("VEC3"):
                return VEC3;
            case hash("VEC4"):
                return VEC4;
            }
        }
    };