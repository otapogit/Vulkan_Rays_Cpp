#ifndef _MODEL_H
#define _MODEL_H

#include "iModel.h"

namespace autis
{
	struct MeshData;

	class Model : public IModel
	{
	public:
		Model();
		~Model();

		void render(GLContext &ctx) const override;
		void renderEdges(GLContext& ctx) const override;
		void renderEdgeVertices(GLContext& ctx) const override;
		
		BoundingBox getBB() override;
		void recomputeBB() override;
		void addMesh(std::shared_ptr<Mesh> mesh) override;
		uint32_t getNMeshes() const override { return static_cast<uint32_t>( meshes.size()); };
		bool keepsAllDataInCPU() const override;
		void clearMeshes() override;
		Mesh& getMesh(const autis::MeshId& i) const override;
		std::shared_ptr<Mesh> getMeshPtr(size_t i) const override;
		void replaceMesh(size_t i, std::shared_ptr<Mesh> mesh);
		void removeMesh(size_t i);
		
		void accept(std::function<void(const Mesh&)> op) const override;
		void accept(std::function<void(Mesh&)> op) override;

		const std::string to_string() const;

		void transform(const glm::mat4& transform) override;

		std::shared_ptr<IModel> clone() const override;

		/**
		 * Construye y devuelve un modelk a partir de la información de sus mallas
		*/
		static std::shared_ptr<Model> fromMeshesData(const MeshesData& ameshes, bool keepDataInCPU);
		static std::shared_ptr<Model> fromMeshesData(const MeshData& mesh, bool keepDataInCPU);
	protected:
		std::vector<std::shared_ptr<Mesh>> meshes;
		BoundingBox bbox;
	};
};

#endif
