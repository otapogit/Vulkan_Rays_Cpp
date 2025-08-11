#include "Renderer.h"


#include "tiny_obj_loader.h"

std::vector<glm::vec3> computeNormals(const std::vector<glm::vec3>& pos, const std::vector<uint32_t>& inds) {
	const auto nbVertices = pos.size();
	std::vector<glm::vec3> normals;
	normals.resize(nbVertices, glm::vec3{ 0.0f });

	// Loop over all surface elements in the mesh
	for (size_t i = 0; i < inds.size(); i += 3) {
		// Get the indices of the vertices for this surface element
		// Get the positions of the vertices
		const auto& p1 = pos[inds[i]];
		const auto& p2 = pos[inds[i + 1]];
		const auto& p3 = pos[inds[i + 2]];

		// Calculate the normal for this triangle
		auto normal = glm::normalize(glm::cross(p2 - p1, p3 - p1));
		// Add the normal to the normal for each vertex
		normals[inds[i]] += normal;
		normals[inds[i + 1]] += normal;
		normals[inds[i + 2]] += normal;
	}
	// Normalize the normals
	for (auto& normal : normals) {
		normal = glm::normalize(normal);
	}
	return normals;
}

std::vector<Renderer::MeshId> Renderer::defineMesh(const std::string& objfilename)
{
	tinyobj::ObjReaderConfig reader_config;
	reader_config.mtl_search_path = "./"; // Path to material files

	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(objfilename, reader_config)) {
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


	std::vector<Renderer::MeshId> result;

	auto cmp = [](tinyobj::index_t const& left, tinyobj::index_t const& right) -> bool
		{
			return
				(left.vertex_index < right.vertex_index) ||
				(left.vertex_index == right.vertex_index && left.normal_index < right.normal_index) ||
				(left.vertex_index == right.vertex_index && left.normal_index == right.normal_index && left.texcoord_index < right.texcoord_index);
		};


	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++) {
		std::vector<glm::vec3> vtcs;
		std::vector<glm::vec3> nrmls;
		std::vector<glm::vec2> uv;
		std::vector<uint32_t> inds;

		std::map<tinyobj::index_t, uint32_t, decltype(cmp)> indexXltr(cmp);

		// Loop over faces(polygon)
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			if (shapes[s].mesh.num_face_vertices[f] != 3) {
				std::cerr << "Sólo se soportan triángulos\n";
				exit(2);
			}
			size_t fv = 3;


			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {

				const auto tinyIndex = shapes[s].mesh.indices[index_offset + v];

				auto theIndexIt = indexXltr.find(tinyIndex);
				if (theIndexIt != indexXltr.end()) {
					// sí que estaba: repetir
					inds.push_back(theIndexIt->second);
				}
				else {
					// no estaba: añadir
					auto newIndex = vtcs.size();
					inds.push_back(newIndex);
					indexXltr[tinyIndex] = newIndex;
					vtcs.push_back({
						attrib.vertices[3 * size_t(tinyIndex.vertex_index) + 0],
						attrib.vertices[3 * size_t(tinyIndex.vertex_index) + 1],
						attrib.vertices[3 * size_t(tinyIndex.vertex_index) + 2] });
					// Check if `normal_index` is zero or positive. negative = no normal data
					if (tinyIndex.normal_index >= 0) {
						nrmls.push_back(
							{
								attrib.normals[3 * size_t(tinyIndex.normal_index) + 0],
								attrib.normals[3 * size_t(tinyIndex.normal_index) + 1],
								attrib.normals[3 * size_t(tinyIndex.normal_index) + 2]
							});
					}

					// Check if `texcoord_index` is zero or positive. negative = no texcoord data
					if (tinyIndex.texcoord_index >= 0) {
						uv.push_back(
							{
								attrib.texcoords[2 * size_t(tinyIndex.texcoord_index) + 0],
								attrib.texcoords[2 * size_t(tinyIndex.texcoord_index) + 1]
							});
					}
					else {
						uv.push_back({ 0.0f, 0.0f });
					}
				}
			}
			index_offset += fv;
		}

		if (nrmls.size() != vtcs.size()) {
			nrmls = computeNormals(vtcs, inds);
		}
		result.push_back(defineMesh(vtcs, nrmls, uv, inds));
	}

	return result;
}
