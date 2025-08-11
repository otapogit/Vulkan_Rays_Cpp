#include <GlRenderer.h>
#include <sstream>

#include <glContext.h>
#include <glContextCreator.h>
#include <model.h>
#include <mesh.h>
#include <fbo.h>
#include <texture2D.h>
#include <drawCommand.h>
#include <bvh.h>
#include <meshData.h>
#include <meshUtils.h>
#include <RTDataManager.h>

GLRenderer::GLRenderer()
{
}

GLRenderer::~GLRenderer()
{
}



bool GLRenderer::init()
{
	ctx = autis::GLContextCreator::create();
	if (!ctx) return false;
	return true;
}

Renderer::MeshId GLRenderer::defineMesh(const std::vector<glm::vec3>& vtcs, const std::vector<glm::vec3>& nrmls, const std::vector<glm::vec2>& uv, const std::vector<uint32_t> inds)
{
	auto m = std::make_shared<autis::Mesh>();
	m->setKeepDataInCPU();
	m->addVertices(vtcs);
	m->addNormals(nrmls);
	if (!uv.empty()) 
		m->addTexCoord(0, uv);
	m->addIndices(inds);

	m->addDrawCommand(new autis::DrawElements(GL_TRIANGLES, inds.size(), GL_UNSIGNED_INT, nullptr));

	auto model = std::make_shared<autis::Model>();
	model->addMesh(m);

	MeshId id = models.size();

	std::cout << "Mesh " << id << " " << m->getBB() << "\n";

	models.push_back(model);
	return id;
}

bool GLRenderer::addMesh(const glm::mat4& modelMatrix, const glm::vec3& color, MeshId id, bool inspectable)
{
	if (inspectable)
		renderModels.push_back({ modelMatrix, color, id });
	else
		occludingModels.push_back({ modelMatrix, color, id });
	recomputeBVH = true;
	return false;
}

Renderer::TextureId GLRenderer::addTexture(uint8_t* texels, uint32_t width, uint32_t height, uint32_t bpp)
{
	return TextureId();
}

void GLRenderer::deleteTexture(TextureId tid)
{
}

bool GLRenderer::addLight(const glm::mat4& modelMatrix, MeshId id, const glm::vec3& color, LightId lid, TextureId tid)
{
	lightModels.push_back({ modelMatrix, color, id, lid, tid });
	recomputeBVH = true;
	return true;
}

bool GLRenderer::removeMesh(MeshId id)
{
	models[id].reset();
	recomputeBVH = true;
	return false;
}

void GLRenderer::clearScene()
{
	renderModels.clear();
	lightModels.clear();
	models.clear();
	recomputeBVH = true;
}

void GLRenderer::setViewCamera(const glm::mat4& viewMatrix, const glm::mat4& projMatrix)
{
	this->viewMatrix = viewMatrix;
	this->projMatrix = projMatrix;
}

void GLRenderer::setInspCamera(const glm::mat4& viewMatrix, const glm::mat4& projMatrix)
{
	inspViewMatrix = viewMatrix;
	inspProjMatrix = projMatrix;
}

void GLRenderer::setOutputResolution(uint32_t width, uint32_t height)
{
	if (resolution.x == width && resolution.y == height)
		return;
	resolution = { width, height };
	fbo.reset();
}

bool GLRenderer::render()
{
	if (!fbo && !prepareFBO()) return false;
	if (!program && !prepareProgram()) return false;
	if (recomputeBVH) 
		if (!prepareNodeTable()) return false;

	ctx->bindFBO(fbo, GL_FRAMEBUFFER);

	ctx->useProgram(program);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glViewport(0, 0, resolution.x, resolution.y);

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);


	glUniformMatrix4fv(viewMatrixLoc, 1, false, &viewMatrix[0][0]);
	glUniformMatrix4fv(inspViewMatrixLoc, 1, false, &inspViewMatrix[0][0]);
	glUniformMatrix4fv(inspProjMatrixLoc, 1, false, &inspProjMatrix[0][0]);
	glm::vec3 trans = glm::vec3(inspViewMatrix[3]);
	auto inspCameraPos = -(glm::transpose(glm::mat3(inspViewMatrix)) * trans);
	glUniform3fv(inspCamPosLoc, 1, &inspCameraPos.x);

	for (auto i = 0ULL; i < renderModels.size(); i++) {
		auto mv = viewMatrix * renderModels[i].modelMatrix;
		auto nm = glm::mat3(glm::transpose(glm::inverse(mv)));
		auto mvp = projMatrix * mv;

		glUniformMatrix4fv(mvpLoc, 1, false, &mvp[0][0]);
		glUniformMatrix4fv(modelMatrixLoc, 1, false, &(renderModels[i].modelMatrix[0][0]));

		auto& model = *models[renderModels[i].modelIndex];
		auto& mesh = model.getMesh(autis::MeshId(0));

		model.render(*ctx);
	}

	return true;
}

