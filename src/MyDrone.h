#pragma once

#include "MyTransforms.h"
#include "MyBuilder.h"
#include "MyFrustum.h"
#include <vsg/all.h>

class MyDrone : public vsg::Inherit<vsg::StateGroup, MyDrone> {
public:
    vsg::ref_ptr<vsg::Node> body;
    vsg::ref_ptr<AbsoluteLookAtTransform> forwardView;
    vsg::ref_ptr<MyFrustum> frustum;
    vsg::ref_ptr<RelativeLookAtTransform> frustumView;
    vsg::ref_ptr<vsg::Perspective> proj;
// public:
    MyDrone(vsg::ref_ptr<MyBuilder> builder, vsg::ref_ptr<vsg::Perspective> proj, double sz = 1.0);
    void setPosition(double x, double y, double alt, double azim);
    void setView(double yaw, double pitch, double roll = 0.0f);
    void setProjection(vsg::ref_ptr<vsg::Perspective> proj);
};
