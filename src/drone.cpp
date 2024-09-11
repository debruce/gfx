#include <vsg/all.h>
#include <vsgXchange/all.h>
#include <tinynurbs/tinynurbs.h>

#include "Demangle.h"
#include "DynamicText.h"
#include "DynamicLighting.h"
#include "MyBuilder.h"

#include <iostream>
#include <sstream>
#include <tuple>

using namespace std;

vsg::ref_ptr<vsg::Group> makeAxes(vsg::ref_ptr<vsg::Builder> builder);
vsg::ref_ptr<vsg::Node> loadObject(vsg::ref_ptr<vsg::Options> options, const string& filepath, const vsg::dmat4& rot);
vsg::ref_ptr<vsg::Group> lightupScene(vsg::ref_ptr<vsg::Group> scene, const float& ambientIntensity, const float& directionalIntensity, const vsg::vec3& direction);

vsg::ref_ptr<vsg::ShaderSet> makeLineShader();
vsg::ref_ptr<vsg::StateGroup> makeLineGroup(vsg::ref_ptr<vsg::ShaderSet> shaderSet, vsg::vec4 color, float thickness, vsg::ref_ptr<vsg::vec3Array> vertices);
vsg::ref_ptr<vsg::StateGroup> makeXYGrid(vsg::ref_ptr<vsg::ShaderSet> shaderSet, vsg::ref_ptr<vsg::Font> font, vsg::ref_ptr<vsg::Options> options, vsg::vec4 color, float thickness, size_t mx, float scale, bool annotate);


vsg::ref_ptr<vsg::Node> generateFlatOcean(vsg::ref_ptr<vsg::Builder> builder)
{
    vsg::GeometryInfo geomInfo;
    geomInfo.dx = vsg::vec3(20.0, 0.0, 0.0);
    geomInfo.dy = vsg::vec3(0.0, 20.0, 0.0);
    geomInfo.dz = vsg::vec3(0.0, 0.0, 0.0);
    geomInfo.color = vsg::vec4{.15, .15, .15, 1.0};
    vsg::StateInfo stateInfo;
    stateInfo.two_sided = true;
    return builder->createQuad(geomInfo, stateInfo);
}

int main(int argc, char** argv)
{
    auto options = vsg::Options::create();
    options->paths = vsg::getEnvPaths("VSG_FILE_PATH");
    options->sharedObjects = vsg::SharedObjects::create();
    options->add(vsgXchange::all::create());

    auto builder = MyBuilder::create();
    builder->options = options;

    auto font = vsg::read_cast<vsg::Font>("fonts/times.vsgb", options);

    auto windowTraits = vsg::WindowTraits::create();
    windowTraits->windowTitle = "drone";

    auto requestFeatures = windowTraits->deviceFeatures = vsg::DeviceFeatures::create();
    auto& requestedLineRasterizationFeatures = requestFeatures->get<VkPhysicalDeviceLineRasterizationFeaturesEXT, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_EXT>();
    requestedLineRasterizationFeatures.bresenhamLines = VK_TRUE;

    windowTraits->vulkanVersion = VK_API_VERSION_1_1;
    windowTraits->deviceExtensionNames.push_back(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME);

    auto window = vsg::Window::create(windowTraits);
    if (!window)
    {
        std::cout << "Could not create window." << std::endl;
        return 1;
    }

    auto scene = vsg::Group::create();

    auto lineShader = makeLineShader();
    scene->addChild(makeXYGrid(lineShader, font, options, vsg::vec4{1.0, 1.0, 1.0, 1.0}, 1.5, 10, 1.0, true));

    // scene->addChild(generateFlatOcean(builder));

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
    scene->addChild(builder->createBat(bat, 50, geomInfo, stateInfo));
    // scene->addChild(builder->createSphere(geomInfo, stateInfo));

    auto litScene = DynamicLighting::create(scene);

    // create the viewer and assign window(s) to it
    auto viewer = vsg::Viewer::create();

    viewer->addWindow(window);
            // auto p0 = vec3{c0 * r0, s0 * r0, z0};
            // auto p1 = vec3{c0 * r1, s0 * r1, z1};
            // auto p2 = vec3{c1 * r0, s1 * r0, z0};
            // auto p3 = vec3{c1 * r1, s1 * r1, z1};
    vsg::ref_ptr<vsg::LookAt> lookAt;

    // vsg::dvec3 centre = (bounds.min + bounds.max) * 0.5;
    // double radius = vsg::length(bounds.max - bounds.min) * 0.6;
    // cout << "center = " << centre << endl;
    // cout << "radius = " << radius << endl;
    vsg::dvec3 centre{0.0, 0.0, 0.0};
    double radius = 18.0;

    // set up the camera
    lookAt = vsg::LookAt::create(centre + vsg::dvec3(0.0, -radius * 3.5, 0.0), centre, vsg::dvec3(0.0, 0.0, 1.0));

    double nearFarRatio = 0.001;
    auto perspective = vsg::Perspective::create(30.0, static_cast<double>(window->extent2D().width) / static_cast<double>(window->extent2D().height), nearFarRatio * radius, radius * 10.0);

    auto camera = vsg::Camera::create(perspective, lookAt, vsg::ViewportState::create(window->extent2D()));

    // add the camera and scene graph to View
    auto view = vsg::View::create();
    view->camera = camera;
    view->addChild(litScene);

    // add close handler to respond to the close window button and to pressing escape
    viewer->addEventHandler(vsg::CloseHandler::create(viewer));
    viewer->addEventHandler(vsg::Trackball::create(camera));

    auto renderGraph = vsg::RenderGraph::create(window, view);
    auto commandGraph = vsg::CommandGraph::create(window, renderGraph);
    viewer->assignRecordAndSubmitTaskAndPresentation({commandGraph});

    auto resourceHints = vsg::ResourceHints::create();
    resourceHints->shadowMapSize = vsg::vec2{4096, 4096};
    viewer->compile(resourceHints);

    auto startTime = vsg::clock::now();
    double numFramesCompleted = 0.0;

    // rendering main loop
    while (viewer->advanceToNextFrame())
    {
        auto t = std::chrono::duration<double, std::chrono::seconds::period>(vsg::clock::now() - startTime).count();
        litScene->setDirectional(1.0, vsg::vec3{cosf(t), sinf(t), 0.0f});
        viewer->handleEvents();
        viewer->update();
        viewer->recordAndSubmit();
        viewer->present();
        numFramesCompleted += 1.0;
    }

    auto duration = std::chrono::duration<double, std::chrono::seconds::period>(vsg::clock::now() - startTime).count();
    if (numFramesCompleted > 0.0)
    {
        std::cout << "Average frame rate = " << (numFramesCompleted / duration) << std::endl;
    }

    return 0;
}