size_t GLRenderer::copyResultBytes(uint8_t* buffer, size_t bufferSize)
{
	return size_t();
}

uint32_t GLRenderer::getResultTextureId()
{
	return 0;
}

bool GLRenderer::saveResultToFile(const std::string& filename)
{
	auto finalTex = std::static_pointer_cast<autis::Texture2D>(fbo->getAttachedTexture(GL_COLOR_ATTACHMENT0));
	finalTex->save(filename);
	return true;
}

bool GLRenderer::prepareFBO()
{
	fbo = ctx->createFBO();
	fbo->createAndAttach(GL_DEPTH_ATTACHMENT, resolution.x, resolution.y);

	auto color = std::make_shared<autis::Texture2D>();
	color->allocate(resolution.x, resolution.y, GL_RGBA8);

	fbo->attach(GL_COLOR_ATTACHMENT0, color);
	return fbo->isComplete();
}


// hay dos cámaras: la cámara de inspección (que simula una cámara de inspección real del túnel) y la cámara
// de la vista, que es la que se usa para generar la imagen final (en el simulador se puede mover interactivamente)
// ambas pueden coincidir: en ese caso, la imagen resultado es la que capturaría la cámara de inspección
std::vector<std::string> vshader{
	"#version 450 core",

	// cámara de la vista
	"uniform mat4 modelviewprojMatrix;",

	// cámara de inspección
	"uniform mat4 inspViewMatrix;",

	// matriz del modelo de la malla
	"uniform mat4 modelMatrix;",
	"layout(location = 0) in vec4 position;",
	"layout(location = 1) in vec3 normal;",
	
	"out vec3 wN;",
	"out vec4 wP;",
	"out vec4 eP;",
	
	"void main() {",

	"  wN = normalize(vec3(modelMatrix * vec4(normal, 0.0)));",
	"  wP = modelMatrix * position;"

	"  mat4 inspectCamModelViewMatrix = inspViewMatrix * modelMatrix;",
	"  eP = inspectCamModelViewMatrix * position;"
	"  gl_Position = modelviewprojMatrix * position;"
	"}"
};

