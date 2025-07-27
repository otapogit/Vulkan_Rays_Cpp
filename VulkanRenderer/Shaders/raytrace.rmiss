// ================================
// raytrace.rmiss - Miss Shader
// ================================
#version 460
#extension GL_EXT_ray_tracing : require

struct RayPayload {
    vec3 color;
    int depth;
    bool hit;
};

layout(location = 0) rayPayloadInEXT RayPayload hitValue;

void main() {
    // Color de fondo (cielo azul claro)
    hitValue.color = vec3(0.7, 0.1, 0.3);
    hitValue.hit = false;
}