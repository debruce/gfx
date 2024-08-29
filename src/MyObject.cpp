#include <vsg/all.h>
#include <iostream>
#include <iomanip>
#include <sstream>

// vsg::ref_ptr<vsg::ShaderSet> pbr_ShaderSet(vsg::ref_ptr<const vsg::Options> options)
// {
//     vsg::info("Local pbr_ShaderSet(", options, ")");

//     auto vertexShader = vsg::read_cast<vsg::ShaderStage>("shaders/standard.vert", options);
//     auto fragmentShader = vsg::read_cast<vsg::ShaderStage>("shaders/standard_pbr.frag", options);

//     if (!vertexShader || !fragmentShader)
//     {
//         vsg::error("pbr_ShaderSet(...) could not find shaders.");
//         return {};
//     }

//     auto shaderSet = vsg::ShaderSet::create(vsg::ShaderStages{vertexShader, fragmentShader});

// #define VIEW_DESCRIPTOR_SET 0
// #define MATERIAL_DESCRIPTOR_SET 1

//     shaderSet->addAttributeBinding("vsg_Vertex", "", 0, VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));
//     shaderSet->addAttributeBinding("vsg_Normal", "", 1, VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));
//     shaderSet->addAttributeBinding("vsg_TexCoord0", "", 2, VK_FORMAT_R32G32_SFLOAT, vsg::vec2Array::create(1));
//     shaderSet->addAttributeBinding("vsg_Color", "", 3, VK_FORMAT_R32G32B32A32_SFLOAT, vsg::vec4Array::create(1));

//     shaderSet->addAttributeBinding("vsg_position", "VSG_INSTANCE_POSITIONS", 4, VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));
//     shaderSet->addAttributeBinding("vsg_position_scaleDistance", "VSG_BILLBOARD", 4, VK_FORMAT_R32G32B32A32_SFLOAT, vsg::vec4Array::create(1));

//     shaderSet->addAttributeBinding("vsg_JointIndices", "VSG_SKINNING", 5, VK_FORMAT_R32G32B32A32_SINT, vsg::ivec4Array::create(1));
//     shaderSet->addAttributeBinding("vsg_JointWeights", "VSG_SKINNING", 6, VK_FORMAT_R32G32B32A32_SFLOAT, vsg::vec4Array::create(1));

//     shaderSet->addDescriptorBinding("displacementMap", "VSG_DISPLACEMENT_MAP", MATERIAL_DESCRIPTOR_SET, 6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_VERTEX_BIT, vsg::floatArray2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R32_SFLOAT}));
//     shaderSet->addDescriptorBinding("diffuseMap", "VSG_DIFFUSE_MAP", MATERIAL_DESCRIPTOR_SET, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::ubvec4Array2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R8G8B8A8_UNORM}));
//     shaderSet->addDescriptorBinding("mrMap", "VSG_METALLROUGHNESS_MAP", MATERIAL_DESCRIPTOR_SET, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec2Array2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R32G32_SFLOAT}));
//     shaderSet->addDescriptorBinding("normalMap", "VSG_NORMAL_MAP", MATERIAL_DESCRIPTOR_SET, 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec3Array2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R32G32B32_SFLOAT}));
//     shaderSet->addDescriptorBinding("aoMap", "VSG_LIGHTMAP_MAP", MATERIAL_DESCRIPTOR_SET, 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::floatArray2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R32_SFLOAT}));
//     shaderSet->addDescriptorBinding("emissiveMap", "VSG_EMISSIVE_MAP", MATERIAL_DESCRIPTOR_SET, 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::ubvec4Array2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R8G8B8A8_UNORM}));
//     shaderSet->addDescriptorBinding("specularMap", "VSG_SPECULAR_MAP", MATERIAL_DESCRIPTOR_SET, 5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::ubvec4Array2D::create(1, 1, vsg::Data::Properties{VK_FORMAT_R8G8B8A8_UNORM}));
//     shaderSet->addDescriptorBinding("material", "", MATERIAL_DESCRIPTOR_SET, 10, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::PbrMaterialValue::create());

