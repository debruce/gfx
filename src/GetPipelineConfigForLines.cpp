#include <vsg/all.h>
#include <iostream>
#include <iomanip>
#include <sstream>
// #include "Demangle.h"

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

class LocalText : public vsg::Inherit<vsg::MatrixTransform, LocalText> {
    vsg::ref_ptr<vsg::Options> options;
    vsg::ref_ptr<vsg::stringValue> label;
    vsg::ref_ptr<vsg::Text> text;
public:
    LocalText(const std::string& s, vsg::ref_ptr<vsg::Font> font, vsg::ref_ptr<vsg::Options> options) : options(options)
    {
        label = vsg::stringValue::create(s);
        auto layout = vsg::StandardLayout::create();
        layout->glyphLayout = vsg::StandardLayout::LEFT_TO_RIGHT_LAYOUT;
        layout->horizontalAlignment = vsg::StandardLayout::CENTER_ALIGNMENT;
        layout->verticalAlignment = vsg::StandardLayout::BOTTOM_ALIGNMENT;
        layout->position = vsg::vec3(0.0, 0.0, 0.0);
        layout->horizontal = vsg::vec3(1.0, 0.0, 0.0);
        layout->vertical = vsg::vec3(0.0, 0.0, 1.0);
        layout->color = vsg::vec4(0.0, 0.0, 0.0, 1.0);

        text = vsg::Text::create();
        text->technique = vsg::GpuLayoutTechnique::create();
        text->text = label;
        text->font = font;
        text->layout = layout;
        text->setup(64);

        addChild(text);
    }

    void set(const std::string& s)
    {
        label->value() = vsg::make_string(s);
        text->setup(0, options);
    }
};

vsg::ref_ptr<vsg::StateGroup> makeXYGrid(vsg::ref_ptr<vsg::ShaderSet> shaderSet, vsg::ref_ptr<vsg::Font> font, vsg::ref_ptr<vsg::Options> options, vsg::vec4 color, float thickness, size_t mx, float scale, bool annotate)
{
    size_t line_count = 2 * mx + 1;
    auto lines = vsg::vec3Array::create(2 * 2 * line_count);
    float grid_max = float(mx) * scale;
    size_t idx = 0;
    for (auto i = 0; i < line_count; i++) {
        float off = float(i) * scale - grid_max;
        (*lines)[idx++] = vsg::vec3{-grid_max, off, 0};
        (*lines)[idx++] = vsg::vec3{grid_max, off, 0};
    }
    for (auto i = 0; i < line_count; i++) {
        float off = float(i) * scale - grid_max;
        (*lines)[idx++] = vsg::vec3{off, -grid_max, 0};
        (*lines)[idx++] = vsg::vec3{off, grid_max, 0};
    }
    auto ret = makeLineGroup(shaderSet, color, thickness, lines);
    if (annotate) {
        using namespace std;

        for (auto xi = 0; xi < line_count; xi++) {
            float xoff = float(xi) * scale - grid_max;
            for (auto yi = 0; yi < line_count; yi++) {
                float yoff = float(yi) * scale - grid_max;
                stringstream ss;

                ss << fixed << setprecision(2) << '(' << xoff <<  ',' << yoff << ')';
                auto layout = vsg::StandardLayout::create();
                layout->glyphLayout = vsg::StandardLayout::LEFT_TO_RIGHT_LAYOUT;
                layout->horizontalAlignment = vsg::StandardLayout::CENTER_ALIGNMENT;
                layout->verticalAlignment = vsg::StandardLayout::BOTTOM_ALIGNMENT;
                layout->position = vsg::vec3(xoff, yoff, 0.0);
                layout->horizontal = vsg::vec3(scale*.1, 0.0, 0.0);
                layout->vertical = vsg::vec3(0.0, 0.0, scale*.1);
                layout->color = vsg::vec4(0.0, 0.0, 0.0, 1.0);

                auto text = vsg::Text::create();
                text->technique = vsg::GpuLayoutTechnique::create();
                text->text = vsg::stringValue::create(ss.str());
                text->font = font;
                text->layout = layout;
                text->setup(0, options);
                ret->addChild(text);
            }
        }      
    }
    return ret;
}