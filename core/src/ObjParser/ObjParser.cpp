#include "core/ObjParser/ObjParser.h"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Setting up custom std::cout of the triangle
std::ostream& operator<<(std::ostream& os, const Triangle& triangle)
{
    os << "(" 
        << "[(" << triangle.v1.x << ", " << triangle.v1.y << ", " << triangle.v1.z << "), "
        << "(" << triangle.v2.x << ", " << triangle.v2.y << ", " << triangle.v2.z << "), "
        << "(" << triangle.v3.x << ", " << triangle.v3.y << ", " << triangle.v3.z << ")] "
        ", (" 
        << triangle.centroid.x << ", " << triangle.centroid.y << ", " << triangle.centroid.z << 
        ")),";
    return os;
}


void loadMesh(std::string filePath, std::vector<Triangle>& mesh, unsigned int& numTriangles)
{

    numTriangles = 0;
    
    const aiScene* scene = aiImportFile(filePath.c_str(), aiProcessPreset_TargetRealtime_MaxQuality);

    if (!scene) {
		std::cerr << "Error loading the model: " << filePath << std::endl;
		return;
	}

    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        const aiMesh* currentMesh = scene->mMeshes[i];
        for (unsigned int i = 0; i < currentMesh->mNumFaces; ++i) {
            const aiFace& face = currentMesh->mFaces[i];

            // Ensure the face is a triangle 
            if (face.mNumIndices != 3) {
                continue; // Skip non-triangle faces
            }

            Triangle triangle;
            triangle.v1 = glm::vec3(currentMesh->mVertices[face.mIndices[0]].x,
                                    currentMesh->mVertices[face.mIndices[0]].y,
                                    currentMesh->mVertices[face.mIndices[0]].z);

            triangle.v2 = glm::vec3(currentMesh->mVertices[face.mIndices[1]].x,
                					currentMesh->mVertices[face.mIndices[1]].y,
                					currentMesh->mVertices[face.mIndices[1]].z);

            triangle.v3 = glm::vec3(currentMesh->mVertices[face.mIndices[2]].x,
                					currentMesh->mVertices[face.mIndices[2]].y,
                					currentMesh->mVertices[face.mIndices[2]].z);

            triangle.NA = glm::vec3(currentMesh->mNormals[face.mIndices[0]].x,
                                    currentMesh->mNormals[face.mIndices[0]].y,
                                    currentMesh->mNormals[face.mIndices[0]].z);

            triangle.NB = glm::vec3(currentMesh->mNormals[face.mIndices[1]].x,
                					currentMesh->mNormals[face.mIndices[1]].y,
                					currentMesh->mNormals[face.mIndices[1]].z);

            triangle.NC = glm::vec3(currentMesh->mNormals[face.mIndices[2]].x,
                					currentMesh->mNormals[face.mIndices[2]].y,
                					currentMesh->mNormals[face.mIndices[2]].z);

            // Calculate centroid 
            triangle.centroid = (triangle.v1 + triangle.v2 + triangle.v3) / 3.0f;


            // Set the material properties
            //triangle.material.color = glm::vec3(144.0f/255.0f, 50.0f/255.0f, 220.0f/255.0f); // purple
            //triangle.material.color = glm::vec3(0.1f, 0.1f, 0.1f); // black
            triangle.material.color = glm::vec3(0.7f, 0.7f, 0.7f); // white
            triangle.material.emissionColor = glm::vec3(0.0f, 0.0f, 0.0f);
            triangle.material.emissionStrength = 0.0f;

            mesh.push_back(triangle);
            numTriangles++;
        }
    }
    std::cout << numTriangles << " triangles loaded" << std::endl;
}

// Constructor for the BVH Node
BVH::Node::Node(glm::vec3 minVec, glm::vec3 maxVec)
    : minVec(minVec), maxVec(maxVec), child1_idx(-1), child2_idx(-1)
{
    //leaf_primitive_indices = new int[AABB_primitives_limit]; 
    // Initialize the leaf_primitive_indices vector with -1
    // populate the leaf_primitive_indices with -1 
    for (unsigned int i = 0; i < AABB_primitives_limit; i++)
    {
        leaf_primitive_indices[i].data = -1;
    }
}

