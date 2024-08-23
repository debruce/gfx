#include <vsg/all.h>

vsg::ref_ptr<vsg::Group> lightupScene(vsg::ref_ptr<vsg::Group> scene, const float& ambientIntensity, const float& directionalIntensity, const vsg::vec3& direction)
{
    // auto span = vsg::length(bounds.max - bounds.min);
    auto litScene = vsg::Group::create();
    litScene->addChild(scene);

    auto ambientLight = vsg::AmbientLight::create();
    ambientLight->name = "ambient";
    ambientLight->color.set(1.0f, 1.0f, 1.0f);
    ambientLight->intensity = ambientIntensity;
    litScene->addChild(ambientLight);

    auto directionalLight = vsg::DirectionalLight::create();
    directionalLight->name = "directional";
    directionalLight->color.set(1.0f, 1.0f, 1.0f);
    directionalLight->intensity = directionalIntensity;
    // directionalLight->direction.set(0.0f, -1.0f, -1.0f);
    directionalLight->direction.set(direction.x, direction.y, direction.z);
    litScene->addChild(directionalLight);

    // auto pointLight = vsg::PointLight::create();
    // pointLight->name = "point";
    // pointLight->color.set(1.0f, 1.0f, 0.0);
    // pointLight->intensity = static_cast<float>(span * 0.5);
    // pointLight->position.set(static_cast<float>(bounds.min.x), static_cast<float>(bounds.min.y), static_cast<float>(bounds.max.z + span * 0.3));
    // // enable culling of the point light by decorating with a CullGroup
    // auto cullGroup = vsg::CullGroup::create();
    // cullGroup->bound.center = pointLight->position;
    // cullGroup->bound.radius = span;
    // cullGroup->addChild(pointLight);
    // litScene->addChild(cullGroup);

    // auto spotLight = vsg::SpotLight::create();
    // spotLight->name = "spot";
    // spotLight->color.set(0.0f, 1.0f, 1.0f);
    // spotLight->intensity = static_cast<float>(span * 0.5);
    // spotLight->position.set(static_cast<float>(bounds.max.x + span * 0.1), static_cast<float>(bounds.min.y - span * 0.1), static_cast<float>(bounds.max.z + span * 0.3));
    // spotLight->direction = (bounds.min + bounds.max) * 0.5 - spotLight->position;
    // spotLight->innerAngle = vsg::radians(8.0f);
    // spotLight->outerAngle = vsg::radians(9.0f);
    // // enable culling of the spot light by decorating with a CullGroup
    // auto cullGroup = vsg::CullGroup::create();
    // cullGroup->bound.center = spotLight->position;
    // cullGroup->bound.radius = span;
    // cullGroup->addChild(spotLight);
    // litScene->addChild(cullGroup);

    // auto ambientLight = vsg::AmbientLight::create();
    // ambientLight->name = "ambient";
    // ambientLight->color.set(1.0f, 1.0f, 1.0f);
    // ambientLight->intensity = 0.1f;
    // auto directionalLight = vsg::DirectionalLight::create();
    // directionalLight->name = "head light";
    // directionalLight->color.set(1.0f, 1.0f, 1.0f);
    // directionalLight->intensity = 0.9f;
    // directionalLight->direction.set(0.0f, 0.0f, -1.0f);
    // auto absoluteTransform = vsg::AbsoluteTransform::create();
    // absoluteTransform->addChild(ambientLight);
    // absoluteTransform->addChild(directionalLight);
    // litScene->addChild(absoluteTransform);

    return litScene;
}