#include <iostream>
#include "glTF_Loader.hpp"

int main() {
	auto gltf = GLTF_Loader("glTF/cube_with_bone_keyframe.gltf");
	
	std::cout << "NODES:";
	std::cout << "\n";
	
	
	for (const auto& obj : gltf.nodes) {
		std::cout << obj.first << ", ";
	}
	std::cout << "\n";

	int i = 1;
	std::vector<unsigned char> joints;
	gltf.getMeshData(joints, "Cube", "JOINTS_0");
	std::cout << "JOINTS:";
	std::cout << "\n {";
	for (auto const& obj : joints) {
		if (i % 4 == 0) {
			std::cout << static_cast<unsigned>(obj) << "}; {";
		}
		else {
			std::cout << static_cast<unsigned>(obj) << ", ";
		}
		i++;
	}
	i = 1;
	std::cout << "\n";


	std::vector<float> weights;
	gltf.getMeshData(weights, "Cube", "WEIGHTS_0");
	std::cout << "WEIGHTS:";
	std::cout << "\n {";
	for (auto const& obj : weights) {
		if (i % 4 == 0) {
			std::cout << obj << "}; {";
		}
		else {
			std::cout << obj << ", ";
		}
		i++;
	}
	i = 1;
	std::cout << "\n";



	std::vector<float> positions;
	gltf.getMeshData(positions, "Cube", "POSITIONS");
	std::cout << "POSITIONS:";
	std::cout << "\n {";
	for (auto const& obj : positions) {
		if (i % 3 == 0) {
			std::cout << obj << "}; {";
		}
		else {
			std::cout << obj << ", ";
		}
		i++;
	}
	std::cout << "\n";

	for (const auto& animation : gltf.animations) {
		std::cout << "animation name: " << animation.first << "\n";
		size_t counter = 0;
		for (const auto& sampler : animation.second.samplers) {
			std::cout << "output " << sampler.output << " at " << counter << "\n";
			std::vector<float> inputs;
			gltf.getAnimationData(inputs, animation.first, counter, "output");
			counter++;
			for (const auto& inputVec : inputs) {
				std::cout << inputVec << ", ";
			}
			std::cout << "\n";
		}
		std::cout << "\n";
		for (const auto& channel : animation.second.channels) {
			std::cout << "output " << channel.targetPath << "\n";
		}
	}
	
}