#include "MyDrone.h"

#include <iostream>

vsg::ref_ptr<vsg::Group> makeAxes(vsg::ref_ptr<vsg::Builder> builder);

MyDrone::MyDrone(vsg::ref_ptr<MyBuilder> builder, double sz)
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
        forwardView = LookAtTransform::create();
        forwardView->addChild(body);
        // forwardView = LookAtTransform::create();
        // forwardView->addChild(makeAxes(builder));
        addChild(forwardView);
    }

    {
        vsg::GeometryInfo geomInfo;
        vsg::StateInfo stateInfo;

        geomInfo.dx = vsg::vec3{1.0, 0.0, 0.0};
        geomInfo.dy = vsg::vec3{0.0, 1.0, 0.0};
        geomInfo.dz = vsg::vec3{0.0, 0.0, 1.0};
        // geomInfo.color = vsg::vec4{1.0, 1.0, 1.0, 1.0};
        geomInfo.transform = vsg::rotate(-M_PI/2.0, 1.0, 0.0, 0.0) * vsg::rotate(M_PI/4.0, 0.0, 0.0, 1.0);
        stateInfo.two_sided = true;
        stateInfo.wireframe = true;
        auto bat = vsg::vec2Array::create({
            { 1.0, 0.0 },
            { 1.0, 0.1 },
            { 10.0, 1.5 },
            { 10.0, 0.0 }
        });
        frustum = builder->createLathe(bat, 4, geomInfo, stateInfo, 0.0*M_PI/4.0);
        cameraView = RelativeViewTransform::create(forwardView->lookAt);
        cameraView->addChild(frustum);
        addChild(cameraView);
    }
}

void MyDrone::setPosition(double x, double y, double alt, double azim)
{
    forwardView->set(vsg::rotate(M_PI-azim, 0.0, 0.0, 1.0) * vsg::translate(vsg::dvec3{-x, -y, -alt}));
}

void MyDrone::setView(double yaw, double pitch, double roll)
{
    cameraView->relativeView.matrix =
        vsg::rotate(-yaw*M_PI/180, 0.0, 0.0, 1.0)
        * vsg::rotate(pitch*M_PI/180, 1.0, 0.0, 0.0)
        * vsg::rotate(roll*M_PI/180, 0.0, 1.0, 0.0);
}

vsg::dvec3 MyDrone::getIntercept(const vsg::dvec3& frustumPt)
{
    // auto t0 = frustumTransform->matrix * vsg::t_vec3<double>{0.0, 0.0, 0.0};
    // auto t1 = frustumTransform->matrix * frustumPt;
    // auto diff = t1 - t0;
    // auto t = -t0.z / diff.z;
    // return t0 + diff * t;
    return vsg::dvec3();
}

MyShip::MyShip(vsg::ref_ptr<MyBuilder> builder, double sz)
{
    vsg::GeometryInfo g;
    vsg::StateInfo s;
    auto meshPts = vsg::vec3Array2D::create(5, 5);
    meshPts->at(0,0) = { -0.5f, -1.0f, 0.0f };
    meshPts->at(0,1) = { -0.5f, 0.75f, 0.0f };
    meshPts->at(0,2) = { 0.0f, 1.0f, 0.0f };
    meshPts->at(0,3) = { 0.5f, 0.75f, 0.0f };
    meshPts->at(0,4) = { 0.5f, -1.0f, 0.0f };

    meshPts->at(1,0) = { -0.5f, -1.0f, 0.25f };
    meshPts->at(1,1) = { -0.5f, 0.75f, 0.25f };
    meshPts->at(1,2) = { 0.0f, 1.0f, 0.25f };
    meshPts->at(1,3) = { 0.5f, 0.75f, 0.25f };
    meshPts->at(1,4) = {0.5f, -1.0f, 0.25f };

    meshPts->at(2,0) = { -0.25f, -0.5f, 0.25f };
    meshPts->at(2,1) = { -0.25f, 0.5f * 0.5f, 0.25f };
    meshPts->at(2,2) = { 0.0f, 0.5f, 0.25f };
    meshPts->at(2,3) = { 0.25f, 0.5f * 0.5f, 0.25f };
    meshPts->at(2,4) = { 0.25f, -0.5f, 0.25f };

    meshPts->at(3,0) = { -0.25f, -0.5f, 0.5f };
    meshPts->at(3,1) = { -0.25f, 0.5f * 0.5f, 0.5f };
    meshPts->at(3,2) = { 0.0f, 0.5f, 0.5f };
    meshPts->at(3,3) = { 0.25f, 0.5f * 0.5f, 0.5f };
    meshPts->at(3,4) = { 0.25f, -0.5f, 0.5f };

    meshPts->at(4,0) = { 0.0f, 0.0f, 0.5f };
    meshPts->at(4,1) = { 0.0f, 0.0f, 0.5f };
    meshPts->at(4,2) = { 0.0f, 0.0f, 0.5f };
    meshPts->at(4,3) = { 0.0f, 0.0f, 0.5f };
    meshPts->at(4,4) = { 0.0f, 0.0f, 0.5f };

    for (auto r = 0; r < meshPts->height(); r++) {
        for (auto c = 0; c < meshPts->width(); c++) {
            meshPts->at(c, r) *= sz;
        }
    }

    body = builder->createMesh(meshPts, g, s);
    bodyTransform = vsg::MatrixTransform::create();
    bodyTransform->addChild(body);
    addChild(bodyTransform);
}

void MyShip::setPosition(double x, double y, double azim)
{
    bodyTransform->matrix  = vsg::translate(vsg::dvec3{x, y, 0.0}) * vsg::rotate(azim, 0.0, 0.0, 1.0);
}
