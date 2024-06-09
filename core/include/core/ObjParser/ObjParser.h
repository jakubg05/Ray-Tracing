// standard
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <queue>

// third-party
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>


#ifndef RTX_MATERIAL
#define RTX_MATERIAL
/**
 * @struct RaytracingMaterial
 * @brief A structure representing the material properties for ray tracing.
 *
 * This structure contains the color, emission strength, and emission color 
 * of a material. It is used in the shading calculations in a ray tracing renderer.
 */
struct RaytracingMaterial
{
    // Thanks to https://learnopengl.com/Advanced-OpenGL/Advanced-GLSL

    glm::vec3 color;              // offset 0   // alignment 16 // size 12 // total 12 bytes
    float emissionStrength;       // offset 12  // alignment 4  // size 4  // total 16 bytes
    glm::vec3 emissionColor;      // offset 16  // alignment 16 // size 12 // total 28 bytes
    float std140padding;          // offset 28  // alignment 4  // size 4  // total 32 bytes
};
#endif

#ifndef TRIANGLE
#define TRIANGLE

/**
 * @struct Triangle
 * @brief A structure representing a triangle in 3D space.
 *
 * This structure contains the vertices and normals of a triangle, 
 * as well as the material properties of the triangle. 
 * It is used in the construction of 3D meshes for ray tracing.
 */
struct Triangle
{
    // vertices
    glm::vec3 v1;       //offset 0   // alignment 16 // size 12 // total 12 bytes
    float padding1;     //offset 12  // alignment 4  // size 4  // total 16 bytes
    glm::vec3 v2;       //offset 16  // alignment 16 // size 12 // total 28 bytes
    float padding2;     //offset 28  // alignment 4  // size 4  // total 32 bytes 
    glm::vec3 v3;       //offset 32  // alignment 16 // size 12 // total 44 bytes
    float padding3;     //offset 44  // alignment 4  // size 4  // total 48 bytes

    // normals
    glm::vec3 NA;       //offset 48  // alignment 16 // size 12 // total 60 bytes
    float padding4;     //offset 60  // alignment 4  // size 4  // total 64 bytes
    glm::vec3 NB;       //offset 64  // alignment 16 // size 12 // total 76 bytes
    float padding5;     //offset 76  // alignment 4  // size 4  // total 80 bytes
    glm::vec3 NC;       //offset 80  // alignment 16 // size 12 // total 92 bytes
    float padding6;     //offset 92  // alignment 4  // size 4  // total 96 bytes

    glm::vec3 centroid;             //offset 96  // alignment 16 // size 12 // total 108 bytes
    float padding7;	                //offset 108 // alignment 4  // size 4  // total 112 bytes

    RaytracingMaterial material;    //offset 112 // alignment 16 // size 32 // total 144 bytes


    // overload the << operator to print the triangle
    friend std::ostream& operator<<(std::ostream& os, const Triangle& triangle);
};
#endif

#ifndef OBJ_PARSER
#define OBJ_PARSER
/** @brief Loads a 3D mesh from an OBJ file.
 *
 * This function reads an OBJ file and extracts the vertex, vertex normal, and face information to construct a mesh of triangles.
 * The mesh and the number of triangles are returned via reference parameters.
 *
 * @param filePath The path to the OBJ file.
 * @param mesh A reference to a vector of Triangles that will be filled with the triangles from the OBJ file.
 * @param numTriangles A reference to an unsigned int that will be set to the number of triangles in the mesh.
 */
void loadMesh(std::string filePath, std::vector<Triangle>& mesh, unsigned int& numTriangles);
#endif

#ifndef BVH_IMPLEMENTATION
#define BVH_IMPLEMENTATION
namespace BVH {

    // The heuristic used to partition the primitives in the BVH
    
#ifndef heuristicEnum
#define heuristicEnum
    enum class Heuristic {
        OBJECT_MEDIAN_SPLIT,
        SPATIAL_MIDDLE_SPLIT,
        SURFACE_AREA_HEURISTIC,
        SURFACE_AREA_HEURISTIC_BUCKETS
    };
#endif

    // The maximum number of primitives (triangles) a leaf node can contain
    const unsigned int AABB_primitives_limit = 2;

    /**
     * @class Node
     * @brief A class representing a node in a Bounding Volume Hierarchy (BVH).
     *
     * Each node in the BVH represents a bounding box in 3D space. It contains the 
     * indices of the primitives (triangles) it contains if it is a leaf node, and 
     * the indices of its child nodes if it is an internal node.
     */

    struct Int128 {
        int data;
        char padding[12];  // Ensure 16-byte size
    };

    class Node {
    public:
        // constructors
        Node() {};
        Node(glm::vec3 minVec, glm::vec3 maxVec);

        // overload the << operator to print the node
        friend std::ostream& operator<<(std::ostream& os, const Node& node);

        /**
         * @brief The indices of the primitives (triangles) contained in the leaf node.
         * Initially populated with -1 to indicate that the node is not a leaf node.
         * Size is determined by AABB_primitives_limit.
         */
        Int128 leaf_primitive_indices[AABB_primitives_limit];

