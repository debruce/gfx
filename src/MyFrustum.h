#pragma once

#include <vsg/all.h>

struct FrustumParams
{
    vsg::mat4 inverseProj;
    vsg::vec4 color;
};

using FrustumParamsValue = vsg::Value<FrustumParams>;

class MyFrustum : public vsg::Inherit<vsg::MatrixTransform, MyFrustum> {
    vsg::ref_ptr<FrustumParamsValue> frustumParams;
public:
    MyFrustum(vsg::ref_ptr<vsg::Perspective> _proj, const std::string& orientation = std::string());
    void update(vsg::ref_ptr<vsg::Perspective> _proj);
};