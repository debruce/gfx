#pragma once

#include "MyBuilder.h"
#include <vsg/all.h>

class MyDrone : public vsg::Inherit<vsg::StateGroup, MyDrone> {
    vsg::ref_ptr<vsg::Node> body;
    vsg::ref_ptr<vsg::MatrixTransform> bodyTransform;
    vsg::ref_ptr<vsg::Node> frustum;
    vsg::ref_ptr<vsg::MatrixTransform> frustumTransform;
    vsg::dmat4 viewMat;
    double altitude;
public:
    MyDrone(vsg::ref_ptr<MyBuilder> builder, double sz = 1.0);
    void setPosition(double x, double y, double alt, double azim);
    void setView(double yaw, double pitch, double roll = 0.0f);
    vsg::dvec3 getIntercept(const vsg::dvec3& frustumPt = vsg::dvec3{0.0, 0.0, 1.0});
};


class MyShip : public vsg::Inherit<vsg::StateGroup, MyShip> {
    vsg::ref_ptr<vsg::Node> body;
    vsg::ref_ptr<vsg::MatrixTransform> bodyTransform;
public:
    MyShip(vsg::ref_ptr<MyBuilder> builder, double sz = 1.0);
    void setPosition(double x, double y, double azim);
};