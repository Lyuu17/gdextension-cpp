
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/variant/color.hpp>

#include "Native/Chunks/ChunkProcessor.hpp"
#include "Native/Models/ModelUtils.hpp"
#include "Native/SWBF2.hpp"

#include "Level.hpp"

namespace SWBF2
{
    void Level::_ready()
    {
        set_name("Level");

        Native::SWBF2::LoadLevelWithGamemode("yav/yav1", "ctf");

        LoadLevelInstances();
    }

    godot::MeshInstance3D *Level::LoadModel(const std::string &id)
    {
        if (!Native::SWBF2::m_models.contains(id))
        {
            return nullptr;
        }

        const auto &model = Native::SWBF2::m_models[id];

        godot::MeshInstance3D *meshInstance = memnew(godot::MeshInstance3D);
        meshInstance->set_name(id.c_str());

        godot::ArrayMesh *arrMesh = memnew(godot::ArrayMesh);
        for (auto const &segment : model.m_segments)
        {
            godot::PackedVector3Array vertices;
            vertices.resize(segment.m_verticesBuf.m_positions.size());
            std::copy(segment.m_verticesBuf.m_positions.begin(), segment.m_verticesBuf.m_positions.end(), reinterpret_cast<Vector3<float>*>(vertices.ptrw()));

            godot::PackedVector3Array normals;
            normals.resize(segment.m_verticesBuf.m_normals.size());
            std::copy(segment.m_verticesBuf.m_normals.begin(), segment.m_verticesBuf.m_normals.end(), reinterpret_cast<Vector3<float>*>(normals.ptrw()));

            godot::PackedColorArray colors;
            colors.resize(segment.m_verticesBuf.m_colors.size());
            std::copy(segment.m_verticesBuf.m_colors.begin(), segment.m_verticesBuf.m_colors.end(), colors.ptrw());

            godot::PackedVector2Array uvs;
            uvs.resize(segment.m_verticesBuf.m_texCoords.size());
            std::copy(segment.m_verticesBuf.m_texCoords.begin(), segment.m_verticesBuf.m_texCoords.end(), reinterpret_cast<Vector2<float>*>(uvs.ptrw()));

            godot::PackedInt32Array indices;
            indices.resize(segment.m_indicesBuf.m_indices.size());
            std::copy(segment.m_indicesBuf.m_indices.begin(), segment.m_indicesBuf.m_indices.end(), indices.ptrw());

            godot::Array arrays;
            arrays.resize(godot::ArrayMesh::ARRAY_MAX);
            arrays[godot::ArrayMesh::ARRAY_VERTEX] = vertices;
            arrays[godot::ArrayMesh::ARRAY_NORMAL] = normals;
            //arrays[godot::ArrayMesh::ARRAY_TANGENT] = tangents;
            if (colors.size() > 0)
                arrays[godot::ArrayMesh::ARRAY_COLOR] = colors;
            arrays[godot::ArrayMesh::ARRAY_TEX_UV] = uvs;
            arrays[godot::ArrayMesh::ARRAY_INDEX] = indices;

            arrMesh->add_surface_from_arrays(Native::ModelUtils::DXtoGLPrimitiveType(segment.m_primitiveType), arrays);

            uint32_t surfaceId = arrMesh->get_surface_count() - 1;
            if (!segment.m_textureNames.empty())
            {
                const auto &mainTextureName = segment.m_textureNames[Native::ModelSegment::TEXTURE_DEFAULT];
                const auto &bumpTextureName = segment.m_textureNames[Native::ModelSegment::TEXTURE_NORMAL];

                auto &material = m_materialPool.getItem(mainTextureName);
                if (material.is_valid())
                {
                    if (!bumpTextureName.empty() && Native::SWBF2::m_tex.contains(bumpTextureName))
                    {
                        auto &bumpTexture = Native::SWBF2::m_tex[bumpTextureName].m_formats[0].m_faceLevels[0].m_gdTexture;

                        material->set_feature(godot::BaseMaterial3D::FEATURE_NORMAL_MAPPING, true);
                        material->set_texture(godot::BaseMaterial3D::TEXTURE_NORMAL, bumpTexture);
                    }

                    // TODO: fix leaves transparency?
                    if (mainTextureName == "leaves")
                        material->set_transparency(godot::BaseMaterial3D::TRANSPARENCY_ALPHA_SCISSOR);

                    if (segment.m_material.m_flags & Native::Material::MATERIAL_TRANSPARENT)
                        material->set_transparency(godot::BaseMaterial3D::TRANSPARENCY_ALPHA);

                    arrMesh->surface_set_material(surfaceId, material);
                }
            }

            if (arrMesh->surface_get_material(surfaceId).is_null())
            {
                godot::UtilityFunctions::printerr(__FILE__, ":", __LINE__, ": Mesh ", id.c_str(), " has no texture at all");
            }
        }

        meshInstance->set_mesh(arrMesh);

        return meshInstance;
    }

    void Level::LoadLevelInstances()
    {
        for (const auto &[worldId, world] : Native::SWBF2::m_worlds)
        {
            for (const auto &inst : world.m_instances)
            {
                godot::MeshInstance3D *mesh = LoadModel(inst.m_type);
                if (!mesh)
                {
                    godot::UtilityFunctions::printerr(__FILE__, ":", __LINE__, ": Missing mesh for instance ", inst.m_type.c_str());
                    continue;
                }

                mesh->set_name(inst.m_name.c_str());
                mesh->translate(inst.m_position);
                mesh->set_basis(inst.m_rotationMatrix);

                add_child(mesh);

                mesh->set_owner(this->get_parent());
                mesh->set_unique_name_in_owner(true);
            }
        }
    }

    void Level::_process(double delta_time)
    {
    }

    void Level::activate(bool active)
    {
    }

    bool Level::active() const
    {
        return true;
    }

    void Level::_bind_methods()
    {
    }
}
