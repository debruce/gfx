#pragma once

#include "MyBuilder.h"
#include <vsg/all.h>

class MyDrone : public vsg::Inherit<vsg::StateGroup, MyDrone> {
    vsg::ref_ptr<vsg::Node> body;
    vsg::ref_ptr<vsg::MatrixTransform> bodyTransform;
    vsg::ref_ptr<vsg::Node> frustum;
    vsg::ref_ptr<vsg::MatrixTransform> frustumTransform;
    vsg::dmat4 viewMat;
public:
    MyDrone(vsg::ref_ptr<MyBuilder> builder);
    void setPosition(double x, double y, double alt, double azim);
    void setView(float yaw, float pitch, float roll = 0.0f);
};


class MyShip : public vsg::Inherit<vsg::StateGroup, MyShip> {
    vsg::ref_ptr<vsg::Node> body;
    vsg::ref_ptr<vsg::MatrixTransform> bodyTransform;
public:
    MyShip(vsg::ref_ptr<MyBuilder> builder);
    void setPosition(double x, double y, double azim);
};