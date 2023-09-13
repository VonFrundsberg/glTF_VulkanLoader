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

    struct MeshAttributes {
        std::unordered_map<std::string, int> attributes;
        MeshAttributes() {};
        MeshAttributes(const std::unordered_map<std::string, int>& argAttributes) :
            attributes(argAttributes){};
    };

    struct NodeAttributes {
        std::unordered_map<std::string, int> attributes;
        NodeAttributes() {};
        NodeAttributes(const std::unordered_map<std::string, int>& argAttributes) :
            attributes(argAttributes) {};
    };

    struct AnimationSampler {
        int input;
        int output;
        std::string interpolation;
    };

    struct AnimationChannel {
        int samplerId;
        int targetNodeId;
        std::string targetPath;
    };

    struct Animation {
        std::vector<AnimationSampler> samplers;
        std::vector<AnimationChannel> channels;
    };

    struct Node {
        std::string name;
    };

    class GLTF_Loader {
    public:

        GLTF_Loader(const std::string& filename);

        template <typename VectorType>
        void getMeshData(
            std::vector<VectorType>& dstVector, const std::string & objectName, const std::string& attributeName) {
            const int accessorIndex  = meshes[objectName].attributes[attributeName];
            const auto& accessor = accessors[accessorIndex];
            const auto& bufferView = bufferViews[accessor.bufferView];
            const int bufferSize = bufferView.byteLength;
            const int byteOffset = bufferView.byteOffset;

            dstVector.resize(bufferSize / sizeof(VectorType));
            std::memcpy(dstVector.data(), (bigBuffers[bufferView.bufferId]).data() + byteOffset, bufferSize);
        }

        std::unordered_map<std::string, MeshAttributes> meshes;
        std::unordered_map<std::string, NodeAttributes> nodes;
        std::unordered_map<std::string, Animation> animations;
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
        std::vector<AnimationSampler> animationSamplers;


        std::vector<std::vector<char>> buffers;
        std::vector<std::vector<char>> bigBuffers;


        void readAnimations();
        void readAnimationSamplers(std::vector<AnimationSampler>& animationSamplers, 
            const rapidjson::Value& samplersArr);

        void readAnimationChannels(std::vector<AnimationChannel>& animationChannels,
            const rapidjson::Value& cnannelsObj);


        void readNodes( std::unordered_map<std::string, NodeAttributes>& nodes);

        void readMeshes(std::unordered_map<std::string, MeshAttributes>& meshes);


        void readAccessors(std::vector<Accessor>& accessors);
        void readBufferViews(std::vector<BufferView>& bufferViews);
        void readBufferInfos(std::vector<BufferInfo>& bufferInfos, const std::string& filepath);


        void writeBuffers();
        void writeBigBuffers();


        /*std::variant<
            std::vector<unsigned short>,
            std::vector<signed char>,
            std::vector<unsigned char>,
            std::vector<short>,
            std::vector<unsigned int>,
            std::vector<float>>   getDataAuto(const int objectId, const std::string& attributeName);*/


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