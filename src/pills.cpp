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

class ExtendedRasterizationState : public vsg::Inherit<vsg::RasterizationState, ExtendedRasterizationState>
{
public:
    ExtendedRasterizationState() {}
    ExtendedRasterizationState(const ExtendedRasterizationState& rs) :
        Inherit(rs) {}

    void apply(vsg::Context& context, VkGraphicsPipelineCreateInfo& pipelineInfo) const override
    {
        // create and assign the VkPipelineRasterizationStateCreateInfo as usual using the base class that wil assign it to pipelineInfo.pRasterizationState
        RasterizationState::apply(context, pipelineInfo);

        /// setup extension feature (stippling) for attachment to pNext below
        auto rastLineStateCreateInfo = context.scratchMemory->allocate<VkPipelineRasterizationLineStateCreateInfoEXT>(1);
        rastLineStateCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO_EXT;
        rastLineStateCreateInfo->pNext = nullptr;
        rastLineStateCreateInfo->lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT;
        rastLineStateCreateInfo->stippledLineEnable = VK_TRUE;
        rastLineStateCreateInfo->lineStippleFactor = 4;
        rastLineStateCreateInfo->lineStipplePattern = 0b1111111100000000;

        // to assign rastLineStateCreateInfo to the pRasterizationState->pNext we have to cast away const first
        // this is safe as these objects haven't been passed to Vulkan yet
        auto pRasterizationState = const_cast<VkPipelineRasterizationStateCreateInfo*>(pipelineInfo.pRasterizationState);
        pRasterizationState->pNext = rastLineStateCreateInfo;
    }

protected:
    virtual ~ExtendedRasterizationState() {}
};

std::string VERT{R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable
layout(push_constant) uniform PushConstants { mat4 projection; mat4 modelView; };
layout(location = 0) in vec3 vertex;
out gl_PerVertex { vec4 gl_Position; };
void main() { gl_Position = (projection * modelView) * vec4(vertex, 1.0); }
)"};

std::string FRAG{R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable
layout(location = 0) out vec4 color;
void main() { color = vec4(1, 0, 0, 1); }
)"};

vsg::ref_ptr<vsg::Group> makeAxes(vsg::ref_ptr<vsg::Builder> builder);

vsg::ref_ptr<vsg::Group> lightupScene(vsg::ref_ptr<vsg::Group> scene, const float& ambientIntensity, const float& directionalIntensity, const vsg::vec3& direction);

int main(int argc, char** argv)
{
    auto options = vsg::Options::create();
    options->paths = vsg::getEnvPaths("VSG_FILE_PATH");
    options->sharedObjects = vsg::SharedObjects::create();
    options->add(vsgXchange::all::create());

    auto builder = vsg::Builder::create();
    builder->options = options;

    auto font = vsg::read_cast<vsg::Font>("fonts/times.vsgb", options);

    auto windowTraits = vsg::WindowTraits::create();
    windowTraits->windowTitle = "pills";

    // set up defaults and read command line arguments to override them
    vsg::CommandLine arguments(&argc, argv);
    windowTraits->debugLayer = arguments.read({"--debug", "-d"});
    windowTraits->apiDumpLayer = arguments.read({"--api", "-a"});
    auto requestFeatures = windowTraits->deviceFeatures = vsg::DeviceFeatures::create();
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

    auto physicalDevice = window->getOrCreatePhysicalDevice();
    if (!physicalDevice->supportsDeviceExtension(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME))
    {
        std::cout << "Line Rasterization Extension not supported.\n";
        return 1;
    }
    auto& requestedLineRasterizationFeatures = requestFeatures->get<VkPhysicalDeviceLineRasterizationFeaturesEXT, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_EXT>();
    requestedLineRasterizationFeatures.bresenhamLines = VK_TRUE;

    auto vertexShader = vsg::ShaderStage::create(VK_SHADER_STAGE_VERTEX_BIT, "main", VERT);
    auto fragmentShader = vsg::ShaderStage::create(VK_SHADER_STAGE_FRAGMENT_BIT, "main", FRAG);
    auto shaderSet = vsg::ShaderSet::create(vsg::ShaderStages{vertexShader, fragmentShader});
    shaderSet->addPushConstantRange("pc", "", VK_SHADER_STAGE_VERTEX_BIT, 0, 128);
    shaderSet->addAttributeBinding("vertex", "", 0, VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));

    auto lineGroup = vsg::StateGroup::create();
    auto gpConf = vsg::GraphicsPipelineConfigurator::create(shaderSet);

    auto scene = vsg::Group::create();

    auto vertices = vsg::vec3Array::create({
        {0, 0, 0},
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1},
        {0, 0, 0},
    });
    vsg::DataList vertexArrays;
    gpConf->assignArray(vertexArrays, "vertex", VK_VERTEX_INPUT_RATE_VERTEX, vertices);
    auto vertexDraw = vsg::VertexDraw::create();
    vertexDraw->assignArrays(vertexArrays);
    vertexDraw->vertexCount = vertices->width();
    vertexDraw->instanceCount = 1;
    lineGroup->addChild(vertexDraw);

    struct SetPipelineStates : public vsg::Visitor
    {
        void apply(vsg::Object& object) { object.traverse(*this); }
        void apply(vsg::RasterizationState& rs)
        {
            rs.lineWidth = 2.0f;
            rs.cullMode = VK_CULL_MODE_NONE;
        }
        void apply(vsg::InputAssemblyState& ias)
        {
            ias.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        }
    } sps;

    /// apply our custom RasterizationState to the GraphicsPipeline
    auto rs = ExtendedRasterizationState::create();
    gpConf->pipelineStates.push_back(rs);

    gpConf->accept(sps);
    gpConf->init();
    gpConf->copyTo(lineGroup);
    scene->addChild(lineGroup);

    auto axes = makeAxes(builder);

    auto grab_node = vsg::MatrixTransform::create();
    grab_node->addChild(axes);
    scene->addChild(grab_node);

    auto text = DynamicText::create("origin", font, options);
    text->matrix = vsg::scale(.2f, .2f, .2f);

    scene->addChild(text);

    // scene->addChild(axes);
    vsg::GeometryInfo geomInfo;
    vsg::StateInfo stateInfo;
    stateInfo.two_sided = true;
    scene->addChild(builder->createQuad(geomInfo, stateInfo));

    // compute the bounds of the scene graph to help position camera
    auto bounds = vsg::visit<vsg::ComputeBounds>(scene).bounds;

    scene = lightupScene(scene, .3f, .85f, vsg::vec3{0.0f, -1.0f, -1.0f});

    // create the viewer and assign window(s) to it
    auto viewer = vsg::Viewer::create();

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

        text->set(to_string(numFramesCompleted));
        text->matrix = vsg::rotate(vsg::radians(25.0f * (float)sin(3.0*t)), 0.0f, 0.0f, 1.0f) * vsg::scale(vsg::vec3{.2, .2, .2});

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
