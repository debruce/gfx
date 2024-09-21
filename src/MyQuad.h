#pragma once

#include <vsg/all.h>

class MyQuad : public vsg::Inherit<vsg::StateGroup, MyQuad> {
    vsg::ref_ptr<vsg::ShaderSet> _phongShaderSet;
    vsg::ref_ptr<vsg::StateGroup> stateGroup;
    vsg::ref_ptr<vsg::vec3Array> vertices;
    vsg::ref_ptr<vsg::ubvec4Array2D> image;
public:
    MyQuad();
    void update(const std::array<vsg::dvec3, 4>& points);
};