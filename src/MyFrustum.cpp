#include "MyFrustum.h"

#include <iostream>

struct FrustumParams
{
    vsg::mat4 inverseProj;
    vsg::vec4 color;
};

using FrustumParamsValue = vsg::Value<FrustumParams>;

static std::string VERT{R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants { mat4 projection; mat4 modelView; };

layout(location = 0) in vec3 vertex;

layout(set = 0, binding = 1) uniform FrustumParams 
{
    mat4 inverseProj;
    vec4 color;
}; 

out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out vec4 vertex_color;

void main()
{
    vec4 frustumVertex = inverseProj * vec4(vertex, 1.0);
    gl_Position = (projection * modelView) * frustumVertex;
    vertex_color = color;
}

)"};

static std::string FRAG{R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 vertex_color;
layout(location = 0) out vec4 color;

void main()
{
    color = vertex_color;
}

)"};

using namespace std;

MyFrustum::MyFrustum(vsg::ref_ptr<vsg::Perspective> proj, const double& sz) : proj(proj)
{
    using namespace vsg;

    auto vertexShader = ShaderStage::create(VK_SHADER_STAGE_VERTEX_BIT, "main", VERT);
    auto fragmentShader = ShaderStage::create(VK_SHADER_STAGE_FRAGMENT_BIT, "main", FRAG);
    if (!vertexShader || !fragmentShader)
    {
        std::cout << "Could not create shaders." << std::endl;
        exit(-1);
    }
    auto shaderSet = ShaderSet::create(ShaderStages{vertexShader, fragmentShader});
    
    PushConstantRanges pushConstantRanges{
        {VK_SHADER_STAGE_VERTEX_BIT, 0, 128} // projection and modelview matrices
    };

    VertexInputState::Bindings vertexBindingsDescriptions{
        VkVertexInputBindingDescription{0, 12, VK_VERTEX_INPUT_RATE_VERTEX}, // interleaved vertex, color, and texcoord data in a single array
    };

    vsg::VertexInputState::Attributes vertexAttributeDescriptions{
        VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},  // vertex data
    };

    GraphicsPipelineStates pipelineStates{
        VertexInputState::create(vertexBindingsDescriptions, vertexAttributeDescriptions),
        InputAssemblyState::create(VK_PRIMITIVE_TOPOLOGY_LINE_LIST, VK_FALSE),
        RasterizationState::create(),
        MultisampleState::create(),
        ColorBlendState::create(),
        DepthStencilState::create()};

    DescriptorSetLayoutBindings descriptorBindings{
        {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
    };

    auto descriptorSetLayout = DescriptorSetLayout::create(descriptorBindings);

    auto pipelineLayout = vsg::PipelineLayout::create(DescriptorSetLayouts{descriptorSetLayout}, pushConstantRanges);
    auto graphicsPipeline = GraphicsPipeline::create(pipelineLayout, vsg::ShaderStages{vertexShader, fragmentShader}, pipelineStates);
    auto bindGraphicsPipeline = vsg::BindGraphicsPipeline::create(graphicsPipeline);

    auto vertices = vec3Array::create({
        {-1.0, -1.0, 1.0 }, // small end vertices
        {+1.0, -1.0, 1.0 },
        {-1.0, +1.0, 1.0 },
        {+1.0, +1.0, 1.0 },

        {-1.0, -1.0, 0.0 }, // large end vertices
        {+1.0, -1.0, 0.0 },
        {-1.0, +1.0, 0.0 },
        {+1.0, +1.0, 0.0 },

        {-1.0, -1.0, 0.0625 },
        {+1.0, -1.0, 0.0625 },
        {-1.0, +1.0, 0.0625 },
        {+1.0, +1.0, 0.0625 },
    });

    auto indices = ushortArray::create({
        0, 4, 1, 5, 2, 6, 3, 7, // risers
        0, 1, 0, 2, 2, 3, 1, 3, // small end rect
        4, 5, 4, 6, 6, 7, 5, 7, // large end rect
        8, 9, 8, 10, 10, 11, 9, 11, 
    });

    DataList arrays;
    arrays.push_back(vertices);

    auto vid = VertexIndexDraw::create();
    vid->assignArrays(arrays);
    vid->assignIndices(indices);
    vid->indexCount = static_cast<uint32_t>(indices->size());
    vid->instanceCount = 1;

    auto frustumParams = FrustumParamsValue::create();
    // frustumParams->properties.dataVariance = DataVariance::DYNAMIC_DATA;
    frustumParams->value().inverseProj = inverse(Perspective::create(15.0, 1.5, .1, 20.0)->transform());
    frustumParams->value().color = vsg::vec4{1.0, 1.0, 0.0, 1.0};
    cout << frustumParams->value().inverseProj << endl;
    // frustumParams->value().inverseProj = mat4();
    auto frustumParamsDescriptor = DescriptorBuffer::create(frustumParams, 1, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    auto descriptorSet = DescriptorSet::create(descriptorSetLayout, Descriptors{frustumParamsDescriptor});
    auto bindDescriptorSet = BindDescriptorSet::create(VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->layout, 0, descriptorSet);

    addChild(bindGraphicsPipeline);
    addChild(bindDescriptorSet);
    addChild(vid);
}

// void MyQuad::update(const vsg::dvec3& a, const vsg::dvec3& b, const vsg::dvec3& c, const vsg::dvec3& d)
// {
//     vertices->at(0) = a;
//     vertices->at(1) = b;
//     vertices->at(2) = c;
//     vertices->at(3) = d;
//     vertices->dirty();
// }

