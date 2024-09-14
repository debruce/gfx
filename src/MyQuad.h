#pragma once

#include <vsg/all.h>

class MyQuad : public vsg::Inherit<vsg::StateGroup, MyQuad> {
    vsg::ref_ptr<vsg::ShaderSet> _phongShaderSet;
    vsg::ref_ptr<vsg::StateGroup> stateGroup;
    vsg::ref_ptr<vsg::vec3Array> vertices;
    vsg::ref_ptr<vsg::ubvec4Array2D> image;
public:
    MyQuad(vsg::ref_ptr<const vsg::Options> options, const vsg::vec3& a, const vsg::vec3& b, const vsg::vec3& c, const vsg::vec3& d);
    void update(const vsg::dvec3& a, const vsg::dvec3& b, const vsg::dvec3& c, const vsg::dvec3& d);
};