std::vector<std::string> fshader{
	"#version 450 core",
	"uniform vec4 diffuseColor = vec4(0.8, 0.8, 0.8, 1);",
	"uniform mat4 viewMatrix;",
	"uniform vec3 inspCamPos;",
	"uniform mat4 inspProjMatrix;",
	"in vec3 wN;",
	"in vec4 wP;",
	"in vec4 eP;",

	"out vec4 final_color;",

	"$RayTracingData",

	"bool isInsideFrustum(vec4 ePosition, in mat4 projMatrix) {",
	"  vec4 projPos = projMatrix * ePosition;",
	"  bool outsideFrustum = any(greaterThan(projPos.xyz , projPos.www)) || any(lessThan(projPos.xyz , -projPos.www));",
	"  return !outsideFrustum;",
	"}",


	// RAYTRACING FUNCTIONS
	"uint getVIndex(uint meshIndex, uint localTriangle, uint index) {",
	"  uint realTriIndex = triangleIndexTable[localTriangle + meshInfo[meshIndex].triIndsBase];",
	"  uint indexBase = meshInfo[meshIndex].indsBase;",
	"  uint vtxBase = meshInfo[meshIndex].vtcsBase;",
	"  uint i = indexTable[realTriIndex * 3 + index + indexBase] + vtxBase;",
	"  return i;",
	"}",
	"vec3 getWV0(uint meshIndex, uint localTriangle) {",
	"  uint i0 = getVIndex(meshIndex, localTriangle, 0);",
	"  return vec3(meshInfo[meshIndex].worldXform * vertexTable[i0]);",
	"}",

	"vec3 getWV1(uint meshIndex, uint localTriangle) {",
	"  uint i1 = getVIndex(meshIndex, localTriangle, 1);",
	"  return vec3(meshInfo[meshIndex].worldXform * vertexTable[i1]);",
	"}",

	"vec3 getWV2(uint meshIndex, uint localTriangle) {",
	"  uint i2 = getVIndex(meshIndex, localTriangle, 2);",
	"  return vec3(meshInfo[meshIndex].worldXform * vertexTable[i2]);",
	"}",
	"struct Ray2 {",
	"    vec3 origin;",
	"    vec3 dir;",
	"    vec3 invDir;",
	"};",
	"const float FLT_EPS =  1.192092896e-07F;",
	"const float FLT_MAX = 3.4e+38F;",
	"Ray2 createRay(vec3 origin, vec3 dir) {",
	"    vec3 invDir = vec3(",
	"        (abs(dir.x) < FLT_EPS) ? FLT_MAX : 1.0 / dir.x,",
	"        (abs(dir.y) < FLT_EPS) ? FLT_MAX : 1.0 / dir.y,",
	"        (abs(dir.z) < FLT_EPS) ? FLT_MAX : 1.0 / dir.z",
	"    );",
	"    return Ray2(origin, dir, invDir);",
	"}",
	"bool rayIntersectAABB(Ray2 ray, BoundingBoxGLM box, out float tMin, out float tMax) {",
	"    vec3 t0 = (box.min - ray.origin) * ray.invDir;",
	"    vec3 t1 = (box.max - ray.origin) * ray.invDir;",
	"    vec3 tmin = min(t0, t1);",
	"    vec3 tmax = max(t0, t1);",
	"    tMin = max(tmin.x, max(tmin.y, tmin.z));",
	"    tMax = min(tmax.x, min(tmax.y, tmax.z));",
	"    return tMax >= max(0, tMin);",
	"}",
	"bool rayIntersectTriangle(Ray2 ray, vec3 v0, vec3 v1, vec3 v2, out float t, bool onlyFrontFace, out vec3 n) {",
	"    vec3 v0v1 = v1 - v0;",
	"    vec3 v0v2 = v2 - v0;",
	"    vec3 pvec = cross(ray.dir, v0v2);",
	"    float det = dot(v0v1, pvec);",
	"    if (abs(det) < FLT_EPS) return false;",
	"    if (onlyFrontFace && det < 0) return false;",
	"    float invDet = 1.0 / det;",
	"    vec3 tvec = ray.origin - v0;",
	"    float u = dot(tvec, pvec) * invDet;",
	"    if (u < 0.0 || u > 1.0) return false;",
	"    vec3 qvec = cross(tvec, v0v1);",
	"    float v = dot(ray.dir, qvec) * invDet;",
	"    if (v < 0.0 || u + v > 1.0) return false;",
	"    t = dot(v0v2, qvec) * invDet;",
	"	 n = normalize(cross(v0v1, v0v2));",
	"    return t >= 0.0;",
	"}",
	"struct HitRecord {",
	"    vec3 point;",
	"    float t;",
	"    uint triangleIndex;",
	"    vec3 n;",
	"    uint meshIndex;",
	"};",

	"bool trace(vec3 origin, vec3 dir, out HitRecord hitRecord) {",
	"    if (nodeTable.length() == 0) return false;",
	"    Ray2 ray = createRay(origin, dir);",
	"    hitRecord.t = 1e8;",
	"    hitRecord.triangleIndex = 0xFFFFFFFF;",
	"    hitRecord.n = vec3(0,0,0);",
	"    hitRecord.meshIndex = 0xFFFFFFFF;",
	"    float tMin, tMax;"
	"    uint stack[64];",
	"    uint stackPtr = 0;",
	"    stack[stackPtr++] = 0;",
	"    while (stackPtr > 0) {",
	"        uint nodeIndex = stack[--stackPtr];",
	"        if (!rayIntersectAABB(ray, nodeTable[nodeIndex].boundingBox, tMin, tMax) || tMax < 0 || tMin > hitRecord.t)",
	"            continue;",
	"        uint node_leaf_info = nodeTable[nodeIndex].leaf_info;",
	"        if ((node_leaf_info & 0xC0000000) == 0xC0000000) {",
	"            uint meshIndex = node_leaf_info & 0x3FFFFFF;",
	"			 if ((meshInfo[meshIndex].flags & ACTIVE) == 0)",
	"				return false;",
	"            bool onlyFrontFaces = bool(meshInfo[meshIndex].flags & INTERSECT_ONLY_FRONT_FACES_BIT);",
	"            for (uint i = nodeTable[nodeIndex].leftOrFirst; i <= nodeTable[nodeIndex].rightOrLast; ++i) {",
	"                float t;",
	"                vec3 n;",
	"                if (rayIntersectTriangle(ray, getWV0(meshIndex, i), getWV1(meshIndex, i), getWV2(meshIndex, i), t, onlyFrontFaces, n) && t < hitRecord.t) {",
	"                    hitRecord.t = t;",
	"                    hitRecord.n = n;",
	"                    hitRecord.point = ray.origin + ray.dir * t;",
	"                    hitRecord.triangleIndex = i;",
	"                    hitRecord.meshIndex = meshIndex;",
	"                }",
	"            }",
	"        } else {",
	"            bool sign = ray.invDir[(node_leaf_info & 0xC0000000) >> 30] < 0;",
	"            uint firstChild = sign ? nodeTable[nodeIndex].rightOrLast : nodeTable[nodeIndex].leftOrFirst;",
	"            uint secondChild = sign ? nodeTable[nodeIndex].leftOrFirst : nodeTable[nodeIndex].rightOrLast;",
	"            stack[stackPtr++] = secondChild;",
	"            stack[stackPtr++] = firstChild;",
	"        }",
	"    }",
	"    return hitRecord.triangleIndex != -1;",
	"}"

	"void main() {",
	"  vec3 lightDir = vec3(0.0f, 0.0f, 1.0f);",
	"  vec3 N = normalize(vec3(viewMatrix * vec4(wN, 0)));"
	"  float diffuseMult = max(dot(N, lightDir), 0.0);",
	"  final_color = diffuseColor * diffuseMult;",

	"  if (!isInsideFrustum(eP, inspProjMatrix)) {",
	"	return;",
	"}",

	"  vec3 ene = normalize(wN);",
	"  vec3 wR = normalize(reflect(vec3(wP) - inspCamPos, ene));",  // Obtain R vector in world space

	"  HitRecord hit;"
	"  if (trace(wP.xyz + ene * 0.001, wR, hit)) {",
	"    if ((meshInfo[hit.meshIndex].flags & (3 << 2)) == LIGHT_SOURCE) {"
	"	   final_color = meshInfo[hit.meshIndex].payload;",
	"    }"
#if 0
	"    else { ",
	"      final_color = vec4(hit.n *0.5 + 0.5, 1);",
	"    }",
	"  } else {",
	"    final_color = vec4(1, 1, 1, 1);",
#endif
	"  }",
	"  final_color.a = 1.0f;"
	"}"
};

