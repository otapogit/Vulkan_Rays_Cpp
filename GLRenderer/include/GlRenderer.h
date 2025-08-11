
#include <Renderer.h>


namespace autis {
    class GLContext;
    class Model;
    class FBO;
    class Program;
    class BufferObject;
    class RTDataManager;
};

class GLRenderer : public Renderer {
public:
	GLRenderer();
	~GLRenderer();

	bool init() override;
    MeshId defineMesh(
        const std::vector<glm::vec3>& vtcs,
        const std::vector<glm::vec3>& nrmls,
        const std::vector<glm::vec2>& uv,
        const std::vector<uint32_t> inds);
    
    using Renderer::defineMesh;
    
    bool addMesh(const glm::mat4& modelMatrix, const glm::vec3
        & color, MeshId id, bool inspectable = true) override;
    TextureId addTexture(uint8_t* texels, uint32_t width,
        uint32_t height, uint32_t bpp) override;
    void deleteTexture(TextureId tid) override;;

    virtual bool addLight(const glm::mat4& modelMatrix, MeshId id,
        const glm::vec3& color, LightId lid, TextureId tid = 0) override;

    bool removeMesh(MeshId id) override;

    void clearScene() override;

    void setViewCamera(const glm::mat4& viewMatrix, const
        glm::mat4& projMatrix) override;
    void setInspCamera(const glm::mat4& viewMatrix, const
        glm::mat4& projMatrix) override;

    void setOutputResolution(uint32_t width, uint32_t height) override;

    bool render() override;

    size_t copyResultBytes(uint8_t* buffer, size_t bufferSize) override;

    uint32_t getResultTextureId() override;

    bool saveResultToFile(const std::string& filename) override;

private:
    bool prepareFBO();
    bool prepareProgram();
    bool prepareNodeTable();

    std::unique_ptr<autis::GLContext> ctx;
    std::vector<std::shared_ptr<autis::Model>> models;
    std::shared_ptr<autis::FBO> fbo;
    std::shared_ptr<autis::Program> program;
    std::unique_ptr<autis::RTDataManager> rtDataManager;
    bool recomputeBVH{ true };

    struct RenderModel {
        glm::mat4 modelMatrix;
        glm::vec3 color;
        MeshId modelIndex;
    };
    std::vector<RenderModel> renderModels;
    std::vector<RenderModel> occludingModels;

    struct LightModel {
        glm::mat4 modelMatrix;
        glm::vec3 color;
        MeshId modelIndex;
        LightId id;
        TextureId textureId;
    };
    std::vector<LightModel> lightModels;
    glm::mat4 viewMatrix, projMatrix;
    glm::mat4 inspViewMatrix, inspProjMatrix;
    glm::uvec2 resolution{ 0, 0 };
    int mvpLoc, normalMatrixLoc, viewMatrixLoc, modelMatrixLoc;
    int inspViewMatrixLoc, inspProjMatrixLoc, inspCamPosLoc;
};
