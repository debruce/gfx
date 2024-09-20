#pragma once

#include <vsg/all.h>

class MyFrustum : public vsg::Inherit<vsg::StateGroup, MyFrustum> {
    vsg::ref_ptr<vsg::Perspective> proj;
public:
    MyFrustum(vsg::ref_ptr<vsg::Perspective> proj, const double& sz = 1.0);
    // void update(vsg::ref_ptr<vsg::Perspective> proj);
};