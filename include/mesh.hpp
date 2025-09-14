
/**
 * @brief Represents a 3D mesh with geometry and half-edge structure.
 * 
 * The Mesh class stores vertex positions, face indices, and half-edge data structures.
 * It provides methods for loading geometry from OBJ files and building the half-edge mesh.
 */
class Mesh {
public:
    /**
     * @brief List of 3D points (vertices) in the mesh.
     */
    std::vector<Point> points;

    /**
     * @brief List of faces, each as a vector of vertex indices.
     */
    std::vector<std::vector<int>> faces;

    /**
     * @brief Half-edge mesh vertices.
     */
    std::vector<Vertex> verticesHE;

    /**
     * @brief Half-edge mesh half-edges.
     */
    std::vector<HalfEdge> halfedgesHE;

    /**
     * @brief Half-edge mesh faces.
     */
    std::vector<Face> facesHE;

    /**
     * @brief Loads mesh geometry from an OBJ file.
     * @param filename Path to the OBJ file.
     * @return True if loading was successful, false otherwise.
     */
    bool loadFromOBJ(const std::string& filename);

    /**
     * @brief Builds the half-edge mesh structure from points and faces.
     */
    void buildHalfEdge();
};
