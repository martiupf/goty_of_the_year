#include "collider.h"
#include <cmath>
#include <algorithm>
#include <vector>

bool testAABBvsAABB(const AABB& a, const AABB& b)
{
    return (a.min.x < b.max.x && a.max.x > b.min.x) &&
        (a.min.y < b.max.y && a.max.y > b.min.y) &&
        (a.min.z <b.max.z && a.max.z > b.min.z);
}