//     shaderSet->addDescriptorBinding("jointMatrices", "VSG_SKINNING", MATERIAL_DESCRIPTOR_SET, 11, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, vsg::mat4Value::create());

//     shaderSet->addDescriptorBinding("lightData", "", VIEW_DESCRIPTOR_SET, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec4Array::create(64));
//     shaderSet->addDescriptorBinding("viewportData", "", VIEW_DESCRIPTOR_SET, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, vsg::vec4Value::create(0, 0, 1280, 1024));
//     shaderSet->addDescriptorBinding("shadowMaps", "", VIEW_DESCRIPTOR_SET, 2, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, vsg::floatArray3D::create(1, 1, 1, vsg::Data::Properties{VK_FORMAT_R32_SFLOAT}));
//     shaderSet->addDescriptorBinding("shadowMapDirectSampler", "VSG_SHADOWS_PCSS", VIEW_DESCRIPTOR_SET, 3, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);
//     shaderSet->addDescriptorBinding("shadowMapShadowSampler", "", VIEW_DESCRIPTOR_SET, 4, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);

//     // additional defines
//     shaderSet->optionalDefines = {"VSG_GREYSCALE_DIFFUSE_MAP", "VSG_TWO_SIDED_LIGHTING", "VSG_WORKFLOW_SPECGLOSS", "VSG_SHADOWS_PCSS", "VSG_SHADOWS_SOFT", "VSG_SHADOWS_HARD", "SHADOWMAP_DEBUG", "VSG_ALPHA_TEST"};

//     shaderSet->addPushConstantRange("pc", "", VK_SHADER_STAGE_VERTEX_BIT, 0, 128);

//     shaderSet->definesArrayStates.push_back(vsg::DefinesArrayState{{"VSG_INSTANCE_POSITIONS", "VSG_DISPLACEMENT_MAP"}, vsg::PositionAndDisplacementMapArrayState::create()});
//     shaderSet->definesArrayStates.push_back(vsg::DefinesArrayState{{"VSG_INSTANCE_POSITIONS"}, vsg::PositionArrayState::create()});
//     shaderSet->definesArrayStates.push_back(vsg::DefinesArrayState{{"VSG_DISPLACEMENT_MAP"}, vsg::DisplacementMapArrayState::create()});
//     shaderSet->definesArrayStates.push_back(vsg::DefinesArrayState{{"VSG_BILLBOARD"}, vsg::BillboardArrayState::create()});

//     shaderSet->customDescriptorSetBindings.push_back(vsg::ViewDependentStateBinding::create(VIEW_DESCRIPTOR_SET));

//     return shaderSet;
// }

struct SetMyPipelineStates : public vsg::Visitor
{
    VkPrimitiveTopology topo;
    const vsg::StateInfo si;

    explicit SetMyPipelineStates(const vsg::StateInfo& in) :
        si(in) {}

    SetMyPipelineStates(const VkPrimitiveTopology& topo, const vsg::StateInfo& si) : topo(topo), si(si){}

    void apply(vsg::Object& object) override
    {
        object.traverse(*this);
    }

    void apply(vsg::RasterizationState& rs) override
    {
        if (si.two_sided) rs.cullMode = VK_CULL_MODE_NONE;
    }

    void apply(vsg::InputAssemblyState& ias) override
    {
        if (si.wireframe) ias.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        else ias.topology = topo;
    }

    void apply(vsg::ColorBlendState& cbs) override
    {
        cbs.configureAttachments(si.blending);
    }

};

using namespace std;

vsg::vec3 calcNorm(const vsg::vec3& o, const vsg::vec3& a, const vsg::vec3& b)
{
    return vsg::normalize(vsg::cross(a - o, b - o));
}

