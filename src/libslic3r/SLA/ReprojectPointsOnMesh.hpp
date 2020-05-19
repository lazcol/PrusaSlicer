#ifndef REPROJECTPOINTSONMESH_HPP
#define REPROJECTPOINTSONMESH_HPP

#include "libslic3r/Point.hpp"
#include "SupportPoint.hpp"
#include "Hollowing.hpp"
#include "EigenMesh3D.hpp"
#include "libslic3r/Model.hpp"

#include <tbb/parallel_for.h>

namespace Slic3r { namespace sla {

template<class Pt> Vec3f pos(const Pt &p) { return p.pos; }
template<class Pt> void pos(Pt &p, const Vec3f &pp) { p.pos = pp; }

template<class PointType>
void reproject_support_points(const EigenMesh3D &mesh, std::vector<PointType> &pts)
{
    tbb::parallel_for(size_t(0), pts.size(), [&mesh, &pts](size_t idx) {
        int junk;
        Vec3f new_pos;
        mesh.squared_distance(pos(pts[idx]), junk, new_pos);
        pos(pts[idx], new_pos);
    });
}

inline void reproject_points_and_holes(ModelObject *object)
{
    bool has_sppoints = !object->sla_support_points.empty();
    bool has_holes   = !object->sla_drain_holes.empty();

    if (!object || (!has_holes && !has_sppoints)) return;

    EigenMesh3D emesh{object->raw_mesh()};

    if (has_sppoints)
        reproject_support_points(emesh, object->sla_support_points);

    if (has_holes)
        reproject_support_points(emesh, object->sla_drain_holes);
}

}}
#endif // REPROJECTPOINTSONMESH_HPP