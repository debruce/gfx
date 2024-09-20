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

    frustum = MyFrustum::create(vsg::Perspective::create(15.0, 1.5, .1, 20.0), "lookTowardPosY");
    cameraView = RelativeViewTransform::create(forwardView->lookAt);
    cameraView->addChild(frustum);
    addChild(cameraView);
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

std::array<vsg::dvec2, 4> simpleCorners = {
    vsg::dvec2{-1,-1},  // lower left
    vsg::dvec2{+1,-1},  // lower right
    vsg::dvec2{-1,+1},  // upper left
    vsg::dvec2{+1,+1}   // upper right
};

std::array<vsg::dvec3,4> MyDrone::getGroundCorners()
{
    using namespace std;
    std::array<vsg::dvec3,4> results;
    auto pinv = vsg::inverse(proj->transform());
    auto m = cameraView->transform();
    cout << "m = " << m << endl;
    // auto m = vsg::inverse(proj->transform());
    for (auto i = 0; i < results.size(); i++) {
        auto pt0 = pinv * vsg::dvec3{simpleCorners[i].x, simpleCorners[i].y, 1.0};
        auto t0 = m * vsg::dvec4{-pt0.z, pt0.y, pt0.x, 1.0};
        auto pt1 = pinv * vsg::dvec3{simpleCorners[i].x, simpleCorners[i].y, .0};
        auto t1 = m * vsg::dvec4{-pt1.z, pt1.y, pt1.x, 1.0};
        auto diff = t1 - t0;
        auto t = -t0.z / diff.z;
        auto out = t0 + diff * t;
        cout << i << ": "
            // << " pt0=" << pt0
            << " t0=" << t0
            // << " pt1=" << pt1
            << " t1=" << t1
            << " diff=" << diff
            << " t=" << t
            << endl;
        results[i] = vsg::dvec3{out.x, out.y, out.z};    
    }
    // // auto inv = vsg::inverse(proj->transform() * cameraView->transform());
    // cout << "proj=" << proj->transform() << endl;
    // auto inv = vsg::inverse(proj->transform());
    // auto pt = inv * frustumPt;
    // // cout << "inv = " << inv << endl;
    // cout << "mapped = " << pt << endl;
    // // cout << "projected = " << pt << endl;
    // // auto t0 = frustumTransform->matrix * vsg::t_vec3<double>{0.0, 0.0, 0.0};
    // // auto t1 = frustumTransform->matrix * frustumPt;
    // // auto diff = t1 - t0;
    // // auto t = -t0.z / diff.z;
    // // return t0 + diff * t;
    return results;
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
