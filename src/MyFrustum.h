#pragma once

#include <vsg/all.h>

struct FrustumParams
{
    vsg::mat4 inverseProj;
    vsg::vec4 color;
};

using FrustumParamsValue = vsg::Value<FrustumParams>;

class MyFrustum : public vsg::Inherit<vsg::MatrixTransform, MyFrustum> {
public:
    vsg::dmat4 inverseProj;
    vsg::ref_ptr<FrustumParamsValue> frustumParams;
    vsg::dmat4 modelView;
    std::array<vsg::dvec3, 4> corners;
// public:
    MyFrustum(vsg::ref_ptr<vsg::Perspective> proj, const std::string& orientation = std::string());
    void update(vsg::ref_ptr<vsg::Perspective> proj, const vsg::dmat4& mv);
};