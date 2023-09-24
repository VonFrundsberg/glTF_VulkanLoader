#include <iostream>
#include "glTF_Loader.hpp"

int main() {
	auto gltf = GLTF_Loader("glTF/cube_2bones_simple_rot.gltf");
	
	std::cout << "NODES:";
	std::cout << "\n";
	gltf.printNodeNames();
	gltf.printMeshData("Cube", "JOINTS_0");
	gltf.printMeshData("Cube", "WEIGHTS_0");
	gltf.printMeshData("Cube", "POSITIONS");
	int i = 1;

	for (const auto& animation : gltf.animations) {
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
	}
	
}