#include "obj.h"

#define TINYOBJLOADER_IMPLEMENTATION 
#include "tiny_obj_loader.h"

#include <iostream>
#include <unordered_map>
#include <string>
#include <limits>

Mesh load_obj(const std::string& dir, const std::string& obj_file, SDL_Surface* surface) {
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = dir; // Path to material files
    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(dir + "/" + obj_file, reader_config)) {
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

    TextureMap texture_map;
    for (const auto& material : materials) {
        const auto add_to_map = [&](const std::string& str) {
            if (texture_map.count(str) == 0) {
                const std::string texture_path = dir + "/" + str;
                Texture texture = Texture(texture_path.c_str(), surface);
                texture_map.emplace(texture_path, texture);
            }
        };
        if (material.ambient_texname != "") {
            add_to_map(material.ambient_texname);
        }
        if (material.diffuse_texname != "") {
            add_to_map(material.diffuse_texname);
        }
        if (material.specular_texname != "") {
            add_to_map(material.specular_texname);
        }
        if (material.specular_highlight_texname != "") {
            add_to_map(material.specular_highlight_texname);
        }
        if (material.bump_texname != "") {
            add_to_map(material.bump_texname);
        }
        if (material.displacement_texname != "") {
            add_to_map(material.displacement_texname);
        }
        if (material.alpha_texname != "") {
            add_to_map(material.alpha_texname);
        }
        if (material.reflection_texname != "") {
            add_to_map(material.reflection_texname);
        }
    }
    std::unordered_map<Vertex, u32> uniqueVertices{};
    Mesh mesh;
    Rect3D_f bb {
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::min(),
        std::numeric_limits<float>::min(),
        std::numeric_limits<float>::min()
    };
    // Loop over shapes
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.p = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            bb.minX = MIN(bb.minX, vertex.p.x);
            bb.maxX = MAX(bb.maxX, vertex.p.x);

            bb.minY = MIN(bb.minY, vertex.p.y);
            bb.maxY = MAX(bb.maxY, vertex.p.y);

            bb.minZ = MIN(bb.minZ, vertex.p.z);
            bb.maxZ = MAX(bb.maxZ, vertex.p.z);

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
    mesh.bb = std::move(bb);
    mesh.SetupTriangles();
    mesh.texture_map = std::move(texture_map);
    return mesh;
}