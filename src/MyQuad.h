#pragma once

#include "MyFrustum.h"

class MyQuad : public vsg::Inherit<vsg::StateGroup, MyQuad> {
    vsg::ref_ptr<vsg::ShaderSet> _phongShaderSet;
    vsg::ref_ptr<vsg::StateGroup> stateGroup;
    vsg::ref_ptr<vsg::vec3Array> vertices;
    vsg::ref_ptr<vsg::ubvec4Array2D> image;
public:
    MyQuad();
    void update(vsg::ref_ptr<MyFrustum> frustum);
};