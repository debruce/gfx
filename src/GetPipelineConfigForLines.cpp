#include <vsg/all.h>

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

vsg::ref_ptr<vsg::ShaderSet> makeLineShader()
{
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

    auto vertexShader = vsg::ShaderStage::create(VK_SHADER_STAGE_VERTEX_BIT, "main", VERT);
    auto fragmentShader = vsg::ShaderStage::create(VK_SHADER_STAGE_FRAGMENT_BIT, "main", FRAG);
    auto shaderSet = vsg::ShaderSet::create(vsg::ShaderStages{vertexShader, fragmentShader});
    shaderSet->addPushConstantRange("pc", "", VK_SHADER_STAGE_VERTEX_BIT, 0, 128);
    shaderSet->addAttributeBinding("vertex", "", 0, VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));
    return shaderSet;
}

vsg::ref_ptr<vsg::GraphicsPipelineConfigurator> getPipelineConfigForLines()
{
    auto shaderSet = makeLineShader();

    auto gpConf = vsg::GraphicsPipelineConfigurator::create(shaderSet);
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

    gpConf->pipelineStates.push_back(ExtendedRasterizationState::create());
    gpConf->accept(sps);
    gpConf->init();
    return gpConf;
}
