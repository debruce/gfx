#pragma once

#include <vsg/all.h>

class AbsoluteLookAtTransform : public vsg::Inherit<vsg::Transform, AbsoluteLookAtTransform> {
public:
    vsg::ref_ptr<vsg::LookAt> lookAt;

    AbsoluteLookAtTransform() : lookAt(vsg::LookAt::create()) {}

    void set(const vsg::dmat4& matrix)
    {
        lookAt->set(matrix);

    }

    vsg::dmat4 transform(const vsg::dmat4 &mv = vsg::dmat4()) const override
    {
        return mv * lookAt->transform();
    }
};

class RelativeLookAtTransform : public vsg::Inherit<vsg::Transform, RelativeLookAtTransform> {
public:
    vsg::RelativeViewMatrix relativeView;

    RelativeLookAtTransform(vsg::ref_ptr<vsg::ViewMatrix> vm) : relativeView(vsg::dmat4(), vm)
    {
    }

    vsg::dmat4 transform(const vsg::dmat4 &mv = vsg::dmat4()) const override
    {
        return mv * relativeView.viewMatrix->transform() * relativeView.matrix;
    }
};