// Overloading the << operator for the BVH Node
namespace BVH {
    std::ostream& operator<<(std::ostream& os, const Node& node) {
        // AABB min and max vectors - output for python visualisation sc
        os <<
            "{ \"minVec\": (" << node.minVec.x << ", " << node.minVec.y << ", " << node.minVec.z << ")," <<
            " \"maxVec\": (" << node.maxVec.x << ", " << node.maxVec.y << ", " << node.maxVec.z << ") },";
       
        // additional info (child indices and leaf primitive indices) can be commented out
        os << "child1_idx: " << node.child1_idx << " child2_idx: " << node.child2_idx <<
            " leaf nodes: ";
        for (int i = 0; i < AABB_primitives_limit; ++i) {
			os << node.leaf_primitive_indices[i].data << ", ";
		}
        return os;
    }
}

// Function to compute the minimum corner of an AABB given the current minimum corner and a vertex
glm::vec3 BVH::minCorner(const glm::vec3& current_min, const glm::vec3& vertex) {
    return glm::vec3(std::min(current_min.x, vertex.x), std::min(current_min.y, vertex.y), std::min(current_min.z, vertex.z));
}

// Function to compute the maximum corner of an AABB given the current maximum corner and a vertex
glm::vec3 BVH::maxCorner(const glm::vec3& current_max, const glm::vec3& vertex) {
    return glm::vec3(std::max(current_max.x, vertex.x), std::max(current_max.y, vertex.y), std::max(current_max.z, vertex.z));
}

void BVH::computeAABB(const std::vector<unsigned int>& triangle_indices, const std::vector<Triangle>& triangle_mesh, glm::vec3& minVec, glm::vec3& maxVec)
{
    minVec = glm::vec3(std::numeric_limits<float>::infinity());
    maxVec = glm::vec3(-std::numeric_limits<float>::infinity());

    for (const unsigned int& tri_index : triangle_indices)
    {
        const Triangle& triangle = triangle_mesh[tri_index]; // Get the current triangle.

        // Update minVec and maxVec for each vertex of the triangle.
        minVec = minCorner(minVec, triangle.v1);
        maxVec = maxCorner(maxVec, triangle.v1);

        minVec = minCorner(minVec, triangle.v2);
        maxVec = maxCorner(maxVec, triangle.v2);

        minVec = minCorner(minVec, triangle.v3);
        maxVec = maxCorner(maxVec, triangle.v3);
    }
}

BVH::Node BVH::init(const std::vector<unsigned int>& triangle_indices, const std::vector<Triangle> triangle_mesh) {
    glm::vec3 minVec, maxVec;
    BVH::computeAABB(triangle_indices, triangle_mesh, minVec, maxVec);
    BVH::Node root_node = BVH::Node(minVec, maxVec);
    return root_node;
}

