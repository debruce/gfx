#include "MyShip.h"

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
