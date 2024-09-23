#include "MyDrone.h"

#include <iostream>

vsg::ref_ptr<vsg::Group> makeAxes(vsg::ref_ptr<vsg::Builder> builder);

MyDrone::MyDrone(vsg::ref_ptr<MyBuilder> builder, vsg::ref_ptr<vsg::Perspective> proj, double sz)
{
    {
        vsg::GeometryInfo geomInfo;
        vsg::StateInfo stateInfo;

        geomInfo.dx = vsg::vec3{1.0, 0.0, 0.0} * float(sz);
        geomInfo.dy = vsg::vec3{0.0, 0.5, 0.0} * float(sz);
        geomInfo.dz = vsg::vec3{0.0, 0.0, 1.0} * float(sz);
        // geomInfo.color = vsg::vec4{1.0, 1.0, 1.0, 1.0};
        geomInfo.transform = vsg::rotate(-M_PI/2.0, 1.0, 0.0, 0.0);
        auto bat = vsg::vec2Array::create({
            {-1.0, 0.0},
            {-1.0, 0.5},
            { 0.0, 0.5},
            { 0.0, 1.0},
            { 1.0, 0.0}
        });
        body = builder->createLathe(bat, 50, geomInfo, stateInfo);
        forwardView = AbsoluteLookAtTransform::create();
        forwardView->addChild(body);
        addChild(forwardView);
    }

    frustum = MyFrustum::create(forwardView, proj, "lookTowardPosY");
    addChild(frustum);
}

void MyDrone::setPosition(double x, double y, double alt, double azim)
{
    forwardView->set(vsg::rotate(M_PI-azim, 0.0, 0.0, 1.0) * vsg::translate(vsg::dvec3{-x, -y, -alt}));
}

void MyDrone::setView(double yaw, double pitch, double roll)
{
    frustum->relativeView.matrix =
        vsg::rotate(-yaw*M_PI/180, 0.0, 0.0, 1.0)
        * vsg::rotate(pitch*M_PI/180, 1.0, 0.0, 0.0)
        * vsg::rotate(roll*M_PI/180, 0.0, 1.0, 0.0);
}

// void MyDrone::setProjection(vsg::ref_ptr<vsg::Perspective> proj)
// {
//     frustum->update(proj);
// }
