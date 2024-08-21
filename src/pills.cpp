#include <vsg/all.h>

#ifdef vsgXchange_FOUND
#    include <vsgXchange/all.h>
#endif

#include <iostream>
#include <sstream>
#include <tuple>

using namespace std;

ostream& operator<<(ostream& os, const vsg::GeometryInfo& gi)
{
    stringstream ss;
    ss << "[ position=" << gi.position << " transform=" << gi.transform << " color=" << gi.color << " ]";
    return os << ss.str();
}

vsg::ref_ptr<vsg::MatrixTransform> makeStovePipe(vsg::ref_ptr<vsg::Builder> builder, const vsg::vec4& clr)
{
    vsg::GeometryInfo geomInfo;
    vsg::StateInfo stateInfo;

    float thickness = .05f;
    geomInfo.dx = {thickness, 0.0f, 0.0f};
    geomInfo.dy = {0.0f, thickness, 0.0f};
    geomInfo.dz = {0.0f, 0.0f, 1.0f};
    geomInfo.color = clr;
    auto z_cylinder = builder->createCylinder(geomInfo, stateInfo);
    geomInfo.dx = {.25f, 0.0f, 0.0f};
    geomInfo.dy = {0.0f, .25f, 0.0f};
    geomInfo.dz = {0.0f, 0.0f, .25f};
    geomInfo.transform = vsg::translate(0.0f, 0.0f, 0.5f);
    auto z_cone = builder->createCone(geomInfo, stateInfo);
    auto stove_pipe = vsg::MatrixTransform::create();
    stove_pipe->addChild(z_cylinder);
    stove_pipe->addChild(z_cone);
    return stove_pipe;
}

vsg::ref_ptr<vsg::Group> makeAxes(vsg::ref_ptr<vsg::Builder> builder)
{
    auto zStovePipe = makeStovePipe(builder, vsg::vec4{0.0f, 0.0f, 1.0f, 1.0f});

    auto xStovePipe = makeStovePipe(builder, vsg::vec4{1.0f, 0.0f, 0.0f, 1.0f});
    xStovePipe->matrix = vsg::rotate(vsg::radians(90.0f), 0.0f, 1.0f, 0.0f);

    auto yStovePipe = makeStovePipe(builder, vsg::vec4{0.0f, 1.0f, 0.0f, 1.0f});
    yStovePipe->matrix = vsg::rotate(vsg::radians(90.0f), 1.0f, 0.0f, 0.0f);

    auto axes = vsg::Group::create();
    axes->addChild(zStovePipe);
    axes->addChild(xStovePipe);
    axes->addChild(yStovePipe);
    return axes;
}

std::tuple<vsg::ref_ptr<vsg::Node>, vsg::ref_ptr<vsg::MatrixTransform>> createTestScene(vsg::ref_ptr<vsg::Options> options)
{
    auto builder = vsg::Builder::create();
    builder->options = options;

    auto font = vsg::read_cast<vsg::Font>("fonts/times.vsgb", options);
    auto scene = vsg::Group::create();

    vsg::ref_ptr<vsg::MatrixTransform> grab_node;

    auto axes = makeAxes(builder);

    grab_node = vsg::MatrixTransform::create();
    grab_node->addChild(axes);
    scene->addChild(grab_node);

    vsg::GeometryInfo geomInfo;
    vsg::StateInfo stateInfo;

    auto layout = vsg::StandardLayout::create();
    layout->glyphLayout = vsg::StandardLayout::LEFT_TO_RIGHT_LAYOUT;
    layout->horizontalAlignment = vsg::StandardLayout::CENTER_ALIGNMENT;
    layout->verticalAlignment = vsg::StandardLayout::BOTTOM_ALIGNMENT;
    layout->position = vsg::vec3(0.0, 0.0, 0.0);
    layout->horizontal = vsg::vec3(0.1, 0.0, 0.0);
    layout->vertical = vsg::vec3(0.0, 0.1, 0.0);
    layout->color = vsg::vec4(0.0, 0.0, 0.0, 1.0);
    auto text = vsg::Text::create();
    text->text = vsg::stringValue::create("origin");
    text->font = font;
    text->layout = layout;
    text->setup(0, options);
    scene->addChild(text);

    // scene->addChild(axes);
    stateInfo.two_sided = true;
    scene->addChild(builder->createQuad(geomInfo, stateInfo));

    return std::make_tuple(scene, grab_node);
}

