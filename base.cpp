// Base class implementation file

#include "include/base.h"
// Include Coin3D header for SbVec3f definition
#include <Inventor/SbVec.h>

namespace Base
{

// Constructor implementation
vec_traits<SbVec3f>::vec_traits(const vec_type& vec)
    : v(vec)
{}

// get() method implementation
inline std::tuple<vec_traits<SbVec3f>::float_type, vec_traits<SbVec3f>::float_type, vec_traits<SbVec3f>::float_type>
vec_traits<SbVec3f>::get() const
{
    float_type x, y, z;
    v.getValue(x, y, z);
    return std::make_tuple(x, y, z);
}

} // namespace Base