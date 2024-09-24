#pragma once

#include "MyTransforms.h"
#include <vsg/all.h>

struct FrustumParams
{
    vsg::mat4 inverseProj;
    vsg::vec4 color;
};

using FrustumParamsValue = vsg::Value<FrustumParams>;

class MyFrustum : public vsg::Inherit<RelativeLookAtTransform, MyFrustum> {
public:
    vsg::dmat4 projection;
    vsg::dmat4 inverseProjection;
    vsg::ref_ptr<FrustumParamsValue> frustumParams;
    std::array<vsg::dvec3, 4> corners;
// public:
    MyFrustum(vsg::ref_ptr<AbsoluteLookAtTransform> absTransform, vsg::ref_ptr<vsg::Perspective> proj);
    void update(vsg::ref_ptr<vsg::Perspective> proj);
    void update();
};