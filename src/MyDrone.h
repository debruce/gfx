#pragma once

#include "MyBuilder.h"
#include <vsg/all.h>

class LookAtTransform : public vsg::Inherit<vsg::Transform, LookAtTransform> {
public:
    vsg::ref_ptr<vsg::LookAt> lookAt;

    LookAtTransform() : lookAt(vsg::LookAt::create()) {}

    void set(const vsg::dmat4& matrix)
    {
        lookAt->set(matrix);

    }

    vsg::dmat4 transform(const vsg::dmat4 &mv = vsg::dmat4()) const override
    {
        return mv * lookAt->transform();
    }
};

class RelativeViewTransform : public vsg::Inherit<vsg::Transform, RelativeViewTransform> {
public:
    vsg::RelativeViewMatrix relativeView;

    RelativeViewTransform(vsg::ref_ptr<vsg::ViewMatrix> vm) : relativeView(vsg::dmat4(), vm)
    {
    }

    vsg::dmat4 transform(const vsg::dmat4 &mv = vsg::dmat4()) const override
    {
        return mv * relativeView.viewMatrix->transform() * relativeView.matrix;
    }
};

class MyDrone : public vsg::Inherit<vsg::StateGroup, MyDrone> {
public:
    vsg::ref_ptr<vsg::Node> body;
    vsg::ref_ptr<LookAtTransform> forwardView;
    vsg::ref_ptr<vsg::Node> frustum;
    vsg::ref_ptr<RelativeViewTransform> cameraView;
    vsg::ref_ptr<vsg::Perspective> proj;
// public:
    MyDrone(vsg::ref_ptr<MyBuilder> builder, double sz = 1.0);
    void setPosition(double x, double y, double alt, double azim);
    void setView(double yaw, double pitch, double roll = 0.0f);
    std::array<vsg::dvec3, 4> getGroundCorners();
};


class MyShip : public vsg::Inherit<vsg::StateGroup, MyShip> {
    vsg::ref_ptr<vsg::Node> body;
    vsg::ref_ptr<vsg::MatrixTransform> bodyTransform;
public:
    MyShip(vsg::ref_ptr<MyBuilder> builder, double sz = 1.0);
    void setPosition(double x, double y, double azim);
};