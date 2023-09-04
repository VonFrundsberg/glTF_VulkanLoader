#include "rapidjson/document.h"
#include <iostream>
#include <fstream>

using namespace rapidjson;

class GLTF_Loader {
public:
    GLTF_Loader(const std::string& filename);
};