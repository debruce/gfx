#pragma once

#include "MyBuilder.h"
#include <vsg/all.h>

class MyDrone : public vsg::Inherit<vsg::Group, MyDrone> {
    vsg::ref_ptr<vsg::Node> body;
public:
    MyDrone(vsg::ref_ptr<MyBuilder> builder);

    // void setDirectional(float intentsity, const vsg::vec3& dir);
};