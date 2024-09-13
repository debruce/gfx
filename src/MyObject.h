#pragma once

#include <vsg/all.h>

class MyObject : public vsg::Inherit<vsg::StateGroup, MyObject> {
    // vsg::ref_ptr<vsg::AmbientLight> ambientLight;
    // vsg::ref_ptr<vsg::DirectionalLight> directionalLight;
    // vsg::ref_ptr<vsg::DirectionalLight> directionalLightFixed;
    vsg::ref_ptr<vsg::ShaderSet> _phongShaderSet;
public:
    MyObject(vsg::ref_ptr<const vsg::Options> options, vsg::ref_ptr<vsg::vec3Array2D> mesh);
    // void update(vsg::ref_ptr<vsg::vec3Array2D> mesh);
};