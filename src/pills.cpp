#include <vsg/all.h>
#include <vsgXchange/all.h>
#include <tinynurbs/tinynurbs.h>

#include "Demangle.h"
#include "DynamicText.h"
#include "MyBuilder.h"

#include <iostream>
#include <sstream>
#include <tuple>

using namespace std;

// ostream& operator<<(ostream& os, const vsg::GeometryInfo& gi)
// {
//     stringstream ss;
//     ss << "[ position=" << gi.position << " transform=" << gi.transform << " color=" << gi.color << " ]";
//     return os << ss.str();
// }

vsg::ref_ptr<vsg::Group> makeAxes(vsg::ref_ptr<vsg::Builder> builder);
vsg::ref_ptr<vsg::Node> loadObject(vsg::ref_ptr<vsg::Options> options, const string& filepath, const vsg::dmat4& rot);
vsg::ref_ptr<vsg::Group> lightupScene(vsg::ref_ptr<vsg::Group> scene, const float& ambientIntensity, const float& directionalIntensity, const vsg::vec3& direction);

vsg::ref_ptr<vsg::ShaderSet> makeLineShader();
vsg::ref_ptr<vsg::StateGroup> makeLineGroup(vsg::ref_ptr<vsg::ShaderSet> shaderSet, vsg::vec4 color, float thickness, vsg::ref_ptr<vsg::vec3Array> vertices);
vsg::ref_ptr<vsg::StateGroup> makeXYGrid(vsg::ref_ptr<vsg::ShaderSet> shaderSet, vsg::ref_ptr<vsg::Font> font, vsg::ref_ptr<vsg::Options> options, vsg::vec4 color, float thickness, size_t mx, float scale, bool annotate);

vsg::ref_ptr<vsg::Node> loadPlane(vsg::ref_ptr<vsg::Options> options)
{
    return loadObject(options, "../models/plane.obj", vsg::rotate(vsg::radians(180.0), 0.0, 0.0, 1.0));
}

vsg::ref_ptr<vsg::Node> loadBoat(vsg::ref_ptr<vsg::Options> options)
{
    return loadObject(options, "../models/boat.vsgt",
        vsg::translate(0.0, 0.0, .35)
        * vsg::rotate(vsg::radians(-90.0), 0.0, 0.0, 1.0)
        * vsg::rotate(vsg::radians(90.0), -1.0, 0.0, 0.0));
}

vsg::ref_ptr<vsg::StateGroup> generateMyObject(vsg::ref_ptr<vsg::Options> options);

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

