#include "MyDrone.h"

MyDrone::MyDrone(vsg::ref_ptr<MyBuilder> builder)
{
    {
        vsg::GeometryInfo geomInfo;
        vsg::StateInfo stateInfo;

        geomInfo.dx = vsg::vec3{1.0, 0.0, 0.0};
        geomInfo.dy = vsg::vec3{0.0, 0.5, 0.0};
        geomInfo.dz = vsg::vec3{0.0, 0.0, 1.0};
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
        bodyTransform = vsg::MatrixTransform::create();
        bodyTransform->addChild(body);
        addChild(bodyTransform);

        viewMat = vsg::dmat4();
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
            { 5.0, 0.5 },
            { 5.0, 0.0 }
        });
        frustum = builder->createLathe(bat, 4, geomInfo, stateInfo);
        frustumTransform = vsg::MatrixTransform::create();
        frustumTransform->addChild(frustum);
        addChild(frustumTransform);
    }
}

void MyDrone::setPosition(double x, double y, double alt, double azim)
{
    bodyTransform->matrix  = vsg::translate(vsg::dvec3{x, y, alt}) * vsg::rotate(azim, 0.0, 0.0, 1.0);
    frustumTransform->matrix  = bodyTransform->matrix * viewMat;
}

void MyDrone::setView(float yaw, float pitch, float roll)
{
    viewMat = vsg::rotate(-yaw*M_PI/180, 0.0, 0.0, 1.0) * vsg::rotate(pitch*M_PI/180, 1.0, 0.0, 0.0) * vsg::rotate(roll*M_PI/180, 0.0, 1.0, 0.0);
    frustumTransform->matrix  = bodyTransform->matrix * viewMat;
}

MyShip::MyShip(vsg::ref_ptr<MyBuilder> builder)
{
    {
        vsg::GeometryInfo geomInfo;
        vsg::StateInfo stateInfo;

        geomInfo.dx = vsg::vec3{1.0, 0.0, 0.0};
        geomInfo.dy = vsg::vec3{0.0, 0.5, 0.0};
        geomInfo.dz = vsg::vec3{0.0, 0.0, .25};
        geomInfo.position = vsg::vec3{0.0, 0.0, 0.25};
        // geomInfo.color = vsg::vec4{1.0, 1.0, 1.0, 1.0};
        // geomInfo.transform = vsg::rotate(-M_PI/2.0, 1.0, 0.0, 0.0);
        body = builder->createBox(geomInfo, stateInfo);
        bodyTransform = vsg::MatrixTransform::create();
        bodyTransform->addChild(body);
        addChild(bodyTransform);
    }
}

void MyShip::setPosition(double x, double y, double azim)
{
    bodyTransform->matrix  = vsg::translate(vsg::dvec3{x, y, 0.0}) * vsg::rotate(azim, 0.0, 0.0, 1.0);
}
