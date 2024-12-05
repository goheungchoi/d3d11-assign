#include "D3DEngine/Renderer/D3D11Utils.h"

class Mesh {
 public:
	static std::shared_ptr<Mesh> fromFile(const std::string& filename);

	const std::vector<Vertex>& vertices() const { return _vertices; }
	const std::vector<Face>& faces() const { return _faces; }

private:
	Mesh(const struct aiMesh* mesh);

	std::vector<Vertex> _vertices;
	std::vector<Face> _faces;
};
