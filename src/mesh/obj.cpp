#include "obj.h"

#define TINYOBJLOADER_IMPLEMENTATION 
#include "tiny_obj_loader.h"

#include <iostream>

Mesh load_obj(const std::string& path) {
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = "./"; // Path to material files

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(path, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty()) {
    std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    Mesh mesh;
    // Loop over shapes
    for (const auto& shape: shapes) {
        // Loop over faces (polygon)
        size_t index_offset = 0;
        for (const size_t num_fv : shape.mesh.num_face_vertices) {
            // Loop over vertices in the face.
            for (size_t v = 0; v < num_fv; v++) {
                Vertex curr_vertex = {};

                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];

                // access to vertex
                curr_vertex.p.x = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                curr_vertex.p.y = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                curr_vertex.p.z = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0) {
                    curr_vertex.uv.u = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    curr_vertex.uv.v = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                }

                mesh.vertices.emplace_back(std::move(curr_vertex));
            }
            index_offset += num_fv;
        }
        for (const tinyobj::index_t index : shape.mesh.indices) {
            mesh.indices.push_back(index.vertex_index);
        }
    }
    return mesh;
}