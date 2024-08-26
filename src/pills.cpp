#include <vsg/all.h>
#include <vsgXchange/all.h>
#include <tinynurbs/tinynurbs.h>

#include "Demangle.h"
#include "DynamicText.h"

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

vsg::ref_ptr<vsg::Group> makeAxes(vsg::ref_ptr<vsg::Builder> builder);

vsg::ref_ptr<vsg::MatrixTransform> makeFrustum(vsg::ref_ptr<vsg::Builder> builder)
{
    vsg::GeometryInfo geomInfo;
    vsg::StateInfo stateInfo;

    stateInfo.wireframe = true;
    geomInfo.dx = {.35f, 0.0f, 0.0f};
    geomInfo.dy = {0.0f, .25f, 0.0f};
    geomInfo.dz = {0.0f, 0.0f, 1.0f};
    geomInfo.position = { 0.0f, 0.0f, 0.5f };
    geomInfo.color = vsg::vec4{1.0, .5, .25, 1.0};
    geomInfo.transform = vsg::mat4{
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f, 0.0f },
        { 0.25f, 0.25f, 0.0f, 1.0f },                
    };
    auto box = builder->createBox(geomInfo, stateInfo);
    auto obox = vsg::MatrixTransform::create();
    obox->addChild(box);
    return obox;
}

vsg::ref_ptr<vsg::Group> lightupScene(vsg::ref_ptr<vsg::Group> scene, const float& ambientIntensity, const float& directionalIntensity, const vsg::vec3& direction);

vsg::ref_ptr<vsg::ShaderSet> makeLineShader();
vsg::ref_ptr<vsg::StateGroup> makeLineGroup(vsg::ref_ptr<vsg::ShaderSet> shaderSet, vsg::vec4 color, float thickness, vsg::ref_ptr<vsg::vec3Array> vertices);
vsg::ref_ptr<vsg::StateGroup> makeXYGrid(vsg::ref_ptr<vsg::ShaderSet> shaderSet, vsg::ref_ptr<vsg::Font> font, vsg::ref_ptr<vsg::Options> options, vsg::vec4 color, float thickness, size_t mx, float scale, bool annotate);

class VSG_DECLSPEC MyBuilder : public vsg::Inherit<vsg::Builder, MyBuilder>
{
protected:
    vsg::Builder::GeometryMap _frustums;
public:
    MyBuilder() {}
    MyBuilder(const vsg::Builder& rhs) = delete;
    MyBuilder& operator=(const vsg::Builder& rhs) = delete;

