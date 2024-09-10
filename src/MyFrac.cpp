#include <vsg/all.h>
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;

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

vsg::ref_ptr<vsg::StateGroup> generateMyFrac(vsg::ref_ptr<vsg::Options> options)
{
    using namespace vsg;

    auto vertices = vec3Array::create({
        { -1.0f, -1.0f, 0.0f },
        { 1.0f, -1.0f, 0.0f },
        { -1.0f, 1.0f, 0.0f },
        { 1.0f, 1.0f, 0.0f }
    });

    // auto normals = vec3Array::create({
    //     { 0.0f, 0.0, 1.0f },
    //     { 0.0f, 0.0, 1.0f },
    //     { 0.0f, 0.0, 1.0f },
    //     { 0.0f, 0.0, 1.0f }
    // });

    // auto texcoords = vec2Array::create({
    //     { 0.0f, 0.0f },
    //     { 1.0f, 0.0f },
    //     { 0.0f, 1.0f },
    //     { 1.0f, 1.0f },
    // });

    auto indices = ushortArray::create({
        0, 1, 2,
        2, 1, 3
    });

    DataList vertexArrays;
    vertexArrays.push_back(vertices);
    // vertexArrays.push_back(normals);
    // vertexArrays.push_back(texcoords);

    auto vid = VertexIndexDraw::create();
    vid->assignArrays(vertexArrays);
    vid->assignIndices(indices);
    vid->indexCount = indices->valueCount();
    vid->instanceCount = 1;

    auto vertexShader = vsg::ShaderStage::create(VK_SHADER_STAGE_VERTEX_BIT, "main", VERT);
    auto fragmentShader = vsg::ShaderStage::create(VK_SHADER_STAGE_FRAGMENT_BIT, "main", FRAG);
    if (!vertexShader || !fragmentShader)
    {
        std::cout << "Could not create shaders." << std::endl;
        exit(-1);
    }
    auto shaderSet = vsg::ShaderSet::create(vsg::ShaderStages{vertexShader, fragmentShader});

    // set up graphics pipeline
    // vsg::DescriptorSetLayoutBindings descriptorBindings{
    //     // {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr} // { binding, descriptorType, descriptorCount, stageFlags, pImmutableSamplers}
    // };

    // auto descriptorSetLayout = vsg::DescriptorSetLayout::create(descriptorBindings);

    vsg::PushConstantRanges pushConstantRanges{
        {VK_SHADER_STAGE_VERTEX_BIT, 0, 128} // projection, view, and model matrices, actual push constant calls automatically provided by the VSG's RecordTraversal
    };

    vsg::VertexInputState::Bindings vertexBindingsDescriptions{
        VkVertexInputBindingDescription{0, 12, VK_VERTEX_INPUT_RATE_VERTEX}, // interleaved vertex, color, and texcoord data in a single array
    };

    vsg::VertexInputState::Attributes vertexAttributeDescriptions{
        VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},  // vertex data
        // VkVertexInputAttributeDescription{1, 0, VK_FORMAT_R32G32B32_SFLOAT, 12}, // colour data
        // VkVertexInputAttributeDescription{2, 0, VK_FORMAT_R32G32_SFLOAT, 24},    // tex coord data
    };

    vsg::GraphicsPipelineStates pipelineStates{
        vsg::VertexInputState::create(vertexBindingsDescriptions, vertexAttributeDescriptions),
        vsg::InputAssemblyState::create(),
        vsg::RasterizationState::create(),
        vsg::MultisampleState::create(),
        vsg::ColorBlendState::create(),
        vsg::DepthStencilState::create()};

    // auto pipelineLayout = vsg::PipelineLayout::create(vsg::DescriptorSetLayouts{descriptorSetLayout}, pushConstantRanges);
    auto pipelineLayout = vsg::PipelineLayout::create(vsg::DescriptorSetLayouts{}, pushConstantRanges);
    auto graphicsPipeline = vsg::GraphicsPipeline::create(pipelineLayout, vsg::ShaderStages{vertexShader, fragmentShader}, pipelineStates);
    auto bindGraphicsPipeline = vsg::BindGraphicsPipeline::create(graphicsPipeline);

    // auto descriptorSet = vsg::DescriptorSet::create(descriptorSetLayout, vsg::Descriptors{});
    // auto bindDescriptorSet = vsg::BindDescriptorSet::create(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, descriptorSet);

    auto geomGroup = StateGroup::create();
    geomGroup->addChild(vid);
    geomGroup->add(bindGraphicsPipeline);
    // geomGroup->add(bindDescriptorSet);
    return geomGroup;
}