BVH::Partition_output BVH::PartitionNode(const BVH::Node parent_node, std::vector<unsigned int>& triangle_indices, const std::vector<Triangle>& triangles, const Heuristic& heuristic) {

    if (heuristic == BVH::Heuristic::SURFACE_AREA_HEURISTIC) {
		return BVH::surface_area_heuristic(parent_node, triangle_indices, triangles, false);
	}
    else if (heuristic == BVH::Heuristic::SURFACE_AREA_HEURISTIC_BUCKETS) {
        return BVH::surface_area_heuristic(parent_node, triangle_indices, triangles, true);
    }

    BVH::Partition_output output;

    float parent_AABB_Width = parent_node.maxVec.x - parent_node.minVec.x;
    float parent_AABB_Height = parent_node.maxVec.y - parent_node.minVec.y;
    float parent_AABB_Depth = parent_node.maxVec.z - parent_node.minVec.z;

    float* sorted_axis_sizes[3]{ &parent_AABB_Width, &parent_AABB_Height, &parent_AABB_Depth };

    // sorting the addresses by the values they point do
    std::sort(sorted_axis_sizes, sorted_axis_sizes + 3, [&](float* a, float* b)
        {
            return *a > *b;
        }
    );

    /*
        Lambda to split the box exactly in the middle
        returns an integer value - depending on the axis it is the given component of the vector
    */
    auto spatial_middle_split = [&](unsigned int axis) {
        switch (axis) {
        case 0:
            return parent_node.minVec.x + parent_AABB_Width / 2;
        case 1:
            return parent_node.minVec.y + parent_AABB_Height / 2;
        case 2:
            return parent_node.minVec.z + parent_AABB_Depth / 2;
        default:
            std::cout << "BVH spatial_middle_split() - axis can only be 0, 1, 2" << std::endl;
        }
    };

    /*
        Lambda to split the box based on the median
        calculates the median x or y or z based on the axis of all the centroids in the box
        returns the median, integer value - depending on the axis it is the given component of the vector
    */
    auto object_median_split = [&](unsigned int axis) {
        std::vector<float> centroid_axis_components;
        for (const unsigned int& triangle_index : triangle_indices)
        {
            centroid_axis_components.push_back(triangles[triangle_index].centroid[axis]);
        }

        std::sort(centroid_axis_components.begin(), centroid_axis_components.end());

        float median;
        unsigned int length = centroid_axis_components.size();
        if (centroid_axis_components.size() % 2 == 0) { // if even
            median = (centroid_axis_components[length / 2 - 1] + centroid_axis_components[length / 2]) / 2;
        }
        else { // if odd
            median = centroid_axis_components[length / 2];
        }
        return median;
    };

    /*
        Lambda to split the triangles into 2 std::vectors it chooses where to
        put them based on the division axis defined before
    */
    auto partition_triangles = [&](unsigned int division_axis, float axis_value)
        // Note - Division axis can be 0, 1 or 2 corresponding to the x, y, z vector components
    {
        for (unsigned int triangle_index : triangle_indices)
        {
            if (triangles[triangle_index].centroid[division_axis] < axis_value) {
                output.LTris.push_back(triangle_index);
            }
            else {
                output.RTris.push_back(triangle_index);
            }
        }
    };

    /*
        We assume that the a single axis might not be able to split the triangles
        We schedule the split by all 3 axes starting with the longest
        Once the triangles have been divided we check if any side is empty
        in which case we need to continue and split with a different axis.
        Once we do get a successful split meaning no side is empty
        we break out of the loop
    */
    unsigned int axis;
    for (float* current_axis_ptr : sorted_axis_sizes)
    {
        if (current_axis_ptr == &parent_AABB_Width) {
            axis = 0;
        }
        else if (current_axis_ptr == &parent_AABB_Height) {
            axis = 1;
        }
        else {
            axis = 2;
        }

        if (heuristic == BVH::Heuristic::OBJECT_MEDIAN_SPLIT) {
            partition_triangles(axis, object_median_split(axis));
        }
        else if (heuristic == BVH::Heuristic::SPATIAL_MIDDLE_SPLIT) {
            partition_triangles(axis, spatial_middle_split(axis));
        }

        if (!(output.LTris.empty() || output.RTris.empty())) {
            break;
        }
        output = BVH::Partition_output(); // reset the output
    }

    /*
        If we are very unlucky, not a single axis was able to split the triangles into 2 sides
        in this case we simply put half of them in one list and the second half to the other list
        - at this point we dont really care, that they might overlap because since this will very
            rarely happen and the bigger priority is to have a specific number of primitives in AABB
    */
    if ((output.LTris.empty() && output.RTris.size() > AABB_primitives_limit) || (output.RTris.empty() && output.LTris.size() > AABB_primitives_limit)) {
        output = BVH::Partition_output();
        for (unsigned int i = 0; i < triangle_indices.size() / 2; i++)
        {
            output.LTris.push_back(triangle_indices[i]);
        }
        for (unsigned int i = triangle_indices.size() / 2 + 1; i < triangle_indices.size(); i++)
        {
            output.RTris.push_back(triangle_indices[i]);
        }
    }

    /*
        Making the AABBs for the two sides
        based on the number on each side we set the isLeaf bool
    */
    BVH::computeAABB(output.LTris, triangles, output.LAABBmin, output.LAABBmax);
    if (output.LTris.size() <= AABB_primitives_limit) {
        output.LIsLeaf = true;
    }

    BVH::computeAABB(output.RTris, triangles, output.RAABBmin, output.RAABBmax);
    if (output.RTris.size() <= AABB_primitives_limit) {
        output.RIsLeaf = true;
    }

    return output;
}


