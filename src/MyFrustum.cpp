#include "MyFrustum.h"

#include <iostream>

// struct FrustumParams
// {
//     vsg::mat4 inverseProj;
//     vsg::vec4 color;
// };

// using FrustumParamsValue = vsg::Value<FrustumParams>;

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

MyFrustum::MyFrustum(vsg::ref_ptr<vsg::Perspective> proj, const std::string& orientation)
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

        { 0.0, +0.75, 1.0 }, // small up arrow tip
        { 0.0, -0.75, 1.0 }, // small up arrow center
        {-0.5,  0.0,  1.0 }, // small up arrow left
        {+0.5,  0.0,  1.0 }, // small up arrow right

        { 0.0, +0.75, 0.0 }, // large up arrow tip
        { 0.0, -0.75, 0.0 }, // large up arrow center
        {-0.5,  0.0,  0.0 }, // large up arrow left
        {+0.5,  0.0,  0.0 }, // large up arrow right
    });

    auto indices = ushortArray::create({
        0, 4, 1, 5, 2, 6, 3, 7, // risers
        0, 1, 0, 2, 2, 3, 1, 3, // small end rect
        4, 5, 4, 6, 6, 7, 5, 7, // large end rect
        9, 8, 10, 8, 11, 8,     // small up arrow
        13, 12, 14, 12, 15, 12, // large up arrow
    });

    DataList arrays;
    arrays.push_back(vertices);

    auto vid = VertexIndexDraw::create();
    vid->assignArrays(arrays);
    vid->assignIndices(indices);
    vid->indexCount = static_cast<uint32_t>(indices->size());
    vid->instanceCount = 1;

    frustumParams = FrustumParamsValue::create();
    frustumParams->properties.dataVariance = DataVariance::DYNAMIC_DATA;
    inverseProj = inverse(proj->transform());
    frustumParams->value().inverseProj = inverseProj;
    frustumParams->value().color = vsg::vec4{1.0, 1.0, 0.0, 1.0};
    auto frustumParamsDescriptor = DescriptorBuffer::create(frustumParams, 1, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    auto descriptorSet = DescriptorSet::create(descriptorSetLayout, Descriptors{frustumParamsDescriptor});
    auto bindDescriptorSet = BindDescriptorSet::create(VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->layout, 0, descriptorSet);

    auto stateGroup = StateGroup::create();
    stateGroup->addChild(bindGraphicsPipeline);
    stateGroup->addChild(bindDescriptorSet);
    stateGroup->addChild(vid);

    addChild(stateGroup);

    if (orientation == "lookTowardPosY") {
        matrix = rotate(M_PI/2, vsg::dvec3{1.0, 0.0, 0.0}) * rotate(M_PI, vsg::dvec3{0.0, 0.0, 1.0});
    }
    if (orientation == "lookTowardNegY") {
        matrix = rotate(-M_PI/2, vsg::dvec3{1.0, 0.0, 0.0});
    }
    else if (orientation == "lookTowardPosX") {
        matrix = rotate(-M_PI/2, vsg::dvec3{0.0, 1.0, 0.0}) * rotate(M_PI/2, vsg::dvec3{0.0, 0.0, 1.0});
    }
    else if (orientation == "lookTowardNegX") {
        matrix = rotate(M_PI/2, vsg::dvec3{0.0, 1.0, 0.0}) * rotate(-M_PI/2, vsg::dvec3{0.0, 0.0, 1.0});
    }
}

void MyFrustum::update(vsg::ref_ptr<vsg::Perspective> proj)
{
    inverseProj = inverse(proj->transform());
    frustumParams->value().inverseProj = inverseProj;
    frustumParams->dirty();
}

vsg::dvec3 hnorm(const vsg::dvec4 vec)
{
    return vsg::dvec3{vec.x/vec.w, vec.y/vec.w, vec.z/vec.w};
}

std::array<vsg::dvec3, 4> MyFrustum::getZIntercept(const vsg::dmat4& modelView)
{
    using namespace vsg;

    std::array<dvec3, 4> results;

    std::array<dvec4, 8> rawCube = {
        dvec4{-1.0, -1.0, 1.0, 1.0 }, // small end vertices
        dvec4{+1.0, -1.0, 1.0, 1.0 },
        dvec4{-1.0, +1.0, 1.0, 1.0 },
        dvec4{+1.0, +1.0, 1.0, 1.0 },
        dvec4{-1.0, -1.0, 0.0, 1.0 }, // large end vertices
        dvec4{+1.0, -1.0, 0.0, 1.0 },
        dvec4{-1.0, +1.0, 0.0, 1.0 },
        dvec4{+1.0, +1.0, 0.0, 1.0 },
    };

    auto m = modelView * inverseProj;
    for (auto i = 0; i < 4; i++) {
        auto near = hnorm(m * rawCube[i]);
        auto far = hnorm(m * rawCube[i+4]);
        auto diff = far - near;
        auto t = -near.z / diff.z;
        results[i] = near + diff * t;
    }
    return results;
}
