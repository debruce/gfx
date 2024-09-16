#include "MyQuad.h"

#include <iostream>

std::string VERT{R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants { mat4 projection; mat4 modelView; };

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 in_texcoord;

out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out vec2 out_texcoord;

void main()
{
    gl_Position = (projection * modelView) * vec4(vertex, 1.0);
    out_texcoord = in_texcoord;
}

)"};

std::string FRAG{R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 out_texcoord;
layout(location = 0) out vec4 color;
layout(set = 0, binding = 0) uniform sampler2D texSampler;

void main()
{
    color = texture(texSampler, out_texcoord);
    // color = vec4(1.0, 0.0, 0.0, 1.0);
}

)"};

struct SetMyPipelineStates : public vsg::Visitor    // auto indices = ushortArray::create({0, 1, 2, 3, 0});

{
    VkPrimitiveTopology topo;
    const vsg::StateInfo si;

    SetMyPipelineStates(const VkPrimitiveTopology& topo, const vsg::StateInfo& si) : topo(topo), si(si){}

    void apply(vsg::Object& object) override { object.traverse(*this); }
    void apply(vsg::RasterizationState& rs) override { rs.cullMode = VK_CULL_MODE_NONE; }
    void apply(vsg::InputAssemblyState& ias) override { ias.topology = topo; }
    void apply(vsg::ColorBlendState& cbs) override { cbs.configureAttachments(si.blending); }
};

using namespace std;


MyQuad::MyQuad(vsg::ref_ptr<const vsg::Options> options, const vsg::vec3& a,     // auto indices = ushortArray::create({0, 1, 2, 3, 0});
const vsg::vec3& b, const vsg::vec3& c, const vsg::vec3& d)
{
    using namespace vsg;

    auto vertexShader = vsg::ShaderStage::create(VK_SHADER_STAGE_VERTEX_BIT, "main", VERT);
    auto fragmentShader = vsg::ShaderStage::create(VK_SHADER_STAGE_FRAGMENT_BIT, "main", FRAG);
    if (!vertexShader || !fragmentShader)
    {
        std::cout << "Could not create shaders." << std::endl;
        exit(-1);
    }
    auto shaderSet = vsg::ShaderSet::create(vsg::ShaderStages{vertexShader, fragmentShader});
    
    vsg::PushConstantRanges pushConstantRanges{
        {VK_SHADER_STAGE_VERTEX_BIT, 0, 128} // projection and modelview matrices
    };

    vsg::VertexInputState::Bindings vertexBindingsDescriptions{
        VkVertexInputBindingDescription{0, 12, VK_VERTEX_INPUT_RATE_VERTEX}, // interleaved vertex, color, and texcoord data in a single array
    };

    vsg::VertexInputState::Attributes vertexAttributeDescriptions{
        VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},  // vertex data
        VkVertexInputAttributeDescription{1, 0, VK_FORMAT_R32G32_SFLOAT, 12},    // tex coord data
    };

    vsg::GraphicsPipelineStates pipelineStates{
        vsg::VertexInputState::create(vertexBindingsDescriptions, vertexAttributeDescriptions),
        vsg::InputAssemblyState::create(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE),
        vsg::RasterizationState::create(),
        vsg::MultisampleState::create(),
        vsg::ColorBlendState::create(),
        vsg::DepthStencilState::create()};

    vsg::DescriptorSetLayoutBindings descriptorBindings{
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr} // { binding, descriptorType, descriptorCount, stageFlags, pImmutableSamplers}
    };

    auto descriptorSetLayout = vsg::DescriptorSetLayout::create(descriptorBindings);

    auto pipelineLayout = vsg::PipelineLayout::create(vsg::DescriptorSetLayouts{descriptorSetLayout}, pushConstantRanges);
    // auto pipelineLayout = vsg::PipelineLayout::create(vsg::DescriptorSetLayouts{}, pushConstantRanges);
    auto graphicsPipeline = vsg::GraphicsPipeline::create(pipelineLayout, vsg::ShaderStages{vertexShader, fragmentShader}, pipelineStates);
    auto bindGraphicsPipeline = vsg::BindGraphicsPipeline::create(graphicsPipeline);

    vertices = vec3Array::create({a, b, c, d});
    vertices->properties.dataVariance = vsg::DYNAMIC_DATA;
    auto texcoords = vec2Array::create({vec2{0.0, 0.0}, vec2{1.0, 0.0}, vec2{1.0, 1.0}, vec2{0.0, 1.0}});
    auto indices = ushortArray::create({0, 2, 1, 0, 3, 2});

    DataList arrays;
    arrays.push_back(vertices);
    arrays.push_back(texcoords);

    auto vid = VertexIndexDraw::create();
    vid->assignArrays(arrays);
    vid->assignIndices(indices);
    vid->indexCount = static_cast<uint32_t>(indices->size());
    vid->instanceCount = 1;

    image = ubvec4Array2D::create(512, 512);
    for (auto i = 0; i < 512; i++) {
        for (auto j = 0; j < 512; j++) {
            if ((i & 64) == (j & 64))
                image->at(i,j) = vsg::ubvec4{255, 0, 255, 255};
            else
                image->at(i,j) = vsg::ubvec4{0, 255, 0, 255};
        }
    }
    image->properties.format = VK_FORMAT_R8G8B8A8_UNORM;
    image->properties.dataVariance = vsg::DYNAMIC_DATA;
    image->dirty();

    auto sampler = vsg::Sampler::create();
    sampler->addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler->addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

    auto descriptorImage = vsg::DescriptorImage::create(sampler, image, 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    auto descriptorSet = vsg::DescriptorSet::create(descriptorSetLayout, vsg::Descriptors{descriptorImage});
    auto bindDescriptorSet = vsg::BindDescriptorSet::create(VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->layout, 0, descriptorSet);

    stateGroup = vsg::StateGroup::create();

    stateGroup->addChild(bindGraphicsPipeline);
    stateGroup->addChild(vid);
    stateGroup->addChild(bindDescriptorSet);

    addChild(stateGroup);
}

void MyQuad::update(const vsg::dvec3& a, const vsg::dvec3& b, const vsg::dvec3& c, const vsg::dvec3& d)
{
    vertices->at(0) = a;
    vertices->at(1) = b;
    vertices->at(2) = c;
    vertices->at(3) = d;
    vertices->dirty();
}

