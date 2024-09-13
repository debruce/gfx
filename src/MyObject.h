#pragma once

#include <vsg/all.h>

class MyObject : public vsg::Inherit<vsg::StateGroup, MyObject> {
    vsg::ref_ptr<vsg::ShaderSet> _phongShaderSet;
    vsg::ref_ptr<vsg::VertexIndexDraw> vid;
    vsg::ref_ptr<vsg::StateGroup> stateGroup;
    static vsg::ref_ptr<vsg::VertexIndexDraw> generate(vsg::ref_ptr<vsg::vec3Array2D> mesh);
public:
    MyObject(vsg::ref_ptr<const vsg::Options> options, vsg::ref_ptr<vsg::vec3Array2D> mesh);
    void update(vsg::ref_ptr<vsg::vec3Array2D> mesh);
};