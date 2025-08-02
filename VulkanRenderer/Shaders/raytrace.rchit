// ================================
// raytrace.rchit - Closest Hit Shader
// ================================
#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 2, binding = 0) readonly buffer VertexBuffers {
    vec4 vertices[];
} vertexBuffers[];

layout(set = 2, binding = 1) readonly buffer IndexBuffers {
    uint indices[];
} indexBuffers[];

layout(set = 2, binding = 2) readonly buffer NormalBuffers {
    vec4 normals[];
} normalBuffers[];

layout(set = 2, binding = 3) readonly buffer TextureIndexBuffers {
    int textureIndex[];
} textureIndexBuffers; //Arrays de 1 numero

layout(set = 2, binding = 4) restrict readonly buffer ColorBuffer {
    vec4 colors[];
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
    vec3 v0 = vertexBuffers[meshIndex].vertices[i0].xyz;
    vec3 v1 = vertexBuffers[meshIndex].vertices[i1].xyz;
    vec3 v2 = vertexBuffers[meshIndex].vertices[i2].xyz;

    vec3 n0 = normalBuffers[meshIndex].normals[i0].xyz;
    vec3 n1 = normalBuffers[meshIndex].normals[i1].xyz;
    vec3 n2 = normalBuffers[meshIndex].normals[i2].xyz;


    // Coordenadas barycéntricas del hit    
    const vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
    
    // Calcular la normal geométrica del triángulo
    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;
    vec3 geometricNormal = normalize(cross(edge1, edge2));


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

    //condicion de terminación

    // CORRECCIÓN: Decidir qué normal usar basándose en su consistencia
    vec3 finalNormal;
    
    // Verificar si la normal interpolada es consistente con la geométrica
    float similarity = dot(geometricNormal, interpolatedNormal);
    
    if (abs(similarity) > 0.5) {
        // Las normales son razonablemente consistentes
        if (similarity < 0.0) {
            // La normal interpolada apunta en dirección opuesta, voltearla
            finalNormal = -interpolatedNormal;
        } else {
            // Usar la normal interpolada tal como está
            finalNormal = interpolatedNormal;
        }
    } else {
        // Las normales son muy inconsistentes, usar la geométrica
        finalNormal = geometricNormal;
    }
    
    if (dot(finalNormal, -gl_WorldRayDirectionEXT) < 0.0) {
    finalNormal = -finalNormal; // Flip if facing away
    }

    // Asegurar que la normal esté normalizada
    finalNormal = normalize(finalNormal);

    /////////////////////////////////////////////
    ///////////CONDICION TERMINACIÓN////////////////    
    /////////////////////////////////////////////////

    if(textureIndexBuffers.textureIndex[meshIndex] >=0){
        rayPayload.color = colorBuffer.colors[meshIndex].xyz;
        //rayPayload.color = vec3(1.0, 0.0, 1.0); 
        rayPayload.hit = true;
    }else{


    int debugColor = 2;


    //Color a pasar
    vec3 baseColor;
    
    switch(debugColor){
    
    case 1:
    //Color en funcion de rebotes
    
    if (rayPayload.depth == 0) {
        baseColor = vec3(1.0,0.0,0.0);   // Rojo - primer impacto
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
    break;

   case 2:
    
    // Calcular la dirección hacia la cámara (inversa del rayo incidente)
    vec3 viewDirection = -normalize(gl_WorldRayDirectionEXT);
    
    // Calcular el factor de sombreado basado en el ángulo entre normal y dirección de vista
    // dot(normal, viewDir) da 1 cuando son paralelos (perpendicular a la superficie vista de frente)
    // y 0 cuando son perpendiculares (superficie vista de lado)
    float shadingFactor = dot(finalNormal, viewDirection);
    
    // Asegurar que el factor esté en el rango [0, 1]
    shadingFactor = max(0.0, shadingFactor);

    //Shading básico para el color base
    baseColor = colorBuffer.colors[meshIndex].xyz * (shadingFactor); 

    break;

    case 3:
    //Color plano
    baseColor = colorBuffer.colors[meshIndex].xyz;
    break;

    case 4: // Visualizar normales como colores RGB
    baseColor = normalize((interpolatedNormal + 1.0) * 0.5); // Convertir de [-1,1] a [0,1]
    break;

    
    case 5: // Visualizar normales como colores RGB
    baseColor = normalize((geometricNormal + 1.0) * 0.5); // Convertir de [-1,1] a [0,1]
    break;

    case 6:
    baseColor = vec3(hitPosition.x/2.0,hitPosition.z/2,0);
    break;

    default:

    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;
    vec3 geometricNormal = normalize(cross(edge1, edge2));
            
    // Comparar con normal interpolada
    float similarity = dot(geometricNormal, interpolatedNormal);
            
    if (similarity > 0.8) {
        baseColor = vec3(0.0, 1.0, 0.0); // Verde: consistentes
    } else if (similarity > 0.0) {
        baseColor = vec3(1.0, 1.0, 0.0); // Amarillo: parcialmente consistentes
    } else {
        baseColor = vec3(1.0, 0.0, 0.0); // Rojo: inconsistentes
    }
    break;
    }

     vec3 incomingDirection = gl_WorldRayDirectionEXT;
     const int MAX_DEPTH = 2;


   // Si no hemos alcanzado la profundidad máxima, lanzar rayo de reflexión
    if (rayPayload.depth < MAX_DEPTH) {
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

        //Cambiar a false para no propagar color
        rayPayload.hit = false;
    }
    }
    //rayPayload.color = baseColor; 
    
}