BVH::Partition_output BVH::surface_area_heuristic(const BVH::Node parent_node, std::vector<unsigned int>& triangle_indices, const std::vector<Triangle>& triangles, const bool& split_buckets) {
    BVH::Partition_output output;

    float best_SAH_cost = std::numeric_limits<float>::infinity();
    unsigned int best_axis;
    unsigned int best_idx;

    float parent_surface_area = 2 * ((parent_node.maxVec.x - parent_node.minVec.x) * (parent_node.maxVec.y - parent_node.minVec.y) +
        		                     (parent_node.maxVec.x - parent_node.minVec.x) * (parent_node.maxVec.z - parent_node.minVec.z) +
        		                     (parent_node.maxVec.y - parent_node.minVec.y) * (parent_node.maxVec.z - parent_node.minVec.z));


    for (size_t axis = 0; axis < 3; axis++)
    {
        // sort the triangles based on the centroid of the triangles
        std::sort(triangle_indices.begin(), triangle_indices.end(), [&](unsigned int a, unsigned int b) 
        {
			return triangles[a].centroid[axis] < triangles[b].centroid[axis];
		});

        // iterate over each split possible for the current axis and calculate the SAH cost
        // always having at least one triangle on each side (starting from idx 1, ending one before the last)
        unsigned int iterator = 1;
        if (split_buckets && size(triangle_indices) > 16) {
            const unsigned int num_buckets = 16;
            iterator = size(triangle_indices) / num_buckets;
        }

        for (size_t split_idx = 0; split_idx < size(triangle_indices); split_idx += iterator) 
        {
            std::vector<unsigned int> left_triangles(triangle_indices.begin(), triangle_indices.begin() + split_idx);
			std::vector<unsigned int> right_triangles(triangle_indices.begin() + split_idx, triangle_indices.end());

			glm::vec3 left_min, left_max, right_min, right_max;
			BVH::computeAABB(left_triangles, triangles, left_min, left_max);
			BVH::computeAABB(right_triangles, triangles, right_min, right_max);

			float left_surface_area = 2 * ((left_max.x - left_min.x) * (left_max.y - left_min.y) +
                				           (left_max.x - left_min.x) * (left_max.z - left_min.z) +
                				           (left_max.y - left_min.y) * (left_max.z - left_min.z));

			float right_surface_area = 2 * ((right_max.x - right_min.x) * (right_max.y - right_min.y) +
                				            (right_max.x - right_min.x) * (right_max.z - right_min.z) +
                				            (right_max.y - right_min.y) * (right_max.z - right_min.z));
            
            /*
            Surface Area Heuristic (SAH) cost calculation
            
            https://www.pbr-book.org/3ed-2018/Primitives_and_Intersection_Acceleration/Bounding_Volume_Hierarchies#fragment-Computecostsforsplittingaftereachbucket-0

            The assumption made in PBRT (Physically Based Rendering Techniques) is that the cost of intersecting a ray with any primitive 
            (be it a triangle, sphere, etc.) is roughly the same. This is a simplification, 
            but it's one that generally holds true in practice and simplifies the calculation of the Surface Area Heuristic (SAH).
            Given this assumption, the cost of intersecting a ray with a set of primitives is proportional to the number of primitives. 
            Therefore, instead of summing the individual intersection costs for each primitive, we can just count the number of primitives.
            This is why the SAH cost formula uses the counts N_left and N_right of primitives in the left and right child AABBs, respectively. 
            By multiplying these counts by the ratios of the child AABBs' surface areas to the parent AABB's surface area, we get an estimate 
            of the expected number of ray-primitive intersection tests for the primitives in each child AABB.
            */

            float SAH_cost = .125f + (size(left_triangles) * left_surface_area + size(right_triangles) * right_surface_area) / parent_surface_area;

            if (SAH_cost < best_SAH_cost) 
            {
                best_SAH_cost = SAH_cost;
                best_axis = axis;
                best_idx = split_idx;
			}   
        }
    }

    // at this point we know the best split so we build the output
    std::sort(triangle_indices.begin(), triangle_indices.end(), [&](unsigned int a, unsigned int b) {
        return triangles[a].centroid[best_axis] < triangles[b].centroid[best_axis];
        });

    std::vector<unsigned int> left_triangles(triangle_indices.begin(), triangle_indices.begin() + best_idx);
    std::vector<unsigned int> right_triangles(triangle_indices.begin() + best_idx, triangle_indices.end());

    BVH::computeAABB(left_triangles, triangles, output.LAABBmin, output.LAABBmax);
    BVH::computeAABB(right_triangles, triangles, output.RAABBmin, output.RAABBmax);

    if (size(left_triangles) <= BVH::AABB_primitives_limit) {
		output.LIsLeaf = true;
	}

    if (size(right_triangles) <= BVH::AABB_primitives_limit) {
        output.RIsLeaf = true;
    }

    output.LTris = left_triangles;
    output.RTris = right_triangles;

    return output;
}

