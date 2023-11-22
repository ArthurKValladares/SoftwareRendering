#include "obj.h"

#define TINYOBJLOADER_IMPLEMENTATION 
#include "tiny_obj_loader.h"

#include <iostream>
#include <unordered_map>

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

    std::unordered_map<Vertex, u32> uniqueVertices{};
    Mesh mesh;
    // Loop over shapes
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.p = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.uv = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            // TODO: Vertex color
            //vertex.color = { 1.0f, 1.0f, 1.0f };

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(mesh.vertices.size());
                mesh.vertices.push_back(vertex);
            }

            mesh.indices.push_back(uniqueVertices[vertex]);
        }
    }
    return mesh;
}