#include "MyQuad.h"

#include <iostream>

static std::string VERT{R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants { mat4 projection; mat4 modelView; };

layout(location = 0) in vec3 vertex;

out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out vec4 world;

void main()
{
    world = modelView * vec4(vertex, 1.0);
    gl_Position = projection * world;
}

)"};

static std::string FRAG{R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 world;
layout(location = 0) out vec4 color;
layout(set = 0, binding = 0) uniform sampler2D texSampler;

layout(set = 0, binding = 1) uniform ProjectiveParams 
{
    mat4 inverseCombo;
}; 

void main()
{
    vec4 lookupCoord = inverseCombo * vec4(world.xy, 0.0, 1.0);
    color = texture(texSampler, lookupCoord.xy / lookupCoord.w);
}

)"};

struct ProjectiveUniform
{
    vsg::mat4 inverseCombo;
};

using ProjectiveUniformValue = vsg::Value<ProjectiveUniform>;

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

vsg::vec3 narrow(const vsg::dvec3& in)
{
    return vsg::vec3{float(in.x), float(in.y), float(in.z)};
}

MyQuad::MyQuad(vsg::ref_ptr<const vsg::Options> options, const std::array<vsg::dvec3, 4>& points)
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
    };

    vsg::GraphicsPipelineStates pipelineStates{
        vsg::VertexInputState::create(vertexBindingsDescriptions, vertexAttributeDescriptions),
        vsg::InputAssemblyState::create(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE),
        vsg::RasterizationState::create(),
        vsg::MultisampleState::create(),
        vsg::ColorBlendState::create(),
        vsg::DepthStencilState::create()};

    vsg::DescriptorSetLayoutBindings descriptorBindings{
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
    };

    auto descriptorSetLayout = vsg::DescriptorSetLayout::create(descriptorBindings);

    auto pipelineLayout = vsg::PipelineLayout::create(vsg::DescriptorSetLayouts{descriptorSetLayout}, pushConstantRanges);
    auto graphicsPipeline = vsg::GraphicsPipeline::create(pipelineLayout, vsg::ShaderStages{vertexShader, fragmentShader}, pipelineStates);
    auto bindGraphicsPipeline = vsg::BindGraphicsPipeline::create(graphicsPipeline);

    vertices = vec3Array::create(4);
    vertices->at(0) = narrow(points[0]);
    vertices->at(1) = narrow(points[1]);
    vertices->at(2) = narrow(points[2]);
    vertices->at(3) = narrow(points[3]);
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

    auto texture = vsg::DescriptorImage::create(sampler, image, 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

    auto projectiveUniform = ProjectiveUniformValue::create();
    projectiveUniform->properties.dataVariance = vsg::DataVariance::DYNAMIC_DATA;
    projectiveUniform->value().inverseCombo = vsg::mat4();
    auto projectiveUniformDescriptor = vsg::DescriptorBuffer::create(projectiveUniform, 1, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    auto descriptorSet = vsg::DescriptorSet::create(descriptorSetLayout, vsg::Descriptors{texture, projectiveUniformDescriptor});
    auto bindDescriptorSet = vsg::BindDescriptorSet::create(VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->layout, 0, descriptorSet);

    stateGroup = vsg::StateGroup::create();

    stateGroup->addChild(bindGraphicsPipeline);
    stateGroup->addChild(bindDescriptorSet);
    stateGroup->addChild(vid);

    addChild(stateGroup);
}

void MyQuad::update(const std::array<vsg::dvec3, 4>& points)
{
    using namespace vsg;
    vertices->at(0) = narrow(points[0]);
    vertices->at(1) = narrow(points[1]);
    vertices->at(2) = narrow(points[3]);
    vertices->at(3) = narrow(points[2]);
    vertices->dirty();
}