BVH::BVH_data BVH::construct(std::string path, const Heuristic heuristic) {
    // loading mesh
    std::vector<Triangle> triangles;
    unsigned int num_triangles = 0;
    loadMesh(path, triangles, num_triangles);


    std::vector<unsigned int> triangle_indices;
    for (unsigned int i = 0; i < triangles.size(); i++) {
        triangle_indices.push_back(i);
    }

    BVH::Node root_node = BVH::init(triangle_indices, triangles);
    std::vector<BVH::Node> BVH;
    BVH.push_back(root_node);

    std::queue<unsigned int> queue;
    std::queue<std::vector<unsigned int>> index_queue;

    queue.push(0);
    index_queue.push(triangle_indices);

    while (!queue.empty()) {
        unsigned int current_node_idx = queue.front();
        queue.pop();
        std::vector<unsigned int> current_tri_idxs = index_queue.front();
        index_queue.pop();
        BVH::Partition_output output = PartitionNode(BVH[current_node_idx], current_tri_idxs, triangles, heuristic);
        unsigned int BVH_len = BVH.size();

        Node Lnode(output.LAABBmin, output.LAABBmax);
        if (output.LIsLeaf) {
            for (unsigned int i = 0; i < output.LTris.size(); i++) {
                if (i <= BVH::AABB_primitives_limit - 1) { // protection if somehow there are more cell primitives than 4 in a leaf cell which is unlikely but still
                    Lnode.leaf_primitive_indices[i].data = output.LTris[i];
                }
            }
        }
        else {
            queue.push(BVH_len);
            index_queue.push(output.LTris);
        }
        BVH[current_node_idx].child1_idx = BVH_len; // index if the first child in the BVH_index_array
        BVH.push_back(Lnode);



        Node Rnode(output.RAABBmin, output.RAABBmax);
        if (output.RIsLeaf) {
            for (unsigned int i = 0; i < output.RTris.size(); i++) {
                if (i <= BVH::AABB_primitives_limit - 1) { // protection if somehow there are more cell primitives than 4 in a leaf cell which is unlikely but still
                    Rnode.leaf_primitive_indices[i].data = output.RTris[i];
                }
                else {
                    std::cout << "Was overflow" << std::endl;
                }
            }
        }
        else {
            queue.push(BVH_len + 1);
            index_queue.push(output.RTris);
        }
        BVH[current_node_idx].child2_idx = BVH_len + 1; // index if the first child in the BVH_index_array
        BVH.push_back(Rnode);

    }

    BVH_data bvh_data;

    bvh_data.BVH = BVH;
    bvh_data.BVH_size = BVH.size();
    bvh_data.TRIANGLES = triangles;
    bvh_data.TRIANGLES_size = num_triangles;
    bvh_data.BVH_tree_depth = BVH::getBVHTreeDepth(BVH, BVH[0], 0);
    return  bvh_data;
}

/**
* @brief Get the maximum height of the BVH
* @param BVH - the BVH
* @param current_node - pass the root node
* @param height - pass 0
* 
* recursive function using DFS to get the maximum height of the BVH
* */
unsigned int BVH::getBVHTreeDepth(const std::vector<Node>& BVH, BVH::Node current_node, unsigned int height) 
{
    if (current_node.child1_idx == -1 && current_node.child2_idx == -1) {
        // we found a leaf node so we return the height of the leaf
        return height;
    }
    
    unsigned int left_height = 0;
    unsigned int right_height = 0;

    if (current_node.child1_idx != -1) {
		left_height = getBVHTreeDepth(BVH, BVH[current_node.child1_idx], height+1);
	}
    if (current_node.child2_idx != -1) {
		right_height = getBVHTreeDepth(BVH, BVH[current_node.child2_idx], height+1);
	}

    return std::max(left_height, right_height);

}

