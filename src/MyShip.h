#pragma once

#include "MyBuilder.h"
#include <vsg/all.h>

class MyShip : public vsg::Inherit<vsg::StateGroup, MyShip> {
    vsg::ref_ptr<vsg::Node> body;
    vsg::ref_ptr<vsg::MatrixTransform> bodyTransform;
public:
    MyShip(vsg::ref_ptr<MyBuilder> builder, double sz = 1.0);
    void setPosition(double x, double y, double azim);
};