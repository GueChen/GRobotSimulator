#include "geometry/bounding_volume_hierarchy.h"

namespace GComponent{

BVHNode BVHNode::CreateLeaf(int first, int n, const BoundingBox& b)
{
    BVHNode node;
    node.first_offset_ = first;
    node.count_        = n;
    node.bound_          = b;
    node.children_[0]  = node.children_[1] = nullptr;
    return node;
}

void BVHNode::InsertChildren(int axis, BVHNode* child0, BVHNode* child1)
{
    children_[0] = child0;
    children_[1] = child1;
    bound_       = BoundingBox::MergeTwoBoundingBox(child0->bound_, child1->bound_);
    axis_        = axis;
    count_       = 0;
}

}
