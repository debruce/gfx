#include <vsg/all.h>
#include <iostream>
#include "Demangle.h"

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
        rastLineStateCreateInfo->stippledLineEnable = VK_FALSE;
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

vsg::ref_ptr<vsg::ShaderSet> makeLineShader()
{


    auto vertexShader = vsg::ShaderStage::create(VK_SHADER_STAGE_VERTEX_BIT, "main", VERT);
    auto fragmentShader = vsg::ShaderStage::create(VK_SHADER_STAGE_FRAGMENT_BIT, "main", FRAG);
    auto shaderSet = vsg::ShaderSet::create(vsg::ShaderStages{vertexShader, fragmentShader});
    shaderSet->addPushConstantRange("pc", "", VK_SHADER_STAGE_VERTEX_BIT, 0, 128);
    shaderSet->addDescriptorBinding("color", "", 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr);
    shaderSet->addAttributeBinding("vertex", "", 0, VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));
    return shaderSet;
}

static vsg::ref_ptr<vsg::GraphicsPipelineConfigurator> getPipelineConfigForLines(vsg::ref_ptr<vsg::ShaderSet> shaderSet, float thickness)
{
    auto gpConf = vsg::GraphicsPipelineConfigurator::create(shaderSet);
    struct SetPipelineStates : public vsg::Visitor
    {
        float th;

        SetPipelineStates(float th) : th(th) {}

        void apply(vsg::Object& object) { object.traverse(*this); }
        void apply(vsg::RasterizationState& rs)
        {
            rs.lineWidth = th;
            rs.cullMode = VK_CULL_MODE_NONE;
        }
        void apply(vsg::InputAssemblyState& ias)
        {
            ias.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        }
    } sps(thickness);

    gpConf->pipelineStates.push_back(ExtendedRasterizationState::create());
    gpConf->accept(sps);
    gpConf->init();
    return gpConf;
}

vsg::ref_ptr<vsg::StateGroup> makeLineGroup(vsg::ref_ptr<vsg::ShaderSet> shaderSet, vsg::vec4 color, float thickness, vsg::ref_ptr<vsg::vec3Array> vertices)
{
    auto gpConf = getPipelineConfigForLines(shaderSet, thickness);
    vsg::DataList vertexArrays;
    gpConf->assignArray(vertexArrays, "vertex", VK_VERTEX_INPUT_RATE_VERTEX, vertices);
    auto vertexDraw = vsg::VertexDraw::create();
    vertexDraw->assignArrays(vertexArrays);
    vertexDraw->vertexCount = vertices->width();
    vertexDraw->instanceCount = 1;
    auto lineGroup = vsg::StateGroup::create();
    lineGroup->addChild(vertexDraw);
    gpConf->assignDescriptor("color", vsg::vec4Array::create({color}));
    gpConf->enableDescriptor("color");
    gpConf->copyTo(lineGroup);
    return lineGroup; 
}