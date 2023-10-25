#pragma once
#include "rapidjson/document.h"
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <variant>
#include <glm.hpp>
#include <gtc/type_ptr.hpp>
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

    struct SkinAttributes {
        int inverseMatrixId;
        std::vector<int> joints;
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
        std::string name;
        int meshId{-1};
        int skinId{-1};
        std::vector<int> children{};
        std::vector<float> translation{ 0.0f, 0.0f, 0.0f};
        std::vector<float> rotation{ 0.0f, 0.0f, 0.0f, 0.0f};
        std::vector<float> scale{ 0.0f, 0.0f, 0.0f};
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
            const size_t bufferSize = bufferView.byteLength;
            const size_t byteOffset = bufferView.byteOffset;

            dstVector.resize(bufferSize / sizeof(VectorType));
            std::memcpy(dstVector.data(), (bigBuffers[bufferView.bufferId]).data() + byteOffset, bufferSize);
        }

        void getJointsVector(std::vector<int>& dstJointsVector, const std::string& objectName) {
            //const int vectorSize = skins[objectName].joints.size();
            //dstJointsVector.resize(vectorSize);
            dstJointsVector = skins[objectName].joints;
        }

        void getInverseSkinMatrices(
            std::vector<glm::mat4>& dstMatricesVector, const std::string& objectName) {
            const int accessorIndex = this->skins[objectName].inverseMatrixId;
            const auto& accessor = accessors[accessorIndex];
            const auto& bufferView = bufferViews[accessor.bufferView];
            const int bufferSize = bufferView.byteLength;
            const int byteOffset = bufferView.byteOffset;
            const int vectorSize = skins[objectName].joints.size();
            dstMatricesVector.resize(vectorSize);
            for (int i = 0; i < vectorSize; i++) {
                std::memcpy(glm::value_ptr(dstMatricesVector[i]),
                    (bigBuffers[bufferView.bufferId]).data() + byteOffset + i*16*sizeof(float),
                    int(bufferSize/ vectorSize));
            }


        }

        /*template <typename VectorType>
        void getNodeData(
            std::vector<VectorType>& dstVector, const std::string& objectName, const std::string& attributeName) {
            const int accessorIndex = meshes[objectName].attributes[attributeName];
            const auto& accessor = accessors[accessorIndex];
            const auto& bufferView = bufferViews[accessor.bufferView];
            const int bufferSize = bufferView.byteLength;
            const int byteOffset = bufferView.byteOffset;

            dstVector.resize(bufferSize / sizeof(VectorType));
            std::memcpy(dstVector.data(), (bigBuffers[bufferView.bufferId]).data() + byteOffset, bufferSize);
        }*/

        template <typename VectorType>
        void getAnimationData(
            std::vector<VectorType>& dstVector,
            const std::string& objectName, const int samplerId,
            const std::string& attributeName) {
            int accessorIndex;
            if (attributeName == "output") {
                accessorIndex = animations[objectName].samplers[samplerId].output;
            }
            else if (attributeName == "input") {
                accessorIndex = animations[objectName].samplers[samplerId].input;
            }
            else {
                std::cout << "there was a problem while reading attributeName in getAnimationData" << "\n";
                return;
            }
            const auto& accessor = accessors[accessorIndex];
            const auto& bufferView = bufferViews[accessor.bufferView];
            const int bufferSize = bufferView.byteLength;
            const int byteOffset = bufferView.byteOffset;

            dstVector.resize(bufferSize / sizeof(VectorType));
            std::memcpy(dstVector.data(), (bigBuffers[bufferView.bufferId]).data() + byteOffset, bufferSize);
        }

        std::unordered_map<std::string, MeshAttributes> meshes;
        std::vector<NodeAttributes> nodes;
        std::unordered_map<std::string, SkinAttributes> skins;
        std::unordered_map<std::string, Animation> animations;

        void printNodeInfos(const bool showTRS = false);
        void printSkinInfos();

        void printMeshData(const std::string& objectName, const std::string& attributeName);
        void printInverseSkinMatrix(const std::string& objectName);
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


        void readNodes(std::vector<NodeAttributes> &nodes);
        void readSkins(std::unordered_map<std::string, SkinAttributes>& skins);
        void readMeshes(std::unordered_map<std::string, MeshAttributes>& meshes);


        void readAccessors(std::vector<Accessor>& accessors);
        void readBufferViews(std::vector<BufferView>& bufferViews);
        void readBufferInfos(std::vector<BufferInfo>& bufferInfos, const std::string& filepath);


        void writeBuffers();
        void writeBigBuffers();
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