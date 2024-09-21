#include <vsg/all.h>
#include <vsgXchange/all.h>
#include <cmath>
#include <tinynurbs/tinynurbs.h>

#include "Demangle.h"
#include "DynamicText.h"
#include "DynamicLighting.h"
#include "MyBuilder.h"
#include "MyDrone.h"
#include "MyObject.h"
#include "MyQuad.h"
#include "MyFrustum.h"

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

    auto wideCamera = vsg::Perspective::create(30.0, 1.5, .1, 7.0);
    auto narrowCamera = vsg::Perspective::create(10.0, 1.5, .1, 7.0);
    auto drone = MyDrone::create(builder, narrowCamera, .3333);
    scene->addChild(drone);

    auto mo = MyQuad::create();
    scene->addChild(mo);

    auto ship = MyShip::create(builder, .3333);
    scene->addChild(ship);

    auto litScene = DynamicLighting::create(scene);

    // create the viewer and assign window(s) to it
    auto viewer = vsg::Viewer::create();

    viewer->addWindow(window);

    vsg::dvec3 centre{0.0, 0.0, 0.0};
    double radius = 18.0;
    auto viewPos = centre + vsg::dvec3{0.0, -5.0, 45.0};

    // set up the camera
    auto lookAt = vsg::LookAt::create(viewPos, centre, vsg::dvec3(0.0, 0.0, 1.0));

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
    size_t numFramesCompleted = 0;

    // rendering main loop
    while (viewer->advanceToNextFrame())
    {
        // litScene->setDirectional(1.0, vsg::vec3{cosf(t), sinf(t), 0.0f});
        viewer->handleEvents();
        viewer->update();

        auto t = std::chrono::duration<double, std::chrono::seconds::period>(vsg::clock::now() - startTime).count();
        double ipart;
        auto radians = -2.0 * M_PI * modf(t / 100.0, &ipart);
        drone->setPosition(-5.0 * sin(radians), 5.0 * cos(radians), 3.0, radians + M_PI/2);
        drone->setView(90.0, -45.0 + 10.0*sin(radians*10));
        drone->frustum->update(narrowCamera, vsg::dmat4());

        // auto results = drone->frustum->getZIntercept(drone->cameraView->transform());
        mo->update(drone->frustum);

        ship->setPosition(5.0 * sin(radians), -5.0 * cos(radians), radians - M_PI/2 + M_PI);

        viewer->recordAndSubmit();
        viewer->present();
        numFramesCompleted++;
    }

    auto duration = std::chrono::duration<double, std::chrono::seconds::period>(vsg::clock::now() - startTime).count();
    if (numFramesCompleted > 0.0)
    {
        std::cout << "Average frame rate = " << (double(numFramesCompleted) / duration) << std::endl;
    }

    return 0;
}