bool GLRenderer::prepareProgram()
{
	program = std::make_shared<autis::Program>();


	rtDataManager = std::make_unique<autis::RTDataManager>();
	rtDataManager->connectToProgram(*program);

	program->loadStrings(vshader, fshader);

	if (!program->compile())
		return false;
	mvpLoc = program->getUniformLocation("modelviewprojMatrix");
	normalMatrixLoc = program->getUniformLocation("normalMatrix");
	viewMatrixLoc = program->getUniformLocation("viewMatrix");
	modelMatrixLoc = program->getUniformLocation("modelMatrix");
	
	
	inspViewMatrixLoc = program->getUniformLocation("inspViewMatrix");
	inspProjMatrixLoc = program->getUniformLocation("inspProjMatrix");
	inspCamPosLoc = program->getUniformLocation("inspCamPos");



	return true;
}

bool GLRenderer::prepareNodeTable()
{
	std::vector<autis::BVH> nodes;
	for (const auto& lightMesh : lightModels) {
		autis::MeshesData msd;
		autis::modelToMeshesData(*models[lightMesh.modelIndex], msd);
		for (const auto& md : msd.meshes) {
			auto node = autis::BVHBuilder::buildBVH(md, lightMesh.modelMatrix, autis::BVHFlags::LightSource(true));
			node.payload = glm::vec4(lightMesh.color, 0);
			nodes.push_back(node);
		}
	}
	for (const auto& renderModel : renderModels) {
		autis::MeshesData msd;
		autis::modelToMeshesData(*models[renderModel.modelIndex], msd);
		for (const auto& md : msd.meshes) {
			auto node = autis::BVHBuilder::buildBVH(md, renderModel.modelMatrix, autis::BVHFlags::InspectableGeometry(true));
			nodes.push_back(node);
		}
	}
	for (const auto& renderModel : occludingModels) {
		autis::MeshesData msd;
		autis::modelToMeshesData(*models[renderModel.modelIndex], msd);
		for (const auto& md : msd.meshes) {
			auto node = autis::BVHBuilder::buildBVH(md, renderModel.modelMatrix, autis::BVHFlags::NonInspectableGeometry(true));
			nodes.push_back(node);
		}
	}

	auto scene = autis::BVHBuilder::buildMultiHierarchy(nodes);
	rtDataManager->update(scene);

	recomputeBVH = false;
	return true;
}
