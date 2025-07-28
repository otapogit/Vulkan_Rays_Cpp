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
    int textureIndex[];
} textureIndexBuffers; //Arrays de 1 numero

layout(set = 2, binding = 4) restrict readonly buffer ColorBuffer {
    vec3 colors[];
} colorBuffer;

layout(set = 2, binding = 5) uniform sampler2D textures[];

struct RayPayload {
    vec3 color;
    int depth;
    bool hit;
};


layout(location = 0) rayPayloadInEXT RayPayload rayPayload;
layout(location = 1) rayPayloadEXT RayPayload reflectionRayPayload;
hitAttributeEXT vec3 attribs;

layout(binding = 1, set = 0) uniform accelerationStructureEXT topLevelAS;

void main() {

    uint meshIndex = gl_InstanceCustomIndexEXT;
    uint primitiveIndex = gl_PrimitiveID;
    
    // Obtener los índices del triángulo
    uint i0 = indexBuffers[meshIndex].indices[primitiveIndex * 3 + 0];
    uint i1 = indexBuffers[meshIndex].indices[primitiveIndex * 3 + 1];
    uint i2 = indexBuffers[meshIndex].indices[primitiveIndex * 3 + 2];
    
    // Obtener los vértices del triángulo
    vec3 v0 = vertexBuffers[meshIndex].vertices[i0];
    vec3 v1 = vertexBuffers[meshIndex].vertices[i1];
    vec3 v2 = vertexBuffers[meshIndex].vertices[i2];

    vec3 n0 = normalBuffers[meshIndex].normals[i0];
    vec3 n1 = normalBuffers[meshIndex].normals[i1];
    vec3 n2 = normalBuffers[meshIndex].normals[i2];


    // Coordenadas barycéntricas del hit    
    const vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
    

    vec3 interpolatedNormal = normalize(
        n0 * barycentricCoords.x + 
        n1 * barycentricCoords.y + 
        n2 * barycentricCoords.z
    );
    
    // Interpolar la posición del hit
    vec3 hitPosition = 
        v0 * barycentricCoords.x + 
        v1 * barycentricCoords.y + 
        v2 * barycentricCoords.z;

    /*
    if(textureIndexBuffers.textureIndex[meshIndex] >=0){
        rayPayload.color = colorBuffer.colors[meshIndex];
        rayPayload.hit = true;
    }*/
    /*
    hitValue = vec3(
        float((meshIndex + 1) % 2), 
        float((meshIndex + 1) / 2 % 2), 
        float((meshIndex + 1) / 4 % 2)
    );*/

    vec3 baseColor;
    
    if (rayPayload.depth == 0) {
        //int l = textureIndexBuffers.textureIndex[meshIndex];
        //baseColor = colorBuffer.colors[meshIndex]; // Rojo - primer impacto
        baseColor = vec3(1.0,0.0,0.0);
    } else if (rayPayload.depth == 1) {
        baseColor = vec3(0.0, 1.0, 0.0); // Verde - primer rebote
    } else if (rayPayload.depth == 2) {
        baseColor = vec3(0.0, 0.0, 1.0); // Azul - segundo rebote
    } else if (rayPayload.depth == 3) {
        baseColor = vec3(1.0, 1.0, 0.0); // Amarillo - tercer rebote
    } else if (rayPayload.depth == 4) {
        baseColor = vec3(1.0, 0.0, 1.0); // Magenta - cuarto rebote
    } else {
        baseColor = vec3(0.0, 1.0, 1.0); // Cian - quinto rebote o más
    }

    baseColor = colorBuffer.colors[meshIndex]; // Rojo - primer impacto

    const int MAX_DEPTH = 2;

    vec3 incomingDirection = gl_WorldRayDirectionEXT;

   // Si no hemos alcanzado la profundidad máxima, lanzar rayo de reflexión
    if (rayPayload.depth < MAX_DEPTH && dot(incomingDirection, interpolatedNormal) > 0.0) {
        // Calcular dirección de reflexión
        vec3 incomingDirection = gl_WorldRayDirectionEXT;
        vec3 reflectedDirection = reflect(incomingDirection, interpolatedNormal);
        
        // Configurar el payload para el rayo de reflexión
        reflectionRayPayload.color = vec3(0.0);
        reflectionRayPayload.depth = rayPayload.depth + 1;
        reflectionRayPayload.hit = false;
        
        // Offset pequeño para evitar self-intersection
        float epsilon = 0.001;
        vec3 rayOrigin = hitPosition;
        
        // Lanzar rayo de reflexión
        traceRayEXT(
            topLevelAS,           // acceleration structure
            gl_RayFlagsOpaqueEXT, // ray flags
            0xFF,                 // cull mask
            0,                    // sbt record offset
            0,                    // sbt record stride
            0,                    // miss index (usa el mismo miss shader)
            rayOrigin,            // ray origin
            0.001,                // ray min distance
            reflectedDirection,   // ray direction
            1000.0,               // ray max distance
            1                     // payload location
        );
        
        // Combinar colores basado en si el rayo de reflexión impactó algo
        if (reflectionRayPayload.hit) {
            // Mezclar el color base con el color de la reflexión
            rayPayload.color = reflectionRayPayload.color;
            rayPayload.hit = reflectionRayPayload.hit;
        } else {
            // No hay más reflexiones, usar solo el color base
            rayPayload.color = baseColor;
            rayPayload.hit = reflectionRayPayload.hit;
        }
    } else {
        // Profundidad máxima alcanzada
        rayPayload.color = baseColor; 
        rayPayload.hit = true;
    }
    //rayPayload.color = baseColor; 
    
}