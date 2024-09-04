#include "DynamicLighting.h"

DynamicLighting::DynamicLighting(vsg::ref_ptr<vsg::Group> scene)
{
    addChild(scene);

    ambientLight = vsg::AmbientLight::create();
    ambientLight->name = "ambient";
    ambientLight->color.set(1.0f, 0.0f, 0.0f);
    ambientLight->intensity = .1;
    addChild(ambientLight);

    directionalLight = vsg::DirectionalLight::create();
    directionalLight->name = "directional";
    directionalLight->color.set(1.0f, 1.0f, 1.0f);
    directionalLight->intensity = 1.0;
    directionalLight->direction.set(0.0f, -1.0f, -1.0f);
    directionalLight->shadowSettings = vsg::HardShadows::create(1);
    addChild(directionalLight);

    directionalLightFixed = vsg::DirectionalLight::create();
    directionalLightFixed->name = "directionalRed";
    directionalLightFixed->color.set(1.0f, 0.0f, 0.0f);
    directionalLightFixed->intensity = 1.0;
    directionalLightFixed->direction.set(1.0f, 1.0f, 0.0f);
    directionalLightFixed->shadowSettings = vsg::HardShadows::create(1);
    addChild(directionalLightFixed);
}

void DynamicLighting::setDirectional(float intensity, const vsg::vec3& dir)
{
    directionalLight->intensity = intensity;
    directionalLight->direction = dir;
}