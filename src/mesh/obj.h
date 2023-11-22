#pragma once

#include "mesh.h"

// TODO: string_view?
Mesh load_obj(const std::string& dir, const std::string& obj_file);