vsg::ref_ptr<vsg::Node> generateBumpyOcean(vsg::ref_ptr<vsg::Builder> builder)
{
    size_t sz = 256;
    auto textureData = vsg::floatArray2D::create(sz, sz);\
    float off = 127.5;
    for (int i = 0; i < textureData->height(); i++) {
        for (int j = 0; j < textureData->width(); j++) {
            float x = (i - off) / 32.0;
            float y = (j - off) / 32.0;
            textureData->at(i,j) = exp(-(x*x + y*y));
        }
    }
    textureData->properties.format = VK_FORMAT_R32_SFLOAT;
    textureData->properties.dataVariance = vsg::STATIC_DATA;

    // auto textureData = vsg::vec3Array2D::create(256, 256);
    // for (size_t i = 0; i < textureData->height(); i++) {
    //     for (size_t j = 0; j < textureData->width(); j++) {
    //         textureData->at(i,j) = vsg::vec3(height(i, j), .5, .25);
    //     }
    // }
    // textureData->properties.format = VK_FORMAT_R32G32B32_SFLOAT;;
    // textureData->properties.dataVariance = vsg::STATIC_DATA;

    vsg::GeometryInfo geomInfo;
    geomInfo.dx = vsg::vec3(20.0, 0.0, 0.0);
    geomInfo.dy = vsg::vec3(0.0, 20.0, 0.0);
    geomInfo.dz = vsg::vec3(0.0, 0.0, 1.0);
    vsg::StateInfo stateInfo;
    stateInfo.displacementMap = textureData;

    return builder->createHeightField(geomInfo, stateInfo);
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
    windowTraits->windowTitle = "pills";

    // set up defaults and read command line arguments to override them
    vsg::CommandLine arguments(&argc, argv);
    windowTraits->debugLayer = arguments.read({"--debug", "-d"});
    windowTraits->apiDumpLayer = arguments.read({"--api", "-a"});

    auto requestFeatures = windowTraits->deviceFeatures = vsg::DeviceFeatures::create();
    auto& requestedLineRasterizationFeatures = requestFeatures->get<VkPhysicalDeviceLineRasterizationFeaturesEXT, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_EXT>();
    requestedLineRasterizationFeatures.bresenhamLines = VK_TRUE;

    windowTraits->vulkanVersion = VK_API_VERSION_1_1;
    windowTraits->deviceExtensionNames.push_back(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME);

    arguments.read("--screen", windowTraits->screenNum);
    arguments.read("--display", windowTraits->display);
    auto numFrames = arguments.value(-1, "-f");
    if (arguments.read({"--fullscreen", "--fs"})) windowTraits->fullscreen = true;
    if (arguments.read({"--window", "-w"}, windowTraits->width, windowTraits->height)) { windowTraits->fullscreen = false; }
    if (arguments.read("--IMMEDIATE")) windowTraits->swapchainPreferences.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    if (arguments.read({"-t", "--test"}))
    {
        windowTraits->swapchainPreferences.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        windowTraits->fullscreen = true;
    }

    auto window = vsg::Window::create(windowTraits);
    if (!window)
    {
        std::cout << "Could not create window." << std::endl;
        return 1;
    }

    auto scene = vsg::Group::create();

    auto lineShader = makeLineShader();
    scene->addChild(makeXYGrid(lineShader, font, options, vsg::vec4{1.0, 1.0, 1.0, 1.0}, 1.5, 10, 1.0, true));
    // scene->addChild(makeXYGrid(lineShader, font, options, vsg::vec4{.5, 0.5, 0.5, 1.0}, 1.0, 100, .1, true));

    auto axes = makeAxes(builder);

    auto grab_node = vsg::MatrixTransform::create();
    grab_node->addChild(axes);
    scene->addChild(grab_node);

    // stateInfo.wireframe = true;
    // scene->addChild(loadPlane(options));
    // scene->addChild(loadBoat(options));

    scene->addChild(generateMyObject(options));

    scene->addChild(generateFlatOcean(builder));
    // scene->addChild(generateBumpyOcean(builder));

    // compute the bounds of the scene graph to help position camera
    // auto bounds = vsg::visit<vsg::ComputeBounds>(scene).bounds;

    scene = lightupScene(scene, .3f, .85f, vsg::vec3{0.0f, -1.0f, -1.0f});

    // create the viewer and assign window(s) to it
    auto viewer = vsg::Viewer::create();

    viewer->addWindow(window);

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
    view->addChild(scene);

    // add close handler to respond to the close window button and to pressing escape
    viewer->addEventHandler(vsg::CloseHandler::create(viewer));
    viewer->addEventHandler(vsg::Trackball::create(camera));

    auto renderGraph = vsg::RenderGraph::create(window, view);
    auto commandGraph = vsg::CommandGraph::create(window, renderGraph);
    viewer->assignRecordAndSubmitTaskAndPresentation({commandGraph});

    viewer->compile();

    auto startTime = vsg::clock::now();
    double numFramesCompleted = 0.0;

    // rendering main loop
    while (viewer->advanceToNextFrame() && (numFrames < 0 || (numFrames--) > 0))
    {
        auto t = std::chrono::duration<double, std::chrono::seconds::period>(vsg::clock::now() - startTime).count();
        grab_node->matrix = vsg::translate(vsg::vec3(0.0f, sin(t), 1.0f))
            * vsg::scale(vsg::vec3(.2f, .2f, .2f)) * vsg::rotate(vsg::radians(45.0f * (float)sin(t)), 0.0f, 1.0f, 0.0f);
        // grab_node->matrix = vsg::rotate(vsg::radians(45.0f * (float)sin(t)), 0.0f, 1.0f, 0.0f);

        // gpConf->assignDescriptor("color", vsg::vec4Array::create({{1.0f, sinf(t) * .5f + .5f, 0.0f, 1.0f}}));
        // text->set(to_string(numFramesCompleted));
        // text->matrix = vsg::rotate(vsg::radians(25.0f * (float)sin(3.0*t)), 0.0f, 0.0f, 1.0f) * vsg::scale(vsg::vec3{.2, .2, .2});

        // pass any events into EventHandlers assigned to the Viewer
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
