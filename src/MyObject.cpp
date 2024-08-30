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
        // if (si.two_sided)
        rs.cullMode = VK_CULL_MODE_NONE;
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

vsg::ref_ptr<vsg::StateGroup> generateMyObject(vsg::ref_ptr<vsg::vec2Array> curve, vsg::ref_ptr<vsg::Options> options, const vsg::StateInfo& si)
{
    using namespace vsg;

    const size_t square_count = 50;

    size_t row_count = curve->valueCount() - 1;

    auto vertices = vec3Array::create(4 * square_count * row_count);
    auto& v = *vertices;

    auto normals = vec3Array::create(4 * square_count * row_count);
    auto& n = *normals;

    auto texcoords = vec2Array::create(4 * square_count * row_count);
    auto& t = *texcoords;

    auto indices = ushortArray::create(3 * 2 * square_count * row_count);
    auto& ndcs = *indices;

    size_t vert = 0;
    size_t iindex = 0;

    double zScale = 1.0 / (*curve)[curve->valueCount()-1].x - (*curve)[0].x;

    for (size_t row = 0; row < row_count; row++) {
        using namespace std;
        auto z0 = (*curve)[row].x;
        auto r0 = (*curve)[row].y;
        auto z1 = (*curve)[row+1].x;
        auto r1 = (*curve)[row+1].y;

        for (size_t i = 0; i < square_count; i++) {
            float percent0 = float(i) / square_count;
            float percent1 = float(i+1) / square_count;
            float c0 = cosf(2 * M_PI * percent0);
            float s0 = sinf(2 * M_PI * percent0);
            float c1 = cosf(2 * M_PI * percent1);
            float s1 = sinf(2 * M_PI * percent1);
            auto p0 = vec3{c0 * r0, s0 * r0, z0};
            auto p1 = vec3{c0 * r1, s0 * r1, z1};
            auto p2 = vec3{c1 * r0, s1 * r0, z0};
            auto p3 = vec3{c1 * r1, s1 * r1, z1};
            auto norm = calcNorm(p0, p1, p2);

            float zCoord0 = (z0 - (*curve)[0].x) * zScale;
            float zCoord1 = (z1 - (*curve)[0].x) * zScale;            
            ndcs[iindex++] = vert;
            ndcs[iindex++] = vert + 1;
            ndcs[iindex++] = vert + 2;
            ndcs[iindex++] = vert + 1;
            ndcs[iindex++] = vert + 3;
            ndcs[iindex++] = vert + 2;

            v[vert] = p0;
            n[vert] = norm;
            t[vert] = vec2{percent0, zCoord0};
            vert++;

            v[vert] = p1;
            n[vert] = norm;
            t[vert] = vec2{percent1, zCoord0};
            vert++;

            v[vert] = p2;
            n[vert] = norm;
            t[vert] = vec2{percent0, zCoord1};
            vert++;

            v[vert] = p3;
            n[vert] = norm;
            t[vert] = vec2{percent1, zCoord1};
            vert++;
        }
    }

    auto shaderSet = createPhongShaderSet(options);
    auto gpConf = GraphicsPipelineConfigurator::create(shaderSet);

    SetMyPipelineStates sps(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, si);

    gpConf->accept(sps);

    DataList vertexArrays;
    gpConf->assignArray(vertexArrays, "vsg_Vertex", VK_VERTEX_INPUT_RATE_VERTEX, vertices);
    gpConf->assignArray(vertexArrays, "vsg_Normals", VK_VERTEX_INPUT_RATE_VERTEX, normals);
    gpConf->assignArray(vertexArrays, "vsg_TexCoord0", VK_VERTEX_INPUT_RATE_VERTEX, texcoords);
    gpConf->assignArray(vertexArrays, "vsg_Color", VK_VERTEX_INPUT_RATE_VERTEX, vec4Value::create(vec4{1.0f, 1.0f, 1.0f, 1.0f}));

    auto mat = PhongMaterialValue::create();

    gpConf->assignDescriptor("material", mat);
    gpConf->init();

    auto vid = VertexIndexDraw::create();
    vid->assignArrays(vertexArrays);
    vid->assignIndices(indices);
    vid->indexCount = 3 * 2 * square_count * row_count; // indices->valueCount();
    vid->instanceCount = 1;

    auto geomGroup = StateGroup::create();
    geomGroup->addChild(vid);
    // gpConf->assignDescriptor("color", vsg::vec4Array::create({vsg::vec4{1.0, .5, 0.0, 1.0}}));
    // gpConf->enableDescriptor("color");
    gpConf->copyTo(geomGroup);
    return geomGroup;
}