    vsg::ref_ptr<vsg::Node> createFrustum(const vsg::GeometryInfo& info, const vsg::StateInfo& stateInfo, const float& pitch)
    {
        using namespace vsg;

        auto& subgraph = _frustums[std::make_pair(info, stateInfo)];
        if (subgraph)
        {
            return subgraph;
        }

        uint32_t instanceCount = 1;
        auto positions = instancePositions(info, instanceCount);
        auto colors = instanceColors(info, instanceCount);

        auto dx = info.dx;
        auto dy = info.dy;
        auto dz = info.dz;
        auto origin = info.position;
        auto [t_origin, t_scale, t_top] = y_texcoord(stateInfo).value;

        vec3 v000(origin - dx - dy);
        vec3 v010(origin - dx + dy);
        vec3 v100(origin + dx - dy);
        vec3 v110(origin + dx + dy);

        vec3 v001(origin - dx * pitch - dy * pitch + dz);
        vec3 v011(origin - dx * pitch + dy * pitch + dz);
        vec3 v101(origin + dx * pitch - dy * pitch + dz);
        vec3 v111(origin + dx * pitch + dy * pitch + dz);

        vec2 t00(0.0f, t_origin);
        vec2 t01(0.0f, t_top);
        vec2 t10(1.0f, t_origin);
        vec2 t11(1.0f, t_top);

        ref_ptr<vec3Array> vertices;
        ref_ptr<vec3Array> normals;
        ref_ptr<vec2Array> texcoords;
        ref_ptr<ushortArray> indices;

        if (stateInfo.wireframe) 
        {
            vec3 n0 = normalize(v000 - v111);
            vec3 n1 = normalize(v100 - v011);
            vec3 n2 = normalize(v110 - v001);
            vec3 n3 = normalize(v010 - v101);
            vec3 n4 = -n2;
            vec3 n5 = -n3;
            vec3 n6 = -n0;
            vec3 n7 = -n1;

            // set up vertex and index arrays
            vertices = vec3Array::create(
                {v000, v100, v110, v010,
                v001, v101, v111, v011});

            normals = vec3Array::create(
                {n0, n1, n2, n3,
                n4, n5, n6, n7});

            texcoords = vec2Array::create(
                {t00, t10, t11, t01,
                t00, t10, t11, t01});

            indices = ushortArray::create(
                {0, 1, 1, 2, 2, 3, 3, 0,
                0, 4, 1, 5, 2, 6, 3, 7,
                4, 5, 5, 6, 6, 7, 7, 4});
        }
        else
        {
            vec3 n0 = normalize(cross(dx, dz));
            vec3 n1 = normalize(cross(dy, dz));
            vec3 n2 = -n0;
            vec3 n3 = -n1;
            vec3 n4 = normalize(cross(dy, dx));
            vec3 n5 = -n4;

            // set up vertex and index arrays
            vertices = vec3Array::create(
                {v000, v100, v101, v001,   // front
                v100, v110, v111, v101,   // right
                v110, v010, v011, v111,   // far
                v010, v000, v001, v011,   // left
                v010, v110, v100, v000,   // bottom
                v001, v101, v111, v011}); // top

            normals = vec3Array::create(
                {n0, n0, n0, n0,
                n1, n1, n1, n1,
                n2, n2, n2, n2,
                n3, n3, n3, n3,
                n4, n4, n4, n4,
                n5, n5, n5, n5});

            texcoords = vec2Array::create(
                {t00, t10, t11, t01,
                t00, t10, t11, t01,
                t00, t10, t11, t01,
                t00, t10, t11, t01,
                t00, t10, t11, t01,
                t00, t10, t11, t01});

            indices = ushortArray::create(
                {0, 1, 2, 0, 2, 3,
                4, 5, 6, 4, 6, 7,
                8, 9, 10, 8, 10, 11,
                12, 13, 14, 12, 14, 15,
                16, 17, 18, 16, 18, 19,
                20, 21, 22, 20, 22, 23});
        }

        if (info.transform != identity)
        {
            transform(info.transform, vertices, normals);
        }

        // setup geometry
        auto vid = VertexIndexDraw::create();

        DataList arrays;
        arrays.push_back(vertices);
        if (normals) arrays.push_back(normals);
        if (texcoords) arrays.push_back(texcoords);
        if (colors) arrays.push_back(colors);
        if (positions) arrays.push_back(positions);
        vid->assignArrays(arrays);

        vid->assignIndices(indices);
        vid->indexCount = static_cast<uint32_t>(indices->size());
        vid->instanceCount = instanceCount;

        subgraph = decorateAndCompileIfRequired(info, stateInfo, vid);
        return subgraph;
    }

};

vsg::ref_ptr<vsg::Node> loadObject(vsg::ref_ptr<vsg::Options> options, const string& filepath, const vsg::dmat4& rot)
{
    auto model = vsg::read_cast<vsg::Node>(filepath, options);
    auto bounds = vsg::visit<vsg::ComputeBounds>(model).bounds;
    auto center = (bounds.min + bounds.max) / 2.0;
    auto sz = (bounds.max - bounds.min);
    auto s = 2.0 / std::max(std::max(sz[0], sz[1]), sz[2]);


    auto m = vsg::MatrixTransform::create();
    m->matrix = rot * vsg::scale(s, s, s) * vsg::translate(-center[0], -center[1], -center[2]);
    cout << demangle(m->matrix) << endl;
    cout << m->matrix << endl;
    m->addChild(model);
    return m;
}

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

// vsg::ref_ptr<vsg::StateGroup> generateMyObject();

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
    // scene->addChild(generateMyObject());
    vsg::GeometryInfo geomInfoF;
    vsg::StateInfo stateInfoF;
    geomInfoF.dx = vsg::vec3{0.1f, 0.0f, 0.0f};
    geomInfoF.dy = vsg::vec3{0.0f, 0.15f, 0.0f};
    geomInfoF.dz = vsg::vec3{0.0f, 0.0f, 10.0f};
    auto frustum = builder->createFrustum(geomInfoF, stateInfoF, 5.0f);
    scene->addChild(frustum);

    // auto text = DynamicText::create("origin", font, options);
    // text->matrix = vsg::scale(.2f, .2f, .2f);

    // scene->addChild(text);

    // scene->addChild(axes);

    vsg::GeometryInfo geomInfo;
    vsg::StateInfo stateInfo;
    geomInfo.color = vsg::vec4{.2, .2, .2, 1.0};
    stateInfo.two_sided = true;
    scene->addChild(builder->createQuad(geomInfo, stateInfo));

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
