// Base class header file
// This file provides vector type conversion utilities

#pragma once
#include <tuple>
#include <gp_Vec.hxx>
#include <gp_Pnt.hxx>

// Forward declarations
class SbVec3f;

namespace Base
{

template<class vecT>
struct vec_traits
{
};

template<>
struct vec_traits<gp_Vec>
{
    using vec_type = gp_Vec;
    using float_type = double;
    explicit vec_traits(const vec_type& vec)
        : v(vec)
    {}
    inline std::tuple<float_type, float_type, float_type> get() const
    {
        return std::make_tuple(v.X(), v.Y(), v.Z());
    }

private:
    const vec_type& v;
};

template<>
struct vec_traits<gp_Pnt>
{
    using vec_type = gp_Pnt;
    using float_type = double;
    explicit vec_traits(const vec_type& vec)
        : v(vec)
    {}
    inline std::tuple<float_type, float_type, float_type> get() const
    {
        return std::make_tuple(v.X(), v.Y(), v.Z());
    }

private:
    const vec_type& v;
};

// Specialization for SbVec3f is implemented in base.cpp
// where SbVec3f is fully defined

template<>
struct vec_traits<SbVec3f>
{
    using vec_type = SbVec3f;
    using float_type = float;
    explicit vec_traits(const vec_type& vec);
    inline std::tuple<float_type, float_type, float_type> get() const;

private:
    const vec_type& v;
};

    // Helper function to create a vector from a tuple (3 elements)
    template<class Vec, typename float_type>
    Vec make_vec(const std::tuple<float_type, float_type, float_type>&& ft)
    {
        return Vec(static_cast<float_type>(std::get<0>(ft)),
                  static_cast<float_type>(std::get<1>(ft)),
                  static_cast<float_type>(std::get<2>(ft)));
    }

    // Helper function to create a vector from a tuple (4 elements)
    template<class Vec, typename float_type>
    Vec make_vec(const std::tuple<float_type, float_type, float_type, float_type>&& ft)
    {
        return Vec(static_cast<float_type>(std::get<0>(ft)),
                  static_cast<float_type>(std::get<1>(ft)),
                  static_cast<float_type>(std::get<2>(ft)),
                  static_cast<float_type>(std::get<3>(ft)));
    }

    // Main conversion function between vector types
    template<class Vec1, class Vec2>
    inline Vec1 convertTo(const Vec2& vec)
    {
        using traits_type = vec_traits<Vec2>;
        using float_type = typename traits_type::float_type;
        traits_type tt(vec);
        auto tuple = tt.get();
        return make_vec<Vec1, float_type>(std::move(tuple));
    }
}