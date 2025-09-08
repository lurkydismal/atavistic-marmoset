#pragma once

#include "runtime.hpp"

void loadMesh( runtime::mesh_t& _mesh,
               bgfx::VertexLayout& _vertexLayout,
               const std::string& _scenePath );
