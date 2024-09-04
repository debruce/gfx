#pragma once

#include <vsg/all.h>

class DynamicLighting : public vsg::Inherit<vsg::Group, DynamicLighting> {
    vsg::ref_ptr<vsg::AmbientLight> ambientLight;
    vsg::ref_ptr<vsg::DirectionalLight> directionalLight;
    vsg::ref_ptr<vsg::DirectionalLight> directionalLightFixed;
public:
    DynamicLighting(vsg::ref_ptr<vsg::Group> scene);

    void setDirectional(float intentsity, const vsg::vec3& dir);
};