        // AABB (Axis-Aligned Bounding Box) of the node
        glm::vec3 minVec;
        int child1_idx;
        glm::vec3 maxVec;
        int child2_idx;
    };

    /**
     * @struct BVH_data
     * @brief A structure containing the data of a Bounding Volume Hierarchy (BVH).
     *
     * This structure contains a vector of all nodes in the BVH, the size of the BVH, 
     * a pointer to the array of triangles represented by the BVH, and the size of this array.
     */
    struct BVH_data {
        std::vector<BVH::Node> BVH;
        std::vector<Triangle> TRIANGLES;
        
        unsigned int BVH_tree_depth;
        std::vector<glm::vec3> heatmapLayers;

        unsigned int BVH_size;
        unsigned int TRIANGLES_size;
    };

    // Helper functions for calculating the minimum and maximum vectors of an AABB
    glm::vec3 minCorner(const glm::vec3& current_min, const glm::vec3& vertex);
    glm::vec3 maxCorner(const glm::vec3& current_max, const glm::vec3& vertex);

    /**
     * @brief Computes the Axis-Aligned Bounding Box (AABB) for a set of triangles.
     *
     * This function iterates over a set of triangles, represented by their indices, and computes 
     * the minimum and maximum corners of the AABB that encloses them. The minimum and maximum 
     * corners are updated for each vertex of each triangle.
     *
     * @param triangle_indices The indices of the triangles for which to compute the AABB.
     * @param triangle_mesh The mesh containing the triangles.
     * @param minVec A reference to a vector that will be set to the minimum corner of the AABB. (return value)
     * @param maxVec A reference to a vector that will be set to the maximum corner of the AABB. (return value)
     */
    void computeAABB(const std::vector<unsigned int>& triangle_indices, const std::vector<Triangle>& triangle_mesh, glm::vec3& minVec, glm::vec3& maxVec);

    /**
     * @brief Initializes a BVH node with a set of triangles.
     *
     * This function creates the root BVH node and computes its bounding box based on the given set of triangles (mesh). 
     * The triangles are specified by their indices in the triangle mesh.
     *
     * @param triangle_indices A vector of indices specifying which triangles are contained in the new node.
     * @param triangle_mesh The mesh containing all triangles.
     * @return A new BVH node containing the specified triangles and with a bounding box that encloses them.
     */
    BVH::Node init(const std::vector<unsigned int>& triangle_indices, const std::vector<Triangle> triangle_mesh);


    /**
     * @struct Partition_output
     * @brief A structure containing the output of a BVH node partition operation.
     *
     * This structure contains the indices of the triangles in the left and right 
     * child nodes after partitioning, the minimum and maximum corners of the AABBs 
     * of the left and right child nodes, and flags indicating whether the left and 
     * right child nodes are leaf nodes.
     */
    struct Partition_output {
        std::vector<unsigned int> LTris;    ///< Indices of the triangles in the left child node
        glm::vec3 LAABBmin;                 ///< Minimum corner of the AABB of the left child node
        glm::vec3 LAABBmax;                 ///< Maximum corner of the AABB of the left child node
        bool LIsLeaf = false;               ///< Flag indicating whether the left child node is a leaf node

        std::vector<unsigned int> RTris;    ///< Indices of the triangles in the right child node
        glm::vec3 RAABBmin;                 ///< Minimum corner of the AABB of the right child node
        glm::vec3 RAABBmax;                 ///< Maximum corner of the AABB of the right child node
        bool RIsLeaf = false;               ///< Flag indicating whether the right child node is a leaf node
    };

    /**
     * @brief Partitions a BVH node into two child nodes.
     *
     * This function partitions the primitives (triangles) in a BVH node into two sets, each contained in a child node. The partitioning is done based on a heuristic.
     *
     * @param parent_node The BVH node to partition.
     * @param triangle_indices The indices of the triangles contained in the parent node.
     * @param triangles The mesh containing all triangles.
     * @param heuristic The heuristic to use for partitioning.
     * @return A Partition_output structure containing the output of the partition operation.
     */
    BVH::Partition_output PartitionNode(const BVH::Node parent_node, std::vector<unsigned int>& triangle_indices, const std::vector<Triangle>& triangles, const Heuristic& heuristic);

    BVH::Partition_output surface_area_heuristic(const BVH::Node parent_node, std::vector<unsigned int>& triangle_indices, const std::vector<Triangle>& triangles, const bool& split_buckets);

    /**
     * @brief Constructs a Bounding Volume Hierarchy (BVH) from a 3D mesh.
     *
     * This function reads a 3D mesh from a file, constructs a BVH from the mesh, and returns the BVH data.
     *
     * @param path The path to the file containing the 3D mesh.
     * @param heuristic The heuristic to use for partitioning the BVH nodes.
     * @return A BVH_data structure containing the data of the constructed BVH.
     */
    BVH::BVH_data construct(std::string path, const Heuristic heuristic);

    unsigned int getBVHTreeDepth(const std::vector<Node>& BVH, BVH::Node current_node, unsigned int height);
}
#endif

