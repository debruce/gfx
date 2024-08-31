#include <vsg/all.h>
#include <vsgXchange/all.h>

#include <iostream>

using namespace std;

struct SetMyPipelineStates : public vsg::Visitor
{
    VkPrimitiveTopology topo;
    const vsg::StateInfo si;

    // explicit SetMyPipelineStates(const vsg::StateInfo& in) :
    //     si(in) {}

    SetMyPipelineStates(const VkPrimitiveTopology& topo, const vsg::StateInfo& si) : topo(topo), si(si){}

    void apply(vsg::Object& object) override
    {
        object.traverse(*this);
    }

    void apply(vsg::RasterizationState& rs) override
    {
        // if (si.two_sided)
        rs.cullMode = VK_CULL_MODE_NONE;
    }

    void apply(vsg::InputAssemblyState& ias) override
    {
        if (si.wireframe) ias.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        else ias.topology = topo;
    }

    void apply(vsg::ColorBlendState& cbs) override
    {
        cbs.configureAttachments(si.blending);
    }

};

std::string VERT{R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants { mat4 projection; mat4 modelView; };

layout(location = 0) in vec2 vertex;
layout(location = 0) out vec2 pos;

out gl_PerVertex { vec4 gl_Position; };

void main()
{
    gl_Position = (projection * modelView) * vec4(vertex, 0.0, 1.0);
    pos = gl_Position.xy;
}

)"};

std::string FRAG{R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 pos;
layout(location = 0) out vec4 color;

vec2 square(vec2 z)
{
    return vec2(z.x*z.x - z.y*z.y, 2*z.x*z.y);
}

float zabs(vec2 z)
{
    return sqrt(z.x * z.x + z.y * z.y);
}

void main()
{
    int cnt = 0;
    vec2 z = {0.0, 0.0};

    while (cnt < 256) {
        vec2 zp = square(z) + pos;
        if (zabs(zp) > 100.0) {
            break;
        }
        z = zp;
        cnt += 1;
    }
    float c = log2(cnt+1) / 8.0;
    color = vec4(c, 0.0, 1.0 - c, 1.0);
}

)"};

int main(int argc, char** argv)
{
    auto options = vsg::Options::create();
    options->paths = vsg::getEnvPaths("VSG_FILE_PATH");
    options->sharedObjects = vsg::SharedObjects::create();
    options->add(vsgXchange::all::create());

    // auto builder = MyBuilder::create();
    // builder->options = options;

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

    auto vertices = vsg::vec2Array::create({
        { -1, -1 },
        { +1, -1 },
        { -1, +1 },
        { +1, +1 }
    });

    auto vertexShader = vsg::ShaderStage::create(VK_SHADER_STAGE_VERTEX_BIT, "main", VERT);
    auto fragmentShader = vsg::ShaderStage::create(VK_SHADER_STAGE_FRAGMENT_BIT, "main", FRAG);
    auto shaderSet = vsg::ShaderSet::create(vsg::ShaderStages{vertexShader, fragmentShader});
    shaderSet->addPushConstantRange("pc", "", VK_SHADER_STAGE_VERTEX_BIT, 0, 128);
    shaderSet->addAttributeBinding("vertex", "", 0, VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));

    auto stateGroup = vsg::StateGroup::create();
    auto gpConf = vsg::GraphicsPipelineConfigurator::create(shaderSet);

    vsg::StateInfo si;
    SetMyPipelineStates sps(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, si);
    gpConf->accept(sps);
    gpConf->init();
    gpConf->copyTo(stateGroup);
    scene->addChild(stateGroup);

    vsg::DataList vertexArrays;
    gpConf->assignArray(vertexArrays, "vertex", VK_VERTEX_INPUT_RATE_VERTEX, vertices);
    auto vertexDraw = vsg::VertexDraw::create();
    vertexDraw->assignArrays(vertexArrays);
    vertexDraw->vertexCount = vertices->width();
    vertexDraw->instanceCount = 1;
    stateGroup->addChild(vertexDraw);

    scene->addChild(stateGroup);

    auto viewer = vsg::Viewer::create();
    viewer->addWindow(window);
    viewer->addEventHandler(vsg::CloseHandler::create(viewer));

    auto viewportState = vsg::ViewportState::create(window->extent2D());
    auto camMat = vsg::Orthographic::create();
    auto lookAt = vsg::LookAt::create(
        vsg::dvec3{0.0, 0.0, 1.0},
        vsg::dvec3{0.0, 0.0, 0.0},
        vsg::dvec3{0.0, 1.0, 0.0}
    );
    auto camera = vsg::Camera::create(camMat, lookAt, viewportState);
    auto commandGraph = vsg::createCommandGraphForView(window, camera, scene);
    viewer->assignRecordAndSubmitTaskAndPresentation({commandGraph});
    viewer->compile();

    auto startTime = vsg::clock::now();
    double numFramesCompleted = 0.0;

    // rendering main loop
    while (viewer->advanceToNextFrame() && (numFrames < 0 || (numFrames--) > 0))
    {
        auto t = std::chrono::duration<double, std::chrono::seconds::period>(vsg::clock::now() - startTime).count();
        // grab_node->matrix = vsg::translate(vsg::vec3(0.0f, sin(t), 1.0f))
        //     * vsg::scale(vsg::vec3(.2f, .2f, .2f)) * vsg::rotate(vsg::radians(45.0f * (float)sin(t)), 0.0f, 1.0f, 0.0f);
        // litScene->setDirectional(1.0, vsg::vec3{cosf(t), sinf(t), -1.0f});
        // // grab_node->matrix = vsg::rotate(vsg::radians(45.0f * (float)sin(t)), 0.0f, 1.0f, 0.0f);

        // // gpConf->assignDescriptor("color", vsg::vec4Array::create({{1.0f, sinf(t) * .5f + .5f, 0.0f, 1.0f}}));
        // // text->set(to_string(numFramesCompleted));
        // // text->matrix = vsg::rotate(vsg::radians(25.0f * (float)sin(3.0*t)), 0.0f, 0.0f, 1.0f) * vsg::scale(vsg::vec3{.2, .2, .2});

        // // pass any events into EventHandlers assigned to the Viewer
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
