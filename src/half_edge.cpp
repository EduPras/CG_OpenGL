#include "../include/utils.hpp"
#include <unordered_map>
#include "../include/half_edge.hpp"
#include <vector>


// Hash function for std::pair, used in unordered_map for edge lookup
struct pair_hash {
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1,T2>& p) const {
        return std::hash<T1>()(p.first) ^ std::hash<T2>()(p.second);
    }
};


// Build the half-edge mesh structure from points and face indices
void buildHalfEdgeMeshFromPointsAndFaces(
    const std::vector<Point>& points,
    const std::vector<std::vector<int>>& face_indices,
    std::vector<Vertex>& vertices,
    std::vector<HalfEdge>& halfedges,
    std::vector<Face>& faces) {
    // Copy points to vertices
    vertices.clear();
    for (const auto& p : points) {
        vertices.push_back({p.x, p.y, p.z, nullptr});
    }

    // Count total half-edges needed
    int hedge_count = 0;
    for (const auto& inds : face_indices) hedge_count += inds.size();
    halfedges.resize(hedge_count);
    faces.resize(face_indices.size());

    // Map from (start, end) vertex index to half-edge pointer
    std::unordered_map<std::pair<int,int>, HalfEdge*, pair_hash> edge_map;
    int hedge_idx = 0;
    for (size_t f = 0; f < face_indices.size(); ++f) {
        const auto& inds = face_indices[f];
        int n = inds.size();
        for (int i = 0; i < n; ++i) {
            int curr = inds[i];
            int next = inds[(i+1)%n];
            HalfEdge* he = &halfedges[hedge_idx];
            he->origin = &vertices[curr];
            he->face = &faces[f];
            // Next edge in the face loop
            he->next = &halfedges[hedge_idx + (i+1)%n - i];
            edge_map[{curr, next}] = he;
            hedge_idx++;
        }
        // Assign one edge to the face
        faces[f].edge = &halfedges[hedge_idx-n];
    }

    // Set twin pointers for each half-edge
    for (const auto& kv : edge_map) {
        auto key = kv.first;
        auto it = edge_map.find({key.second, key.first});
        if (it != edge_map.end()) {
            kv.second->twin = it->second;
        }
    }

    // Assign one outgoing edge to each vertex
    for (auto& he : halfedges) {
        he.origin->edge = &he;
    }
}


// Given a face, return all adjacent faces (sharing an edge)
std::vector<Face*> getAdjacentFacesOfFace(Face* f) {
    std::vector<Face*> adj;
    if (!f || !f->edge) return adj;
    HalfEdge* start = f->edge;
    HalfEdge* e = start;
    do {
        // If the twin exists and is not the same face, add it
        if (e->twin && e->twin->face && e->twin->face != f)
            adj.push_back(e->twin->face);
        e = e->next;
    } while (e != start);
    return adj;
}


// Given an edge, return the two faces it borders (if any)
std::vector<Face*> getAdjacentFacesOfEdge(HalfEdge* e) {
    std::vector<Face*> adj;
    if (e && e->face) adj.push_back(e->face);
    if (e && e->twin && e->twin->face && e->twin->face != e->face) adj.push_back(e->twin->face);
    return adj;
}


// Given a vertex, return all faces that include the vertex
std::vector<Face*> getFacesOfVertex(Vertex* v) {
    std::vector<Face*> faces;
    if (!v || !v->edge) return faces;
    HalfEdge* start = v->edge;
    HalfEdge* e = start;
    do {
        if (e->face) faces.push_back(e->face);
        // Move to the next outgoing edge around the vertex
        e = e->twin ? e->twin->next : nullptr;
    } while (e && e != start);
    return faces;
}


// Given a vertex, return all outgoing half-edges from the vertex
std::vector<HalfEdge*> getEdgesOfVertex(Vertex* v) {
    std::vector<HalfEdge*> edges;
    if (!v || !v->edge) return edges;
    HalfEdge* start = v->edge;
    HalfEdge* e = start;
    do {
        edges.push_back(e);
        // Move to the next outgoing edge around the vertex
        e = e->twin ? e->twin->next : nullptr;
    } while (e && e != start);
    return edges;
}
