#if !defined(ASSETS_H)

#undef global
#undef local_global

#include "libs/assimp/include/assimp/Importer.hpp"
#include "libs/assimp/include/assimp/scene.h"
#include "libs/assimp/include/assimp/postprocess.h"

#define global static
#define local_global static

#define STB_IMAGE_IMPLEMENTATION
#include "libs/stb_image.h"


struct vertex 
{
    v3 Pos;
    v2 UV;
};

struct texture
{
    i32 Width;
    i32 Height;
    u32* Texels;
};

struct mesh
{
    u32 IndexCount;
    u32 IndexOffset;
    u32 VertexOffset;
    u32 VertexCount;
    u32 TextureIndex;
};

struct model
{
    u32 NumMeshes;
    mesh* MeshArray;
    u32 NumTextures;
    texture* TextureArray;

    u32 VertexCount;
    vertex* VertexArray;
    u32 IndexCount;
    u32* IndexArray;

};

#define ASSETS_H
#endif