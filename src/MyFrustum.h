#pragma once

#include <vsg/all.h>

class MyFrustum : public vsg::Inherit<vsg::MatrixTransform, MyFrustum> {
public:
    vsg::ref_ptr<vsg::Perspective> proj;
// public:
    MyFrustum(vsg::ref_ptr<vsg::Perspective> proj, const std::string& orientation = std::string());
    // void update(vsg::ref_ptr<vsg::Perspective> proj);
};