vsg::ref_ptr<vsg::StateGroup> generateMyObject(vsg::ref_ptr<vsg::Options> options, const vsg::StateInfo& si)
{
    // auto shaderSet = pbr_ShaderSet(options);
    auto shaderSet = vsg::createPhongShaderSet(options);
    auto gpConf = vsg::GraphicsPipelineConfigurator::create(shaderSet);

    SetMyPipelineStates sps(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, si);

    gpConf->accept(sps);

    const size_t square_count = 50;
    const size_t triangle_count = square_count * 2;

    auto vertices_ptr = vsg::vec3Array2D::create(3, triangle_count);
    auto& vertices = *vertices_ptr;

    auto normals_ptr = vsg::vec3Array2D::create(3, triangle_count);
    auto& normals = *normals_ptr;

    auto texCoord_ptr = vsg::vec2Array2D::create(3, triangle_count);
    auto& texCoord = *texCoord_ptr;

    auto angle_step = 2 * M_PI / square_count;
    float z0 = 0.0;
    float z1 = 1.5;
    size_t v_index = 0;

    for (size_t i = 0; i < square_count; i++) {
        float percent0 = float(i) / square_count;
        float percent2 = float(i+1) / square_count;
        float angle = 2 * M_PI * percent0;
        float x0 = cos(angle) * 5.0;
        float y0 = sin(angle) * 5.0;
        float x1 = cos(angle + angle_step) * 5.0;
        float y1 = sin(angle + angle_step) * 5.0;

        auto p0 = vsg::vec3{x0, y0, z0};
        auto p1 = vsg::vec3{x0, y0, z1};
        auto p2 = vsg::vec3{x1, y1, z0};
        auto p3 = vsg::vec3{x1, y1, z1};
        auto norm = calcNorm(p0, p1, p2);

        vertices(0,v_index) = p0;
        vertices(1,v_index) = p1;
        vertices(2,v_index) = p2;

        normals(0,v_index) = -norm;
        normals(1,v_index) = -norm;
        normals(2,v_index) = -norm;

        texCoord(0,v_index) = vsg::vec2{percent0, 0.0};
        texCoord(1,v_index) = vsg::vec2{percent0, 1.0};
        texCoord(2,v_index) = vsg::vec2{percent2, 0.0};

        v_index++;

        vertices(0,v_index) = p1;
        vertices(1,v_index) = p2;
        vertices(2,v_index) = p3;

        normals(0,v_index) = norm;
        normals(1,v_index) = norm;
        normals(2, v_index) = norm;

        texCoord(0, v_index) = vsg::vec2{percent0, 1.0};
        texCoord(1, v_index) = vsg::vec2{percent2, 0.0};
        texCoord(2, v_index) = vsg::vec2{percent2, 1.0};

        v_index++;
    }

    vsg::DataList vertexArrays;
    gpConf->assignArray(vertexArrays, "vsg_Vertex", VK_VERTEX_INPUT_RATE_VERTEX, vertices_ptr);
    gpConf->assignArray(vertexArrays, "vsg_Normals", VK_VERTEX_INPUT_RATE_VERTEX, normals_ptr);
    gpConf->assignArray(vertexArrays, "vsg_TexCoord0", VK_VERTEX_INPUT_RATE_VERTEX, texCoord_ptr);
    gpConf->assignArray(vertexArrays, "vsg_Color", VK_VERTEX_INPUT_RATE_VERTEX, vsg::vec4Value::create(vsg::vec4{1.0f, 1.0f, 1.0f, 1.0f}));

    // auto mat = vsg::PbrMaterialValue::create();
    // mat->value().baseColorFactor.set(1.0f, 0.0f, 0.0f, 1.0f);
    // mat->value().diffuseFactor.set(0.0f, 1.0f, 0.0f, 1.0f);
    // mat->value().specular.set(1.0f, 0.0f, 0.0f, 1.0f); // red specular highlight

    auto mat = vsg::PhongMaterialValue::create();

    gpConf->assignDescriptor("material", mat);
    gpConf->init();

    auto vertexDraw = vsg::VertexDraw::create();
    vertexDraw->assignArrays(vertexArrays);
    vertexDraw->vertexCount = vertices.valueCount();
    vertexDraw->instanceCount = 1;

    auto geomGroup = vsg::StateGroup::create();
    geomGroup->addChild(vertexDraw);
    // gpConf->assignDescriptor("color", vsg::vec4Array::create({vsg::vec4{1.0, .5, 0.0, 1.0}}));
    // gpConf->enableDescriptor("color");
    gpConf->copyTo(geomGroup);
    return geomGroup;
}