int main(int argc, char** argv)
{
    auto options = vsg::Options::create();
    options->paths = vsg::getEnvPaths("VSG_FILE_PATH");
    options->sharedObjects = vsg::SharedObjects::create();

#ifdef vsgXchange_all
    // add vsgXchange's support for reading and writing 3rd party file formats
    options->add(vsgXchange::all::create());
#endif

    auto windowTraits = vsg::WindowTraits::create();
    windowTraits->windowTitle = "vsglights";

    // set up defaults and read command line arguments to override them
    vsg::CommandLine arguments(&argc, argv);
    windowTraits->debugLayer = arguments.read({"--debug", "-d"});
    windowTraits->apiDumpLayer = arguments.read({"--api", "-a"});

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
    if (arguments.read("--st"))
    {
        windowTraits->swapchainPreferences.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        windowTraits->width = 192, windowTraits->height = 108;
        windowTraits->decoration = false;
    }

    // bool useStagingBuffer = arguments.read({"--staging-buffer", "-s"});

    auto outputFilename = arguments.value<vsg::Path>("", "-o");

    bool add_ambient = false;
    bool add_directional = true;
    bool add_point = false;
    bool add_spotlight = false;
    bool add_headlight = arguments.read("--headlight");
    if (add_headlight || arguments.read({"--no-lights", "-n"}))
    {
        add_ambient = false;
        add_directional = false;
        add_point = false;
        add_spotlight = false;
    }


    auto tup = createTestScene(options);
    auto scene = std::get<0>(tup);
    auto grab_node = std::get<1>(tup);

    // compute the bounds of the scene graph to help position camera
    auto bounds = vsg::visit<vsg::ComputeBounds>(scene).bounds;

    if (add_ambient || add_directional || add_point || add_spotlight || add_headlight)
    {
        auto span = vsg::length(bounds.max - bounds.min);
        auto group = vsg::Group::create();
        group->addChild(scene);

        // ambient light
        if (add_ambient)
        {
            auto ambientLight = vsg::AmbientLight::create();
            ambientLight->name = "ambient";
            ambientLight->color.set(1.0f, 1.0f, 1.0f);
            ambientLight->intensity = 0.01f;
            group->addChild(ambientLight);
        }

        // directional light
        if (add_directional)
        {
            auto directionalLight = vsg::DirectionalLight::create();
            directionalLight->name = "directional";
            directionalLight->color.set(1.0f, 1.0f, 1.0f);
            directionalLight->intensity = 0.85f;
            directionalLight->direction.set(0.0f, -1.0f, -1.0f);
            group->addChild(directionalLight);
        }

        // point light
        if (add_point)
        {
            auto pointLight = vsg::PointLight::create();
            pointLight->name = "point";
            pointLight->color.set(1.0f, 1.0f, 0.0);
            pointLight->intensity = static_cast<float>(span * 0.5);
            pointLight->position.set(static_cast<float>(bounds.min.x), static_cast<float>(bounds.min.y), static_cast<float>(bounds.max.z + span * 0.3));

            // enable culling of the point light by decorating with a CullGroup
            auto cullGroup = vsg::CullGroup::create();
            cullGroup->bound.center = pointLight->position;
            cullGroup->bound.radius = span;

            cullGroup->addChild(pointLight);

            group->addChild(cullGroup);
        }

        // spot light
        if (add_spotlight)
        {
            auto spotLight = vsg::SpotLight::create();
            spotLight->name = "spot";
            spotLight->color.set(0.0f, 1.0f, 1.0f);
            spotLight->intensity = static_cast<float>(span * 0.5);
            spotLight->position.set(static_cast<float>(bounds.max.x + span * 0.1), static_cast<float>(bounds.min.y - span * 0.1), static_cast<float>(bounds.max.z + span * 0.3));
            spotLight->direction = (bounds.min + bounds.max) * 0.5 - spotLight->position;
            spotLight->innerAngle = vsg::radians(8.0f);
            spotLight->outerAngle = vsg::radians(9.0f);

            // enable culling of the spot light by decorating with a CullGroup
            auto cullGroup = vsg::CullGroup::create();
            cullGroup->bound.center = spotLight->position;
            cullGroup->bound.radius = span;

            cullGroup->addChild(spotLight);

            group->addChild(cullGroup);
        }

        if (add_headlight)
        {
            auto ambientLight = vsg::AmbientLight::create();
            ambientLight->name = "ambient";
            ambientLight->color.set(1.0f, 1.0f, 1.0f);
            ambientLight->intensity = 0.1f;

            auto directionalLight = vsg::DirectionalLight::create();
            directionalLight->name = "head light";
            directionalLight->color.set(1.0f, 1.0f, 1.0f);
            directionalLight->intensity = 0.9f;
            directionalLight->direction.set(0.0f, 0.0f, -1.0f);

            auto absoluteTransform = vsg::AbsoluteTransform::create();
            absoluteTransform->addChild(ambientLight);
            absoluteTransform->addChild(directionalLight);

            group->addChild(absoluteTransform);
        }

        scene = group;
    }

    // write out scene if required
    if (outputFilename)
    {
        vsg::write(scene, outputFilename, options);
        return 0;
    }

    // create the viewer and assign window(s) to it
    auto viewer = vsg::Viewer::create();

    auto window = vsg::Window::create(windowTraits);
    if (!window)
    {
        std::cout << "Could not create window." << std::endl;
        return 1;
    }

    viewer->addWindow(window);

    vsg::ref_ptr<vsg::LookAt> lookAt;

    vsg::dvec3 centre = (bounds.min + bounds.max) * 0.5;
    double radius = vsg::length(bounds.max - bounds.min) * 0.6;

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
