#include <iostream>
#include "glTF_Loader.hpp"
#include <gtx/string_cast.hpp>

int main() {
	auto gltf = GLTF_Loader("glTF/flexing_cube.gltf");
	
	std::cout << "\n";
	std::cout << "NODES:";
	std::cout << "\n";
	gltf.printNodeInfos();
	std::cout << "SKINS:";
	std::cout << "\n";
	gltf.printSkinInfos();
	std::cout << "skins inverse matrices:";
	std::cout << "\n";
	gltf.printInverseSkinMatrix("Armature");
	std::vector<glm::mat4> invMatrices;
	gltf.getInverseSkinMatrices(invMatrices, "Armature");
	for (int i = 0; i < 3; i++) {
		std::cout << glm::to_string(invMatrices[i]) << "\n";
	}
	std::cout << "JOINTS:" << "\n";
	std::vector<int> jointsVector;
	gltf.getJointsVector(jointsVector, "Armature");
	for (int i = 0; i < jointsVector.size(); i++) {
		std::cout << jointsVector[i] << "\n";
	}

	/*std::cout << "JOINTS:";
	std::cout << "\n";
	gltf.printMeshData("Cube.001", "JOINTS_0");
	std::cout << "WEIGHTS:";
	std::cout << "\n";
	gltf.printMeshData("Cube.001", "WEIGHTS_0");
	std::cout << "POSITIONS:";
	std::cout << "\n";
	gltf.printMeshData("Cube.001", "POSITIONS");
	int i = 1;*/

	/*for (const auto& animation : gltf.animations) {
		std::cout << "animation name: " << animation.first << "\n";
		size_t counter = 0;
		for (const auto& sampler : animation.second.samplers) {
			

			std::cout << "input " << sampler.input << "\n";
			std::vector<float> inputs;
			gltf.getAnimationData(inputs, animation.first, counter, "input");
			for (const auto& input : inputs) {
				std::cout << input << ", ";
			}
			std::cout << "\n";


			std::cout << "output " << sampler.output << "\n";
			std::vector<float> outputs;
			gltf.getAnimationData(outputs, animation.first, counter, "output");
			int i = 0;
			for (const auto& output : outputs) {
				std::cout << output << ", ";
				i++;
			}
			std::cout << i << " elements" << "\n";
			counter++;
			
		}

		std::cout << "\n";
		for (const auto& channel : animation.second.channels) {
			std::cout << "output " << channel.targetPath << "\n";
		}
	}*/
	
}