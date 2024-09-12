#pragma once

#include <vsg/all.h>

class VSG_DECLSPEC MyBuilder : public vsg::Inherit<vsg::Builder, MyBuilder>
{
protected:
    vsg::Builder::GeometryMap _frustums;
    vsg::Builder::GeometryMap _lathes;
    vsg::Builder::GeometryMap _meshes;
public:
    MyBuilder() {}
    MyBuilder(const vsg::Builder& rhs) = delete;
    MyBuilder& operator=(const vsg::Builder& rhs) = delete;

    vsg::ref_ptr<vsg::Node> createLathe(vsg::ref_ptr<vsg::vec2Array> curve, const size_t& square_count, const vsg::GeometryInfo& info, const vsg::StateInfo& stateInfo, const double& phase = 0.0);
    vsg::ref_ptr<vsg::Node> createMesh(vsg::ref_ptr<vsg::vec3Array2D> mesh, const vsg::GeometryInfo& info, const vsg::StateInfo& stateInfo);
};
