#include "../include/utils.hpp"
#include <unordered_map>
#include "../include/half_edge.hpp"
#include <vector>
#include <iostream>
#include <algorithm>


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
        // Store indices of half-edges for this face
        std::vector<int> hedge_indices;
        for (int i = 0; i < n; ++i) {
            int curr = inds[i];
            int next = inds[(i+1)%n];
            HalfEdge* he = &halfedges[hedge_idx];
            he->origin = &vertices[curr];
            he->face = &faces[f];
            hedge_indices.push_back(hedge_idx);
            edge_map[{curr, next}] = he;
            hedge_idx++;
        }
        // Assign next pointers for this face
        for (int i = 0; i < n; ++i) {
            halfedges[hedge_indices[i]].next = &halfedges[hedge_indices[(i+1)%n]];
        }
        // Assign one edge to the face
        faces[f].edge = &halfedges[hedge_indices[0]];
    }

    // Set twin pointers for each half-edge
    for (const auto& kv : edge_map) {
        auto key = kv.first;
        auto it = edge_map.find({key.second, key.first});
        if (it != edge_map.end()) {
            kv.second->twin = it->second;
        }
    }

    // Assign one outgoing edge to each vertex (first found)
    for (auto& he : halfedges) {
        if (!he.origin->edge) {
            he.origin->edge = &he;
        }
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


// *** FIXED ROBUST VERSION ***
// Traverses all faces around a vertex, even if there are boundaries.
std::vector<Face*> getFacesOfVertex(Vertex* v) {
    std::vector<Face*> faces;
    if (!v || !v->edge) return faces;

    HalfEdge* start_he = v->edge;
    
    // --- Forward (CW) traversal ---
    HalfEdge* current_he = start_he;
    do {
        if (current_he->face) {
            faces.push_back(current_he->face);
        }
        current_he = current_he->twin ? current_he->twin->next : nullptr;
    } while (current_he != nullptr && current_he != start_he);

    // --- Backward (CCW) traversal for boundaries ---
    if (current_he == nullptr) { // We hit a boundary, so traverse the other way
        // To go "backward", find the previous edge in the face loop and take its twin.
        HalfEdge* pred = start_he;
        // Start the backward traversal from the start edge's predecessor
        // Use a safe for loop to prevent infinite loops on malformed geometry
        for (int i = 0; i < 256; ++i) { 
            if (pred->next == start_he) break;
            pred = pred->next;
            if (i == 255) return faces; // Safety break
        }
        current_he = pred->twin;

        while (current_he != nullptr) {
            if (current_he->face) {
                faces.push_back(current_he->face);
            }
            pred = current_he;
            bool found_pred = false;
            // Use a safe for loop to find the next predecessor
            for(int i=0; i < 256; ++i) {
                if (pred->next == current_he) {
                    found_pred = true;
                    break;
                }
                pred = pred->next;
            }
            // If the loop structure is broken, stop to prevent a crash
            if (!found_pred) break;
            current_he = pred->twin;
        }
    }
    return faces;
}


// *** FIXED ROBUST VERSION ***
// Traverses all outgoing edges from a vertex, even if there are boundaries.
std::vector<HalfEdge*> getEdgesOfVertex(Vertex* v) {
    std::vector<HalfEdge*> incident_edges;
    if (!v || !v->edge) return incident_edges;

    // First, get all faces that touch this vertex using the robust function.
    std::vector<Face*> faces = getFacesOfVertex(v);

    // Iterate through all the edges of those faces.
    for (Face* f : faces) {
        if (!f || !f->edge) continue;
        HalfEdge* start_he = f->edge;
        HalfEdge* current_he = start_he;
        do {
            // An edge is incident to v if it starts at v (outgoing)
            // or if it ends at v (incoming). The end of `current_he` is the start of `current_he->next`.
            if (current_he->origin == v || current_he->next->origin == v) {
                incident_edges.push_back(current_he);
            }
            current_he = current_he->next;
        } while (current_he != start_he);
    }

    // The above loop can add the same edge multiple times if it's an interior edge
    // shared by two faces in the list. We must remove duplicates.
    std::sort(incident_edges.begin(), incident_edges.end());
    incident_edges.erase(std::unique(incident_edges.begin(), incident_edges.end()), incident_edges.end());

    return incident_edges;
}
