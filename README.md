# CG_OpenGL

A modular C++ OpenGL project for interactive mesh visualization, featuring a robust half-edge mesh data structure, OBJ loading, and real-time viewport controls.


## Summary

- [Running](#running)
- [Half-Edge Mesh: Logic and Math](#half-edge-mesh-logic-and-math)
- [Transformations: Math and Logic](#transformations-math-and-logic)
- [Shader Pipeline: How Rendering Works](#shader-pipeline-how-rendering-works)
- [Module Overview](#module-overview)
- [Interactive Controls](#interactive-controls)
- [Building and Running](#building-and-running)
- [References](#references)

## Running
```sh
ASSET_FILE=bunny.obj docker compose up
```

## Half-Edge Mesh: Logic and Math

A half-edge mesh is a data structure for representing polygonal meshes, enabling efficient traversal and adjacency queries. Each edge is split into two "half-edges" with opposite directions.

### Core Structures

- **Vertex**: Stores position and a pointer to one outgoing half-edge.
- **HalfEdge**: Stores pointers to its origin vertex, twin (opposite) half-edge, next half-edge in the face, and the face it borders.
- **Face**: Stores a pointer to one of its bounding half-edges.

### Construction Logic

Given a list of points and face indices:
1. For each face, create a loop of half-edges, each pointing to its origin vertex and the next half-edge.
2. For each half-edge, find its twin (the half-edge with reversed origin/destination) and set the twin pointer.
3. Assign one outgoing half-edge to each vertex and one bounding half-edge to each face.

### Adjacency Queries

- **Adjacent Faces of a Face**: For each half-edge of the face, if its twin exists, the twin’s face is adjacent.
- **Adjacent Faces of an Edge**: The two faces sharing the edge (via the half-edge and its twin).
- **Faces of a Vertex**: Walk around the vertex using outgoing half-edges, collecting each face.
- **Edges of a Vertex**: Walk around the vertex, collecting each outgoing half-edge.

### Math Under the Hood

- **Edge Representation**: Each edge is represented as a pair of half-edges with opposite directions.
- **Traversal**: To walk around a face, follow the `next` pointers of its half-edges. To walk around a vertex, follow `twin->next` pointers.
- **Twin Lookup**: For each half-edge from vertex A to B, its twin is the half-edge from B to A.

#### Example: Walking Around a Vertex

```cpp
do {
    // e->face is a face containing v
    e = e->twin->next;
} while (e != start);
```
This loop visits all faces sharing vertex `v`.


## Features

- Modular design: mesh, viewer, input, utils, shader, half-edge modules
- OBJ mesh loading and half-edge mesh construction
- Interactive controls: rotate, zoom, pan (with mouse/keyboard)
- Efficient edge and adjacency queries via half-edge structure
- Modern OpenGL rendering pipeline


## Transformations: Math and Logic

The application supports interactive transformations of the mesh in the viewport:

- **Rotation**: The mesh is rotated around the selected axis (X, Y, or Z) by applying a 2D or 3D rotation matrix to each vertex. For example, rotating around Z:

```math
\begin{bmatrix} x' \\ y' \\ z' \end{bmatrix} =
\begin{bmatrix} \cos\theta & -\sin\theta & 0 \\
                \sin\theta &  \cos\theta & 0 \\
                0          &  0          & 1 \end{bmatrix}
\begin{bmatrix} x \\ y \\ z \end{bmatrix}
```

  In code, this is done by updating each point's coordinates using the current rotation angle and axis.

- **Scaling (Zoom)**: The mesh is scaled uniformly by multiplying each coordinate by the zoom factor:

```math
x' = x \cdot s 
y' = y \cdot s 
z' = z \cdot s
```
where $s$ is the zoom level (controlled by mouse scroll).

- **Translation (Pan)**: The mesh is moved in the viewport by adding pan offsets to the $x$ and $y$ coordinates:

```math
x' = x + t_x 
y' = y + t_y
```
where $t_x$ and $t_y$ are the pan offsets (controlled by SHIFT+mouse drag).

All these transformations are applied in sequence to each vertex before rendering.


## Shader Pipeline: How Rendering Works

The project uses a modern OpenGL shader pipeline for rendering:

1. **Vertex Shader**: Each transformed vertex is sent to the GPU, where the vertex shader processes it. The vertex shader typically applies a model-view-projection (MVP) matrix, but in this project, most transformations are done on the CPU for simplicity.

2. **Fragment Shader**: After rasterization, the fragment shader determines the color of each pixel. In this project, the fragment shader is used to set the color of points/lines.

**Pipeline Steps:**

1. CPU applies transformations to mesh vertices (rotation, scaling, translation).
2. Transformed vertices are uploaded to the GPU via a Vertex Buffer Object (VBO).
3. The Vertex Array Object (VAO) describes how vertex data is laid out.
4. The vertex shader receives each vertex and passes it through (optionally applying further transformations).
5. The fragment shader colors each pixel/point.
6. The final image is displayed in the OpenGL window.

**Why use shaders?**

- Shaders provide flexibility and performance for custom rendering effects.
- The pipeline allows for future extension (lighting, materials, etc).


## Module Overview

### mesh
- Loads OBJ files, stores vertex and face data, and builds the half-edge mesh structure.

### input
- Handles all user input (mouse, keyboard, scroll) and updates transformation state (rotation, zoom, pan).

### half_edge
- Implements the half-edge mesh data structure and provides efficient adjacency queries.

### utils
- Utility functions for OBJ parsing, DDA line drawing, and OpenGL setup.

### shader
- Loads, compiles, and manages OpenGL shader programs.


## Interactive Controls

- **Rotate**: Drag mouse
- **Zoom**: Mouse scroll
- **Pan**: Hold SHIFT and drag mouse


## Building and Running

This project uses [GLFW3](https://www.glfw.org/) for window/context management and [GLAD](https://glad.dav1d.de/) for OpenGL function loading.

### Installing GLFW3

On Ubuntu/Debian:
```sh
sudo apt-get install libglfw3-dev
```
On other systems, see the [GLFW download page](https://www.glfw.org/download.html).

GLAD is included as source in the repository and does not require separate installation.

### Build and Run

```sh
mkdir build
cd build
cmake ..
make
./src/LearnOpenGl
```


### Generating Documentation with Doxygen

To generate API documentation from the source code comments:

1. Install Doxygen (if not already installed):
   ```sh
   sudo apt-get install doxygen
   ```
2. Generate a default configuration file (if you don't have one):
   ```sh
   doxygen -g
   ```
3. Edit the `Doxyfile` as needed (set `PROJECT_NAME`, `RECURSIVE = YES`, etc).
4. Run Doxygen to generate the documentation:
   ```sh
   doxygen Doxyfile
   ```
5. Open `html/index.html` in your browser to view the generated docs.


## References

- [Half-Edge Data Structure (Wikipedia)](https://en.wikipedia.org/wiki/Half-edge_data_structure)
- [OpenGL Programming Guide](https://www.opengl.org/documentation/)
- [OBJ File Format](https://en.wikipedia.org/wiki/Wavefront_.obj_file)
- [GLFW](https://www.glfw.org/)
- [GLAD](https://glad.dav1d.de/)

---
