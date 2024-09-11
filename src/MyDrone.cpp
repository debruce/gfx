#include "MyDrone.h"

MyDrone::MyDrone(vsg::ref_ptr<MyBuilder> builder)
{
    vsg::GeometryInfo geomInfo;
    vsg::StateInfo stateInfo;

    geomInfo.dx = vsg::vec3{0.5, 0.0, 0.0};
    geomInfo.dy = vsg::vec3{0.0, 1.0, 0.0};
    geomInfo.dz = vsg::vec3{0.0, 0.0, 1.0};
    geomInfo.color = vsg::vec4{1.0, 1.0, 1.0, 1.0};
    geomInfo.transform = vsg::rotate(M_PI/2.0, 0.0, 1.0, 0.0);
    auto bat = vsg::vec2Array::create({
        {-1.0, 0.0},
        {-1.0, 0.5},
        { 0.0, 0.5},
        { 0.0, 1.0},
        { 1.0, 0.0}
    });
    addChild(builder->createBat(bat, 50, geomInfo, stateInfo));
}