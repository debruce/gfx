#include <vsg/all.h>
#include <iostream>
#include <iomanip>
#include <sstream>

static std::string VERT{R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants {
    mat4 projection;
    mat4 modelView;
};

layout(set = 0, binding = 0) uniform Params {
    vec4 color;
} parms;

layout(location = 0) in vec3 vertex;

layout(location = 0) out vec4 fragColor;

out gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
};

void main()
{
    gl_Position = (projection * modelView) * vec4(vertex, 1.0);
    fragColor = parms.color;
    gl_PointSize = 3.0;
}

)"};

static std::string FRAG{R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 fragColor;

layout(location = 0) out vec4 color;

void main()
{
    color = fragColor;
}

)"};

vsg::ref_ptr<vsg::StateGroup> generateMyObject()
{
    auto vertices = vsg::vec3Array::create(52);
    (*vertices)[0] = vsg::vec3(0, 0, 0);
    for (int i = 0; i <= 50; i++) {
        (*vertices)[i+1] = vsg::vec3(cos(i * 2 * M_PI / 50), 0, sin(i * 2 * M_PI / 50));
    }

    auto vertexShader = vsg::ShaderStage::create(VK_SHADER_STAGE_VERTEX_BIT, "main", VERT);
    auto fragmentShader = vsg::ShaderStage::create(VK_SHADER_STAGE_FRAGMENT_BIT, "main", FRAG);
    auto shaderSet = vsg::ShaderSet::create(vsg::ShaderStages{vertexShader, fragmentShader});

    shaderSet->addPushConstantRange("pc", "", VK_SHADER_STAGE_VERTEX_BIT, 0, 128);
    shaderSet->addDescriptorBinding("color", "", 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr);
    shaderSet->addAttributeBinding("vertex", "", 0, VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));

    auto gpConf = vsg::GraphicsPipelineConfigurator::create(shaderSet);

    // gpConf->pipelineStates.push_back(ExtendedRasterizationState::create());
    struct SetPipelineStates : public vsg::Visitor
    {
        VkPrimitiveTopology topo;

        SetPipelineStates(const VkPrimitiveTopology& topo) : topo(topo) {}

        void apply(vsg::Object& object) { object.traverse(*this); }
        void apply(vsg::RasterizationState& rs)
        {
            rs.lineWidth = 1.0;
            rs.cullMode = VK_CULL_MODE_NONE;
        }
        void apply(vsg::InputAssemblyState& ias)
        {
            ias.topology = topo;
        }
    } sps(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    gpConf->accept(sps);
    gpConf->init();

    vsg::DataList vertexArrays;
    gpConf->assignArray(vertexArrays, "vertex", VK_VERTEX_INPUT_RATE_VERTEX, vertices);
    auto vertexDraw = vsg::VertexDraw::create();
    vertexDraw->assignArrays(vertexArrays);
    vertexDraw->vertexCount = vertices->width();
    vertexDraw->instanceCount = 1;
    auto lineGroup = vsg::StateGroup::create();
    lineGroup->addChild(vertexDraw);
    gpConf->assignDescriptor("color", vsg::vec4Array::create({vsg::vec4{1.0, .5, 0.0, 1.0}}));
    gpConf->enableDescriptor("color");
    gpConf->copyTo(lineGroup);
    return lineGroup;
}
