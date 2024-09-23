#include "MyQuad.h"

#include <iostream>

static std::string VERT{R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants { mat4 projection; mat4 modelView; };

layout(location = 0) in vec3 vertex;

out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out vec2 world;

void main()
{
    world = vec2(vertex.x, vertex.y);
    gl_Position = projection * modelView * vec4(vertex, 1.0);
}

)"};

static std::string FRAG{R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 world;
layout(location = 0) out vec4 color;
layout(set = 0, binding = 0) uniform sampler2D texSampler;

layout(set = 0, binding = 1) uniform ProjectiveParams 
{
    mat4 inverseCombo;
}; 

vec4 pattern(vec2 val)
{
    return vec4(mod(val.x*8.0, 1.0), 0.0, mod(val.y*8.0, 1.0), 1.0);
    // return vec4(val.x, 0.0, val.y, 1.0);
}


void main()
{
    vec4 hCoord = (inverseCombo * vec4(world, 0.0, 1.0));
    vec2 lookupCoord = vec2(hCoord.x/hCoord.w, hCoord.y/hCoord.w) * vec2(.5, .5) + vec2(.5, .5);
    color = texture(texSampler, lookupCoord);
    // color = pattern(lookupCoord);
    // color = vec4(lookupCoord.x, 0.0, lookupCoord.y, 1.0);
    // color = vec4(world.x, world.y, 0.0, 1.0);
}

)"};

using namespace std;

vsg::vec3 narrow(const vsg::dvec3& in)
{
    return vsg::vec3{float(in.x), float(in.y), float(in.z)};
}

MyQuad::MyQuad(vsg::ref_ptr<vsg::Options> options)
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
    vertices->properties.dataVariance = vsg::DYNAMIC_DATA;
    auto indices = ushortArray::create({0, 2, 1, 0, 3, 2});

    DataList arrays;
    arrays.push_back(vertices);

    auto vid = VertexIndexDraw::create();
    vid->assignArrays(arrays);
    vid->assignIndices(indices);
    vid->indexCount = static_cast<uint32_t>(indices->size());
    vid->instanceCount = 1;

    auto image = vsg::read_cast<vsg::Data>("../IMG_2791.jpg", options);
    image->properties.dataVariance = vsg::DYNAMIC_DATA;
    image->dirty();

    auto sampler = vsg::Sampler::create();
    sampler->addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler->addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

    auto texture = vsg::DescriptorImage::create(sampler, image, 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

    projectiveUniform = ProjectiveUniformValue::create();
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

void MyQuad::update(vsg::ref_ptr<MyFrustum> frustum)
{
    using namespace vsg;
    vertices->at(0) = narrow(frustum->corners[0]);
    vertices->at(1) = narrow(frustum->corners[1]);
    vertices->at(2) = narrow(frustum->corners[3]);
    vertices->at(3) = narrow(frustum->corners[2]);
    vertices->dirty();

    dmat4 m = inverse(frustum->inverseProj)
        * inverse(frustum->transform()
        * rotate(-M_PI/2.0, dvec3{1.0, 0.0, 0.0}))
        ;
    for (size_t i = 0; i < 4; i++) {
        for (size_t j = 0; j < 4; j++) {
            projectiveUniform->value().inverseCombo[i][j] = float(m[i][j]);
        }
    }
    projectiveUniform->dirty();
}

