// ================================
// raytrace.rchit - Closest Hit Shader
// ================================
#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 2, binding = 0) readonly buffer VertexBuffers {
    vec3 vertices[];
} vertexBuffers[];

layout(set = 2, binding = 1) readonly buffer IndexBuffers {
    uint indices[];
} indexBuffers[];

layout(set = 2, binding = 2) readonly buffer NormalBuffers {
    vec3 normals[];
} normalBuffers[];

layout(set = 2, binding = 3) readonly buffer TextureIndexBuffers {
    uint textureIndex;
} textureIndexBuffers[]; //Arrays de 1 numero

layout(set = 2, binding = 4) uniform sampler2D textures[];

layout(location = 0) rayPayloadInEXT vec3 hitValue;
hitAttributeEXT vec3 attribs;

void main() {

    /*uint meshIndex = gl_InstanceID;
    uint primitiveIndex = gl_PrimitiveID;
    
    // Obtener los índices del triángulo
    uint i0 = indexBuffers[meshIndex].indices[primitiveIndex * 3 + 0];
    uint i1 = indexBuffers[meshIndex].indices[primitiveIndex * 3 + 1];
    uint i2 = indexBuffers[meshIndex].indices[primitiveIndex * 3 + 2];
    
    // Obtener los vértices del triángulo
    Vertex v0 = vertexBuffers[meshIndex].vertices[i0];
    Vertex v1 = vertexBuffers[meshIndex].vertices[i1];
    Vertex v2 = vertexBuffers[meshIndex].vertices[i2];*/

    // Coordenadas barycéntricas del hit    
    const vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
    
    // Color basado en coordenadas barycéntricas (para visualización)
    hitValue = barycentricCoords;
}