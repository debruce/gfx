#include <vsg/all.h>
#include <iostream>
#include <iomanip>
#include <sstream>

struct SetMyPipelineStates : public vsg::Visitor
{
    VkPrimitiveTopology topo;

    SetMyPipelineStates(const VkPrimitiveTopology& topo) : topo(topo) {}

    void apply(vsg::Object& object) { object.traverse(*this); }
    void apply(vsg::RasterizationState& rs)
    {
        rs.lineWidth = 1.0;
        rs.cullMode = VK_CULL_MODE_NONE;
    }
    void apply(vsg::InputAssemblyState& ias)
    {
        ias.topology = topo;
    }
};

using namespace std;

vsg::vec3 calcNorm(const vsg::vec3& o, const vsg::vec3& a, const vsg::vec3& b)
{
    return vsg::normalize(vsg::cross(a - o, b - o));
}

vsg::ref_ptr<vsg::StateGroup> generateMyObject(vsg::ref_ptr<vsg::Options> options)
{
    auto vertexShader = vsg::read_cast<vsg::ShaderStage>("../shaders/myShader.vert", options);
    auto fragmentShader = vsg::read_cast<vsg::ShaderStage>("../shaders/myShader.frag", options);
    auto shaderSet = vsg::ShaderSet::create(vsg::ShaderStages{vertexShader, fragmentShader});

    shaderSet->addAttributeBinding("vsg_Vertex", "", 0, VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));
    shaderSet->addAttributeBinding("vsg_Normal", "", 1, VK_FORMAT_R32G32B32_SFLOAT, vsg::vec3Array::create(1));
    shaderSet->addAttributeBinding("vsg_TexCoord0", "", 2, VK_FORMAT_R32G32_SFLOAT, vsg::vec2Array::create(1));
    shaderSet->addAttributeBinding("vsg_Color", "", 3, VK_FORMAT_R32G32B32A32_SFLOAT, vsg::vec4Array::create(1));

    shaderSet->addPushConstantRange("pc", "", VK_SHADER_STAGE_VERTEX_BIT, 0, 128);

    auto gpConf = vsg::GraphicsPipelineConfigurator::create(shaderSet);

    SetMyPipelineStates sps(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    gpConf->accept(sps);
    gpConf->init();

    const size_t triangle_count = 2;

    auto vertices_ptr = vsg::vec3Array::create(3 * triangle_count);
    auto& vertices = *vertices_ptr;

    auto normals_ptr = vsg::vec3Array::create(3 * triangle_count);
    auto& normals = *normals_ptr;

    auto texCoord_ptr = vsg::vec2Array::create(3 * triangle_count);
    auto& texCoord = *texCoord_ptr;

    auto colors_ptr = vsg::vec4Array::create(3 * triangle_count);
    auto& colors = *colors_ptr;

    vertices[0] = vsg::vec3{0.0, 0.0, 0.0};
    vertices[1] = vsg::vec3{0.0, 0.0, 1.0};
    vertices[2] = vsg::vec3{1.0, 1.0, 0.0};

    auto norm = calcNorm(vertices[0], vertices[1], vertices[2]);
    normals[0] = norm;
    normals[1] = norm;
    normals[2] = norm;

    texCoord[0] = vsg::vec2{0.0, 0.0};
    texCoord[1] = vsg::vec2{0.0, 1.0};
    texCoord[2] = vsg::vec2{1.0, 0.0};

    colors[0] = vsg::vec4{1.0, 1.0, 1.0, 1.0};
    colors[1] = vsg::vec4{1.0, 1.0, 1.0, 1.0};
    colors[2] = vsg::vec4{1.0, 1.0, 1.0, 1.0};

    vertices[3] = vsg::vec3{0.0, 0.0, 1.0};
    vertices[4] = vsg::vec3{1.0, 1.0, 0.0};
    vertices[5] = vsg::vec3{1.0, 1.0, 1.0};

    norm = calcNorm(vertices[3], vertices[4], vertices[5]);
    normals[3] = norm;
    normals[4] = norm;
    normals[5] = norm;

    texCoord[3] = vsg::vec2{0.0, 1.0};
    texCoord[4] = vsg::vec2{1.0, 0.0};
    texCoord[5] = vsg::vec2{1.0, 1.0};

    colors[3] = vsg::vec4{1.0, 1.0, 1.0, 1.0};
    colors[4] = vsg::vec4{1.0, 1.0, 1.0, 1.0};
    colors[5] = vsg::vec4{1.0, 1.0, 1.0, 1.0};


    // auto angle_step = M_PI / triangle_count;
    // float z0 = 0.0;
    // float z1 = 1.5;
    // // auto clr = vsg::vec4{1.0, 1.0, 1.0, 1.0};
    // for (size_t i = 0; i < triangle_count; i += 2) {
    //     float percent = float(i) / triangle_count;
    //     auto clr = vsg::vec4{percent, 0.0, 1-percent, 1.0};
    //     float angle = i * M_PI / triangle_count;
    //     float x0 = cos(angle) * 10.0;
    //     float y0 = sin(angle) + 10.0;
    //     float x1 = cos(angle + angle_step) * 10.0;
    //     float y1 = sin(angle + angle_step) + 10.0;

    //     vertices[i*3] = vsg::vec3{x0, y0, z0};
    //     vertices[i*3+1] = vsg::vec3{x0, y0, z1};
    //     vertices[i*3+2] = vsg::vec3{x1, y1, z0};

    //     auto norm = vsg::normalize(vsg::cross(vertices[i*3+1] - vertices[i*3], vertices[i*3+2] - vertices[i*3]));
    //     normals[i*3] = norm;
    //     normals[i*3+1] = norm;
    //     normals[i*3+2] = norm;

    //     texCoord[i*3] = vsg::vec2{float(i)/triangle_count, 0.0};
    //     texCoord[i*3+1] = vsg::vec2{float(i)/triangle_count, 1.0};
    //     texCoord[i*3+2] = vsg::vec2{float(i+1)/triangle_count, 0.0};

    //     colors[i*3] = clr;
    //     colors[i*3+1] = clr;
    //     colors[i*3+2] = clr;

    //     vertices[(i+1)*3] = vsg::vec3{x0, y0, z0};
    //     vertices[(i+1)*3+1] = vsg::vec3{x0, y0, z1};
    //     vertices[(i+1)*3+2] = vsg::vec3{x1, y1, z0};

    //     norm = vsg::normalize(vsg::cross(vertices[(i+1)*3+1] - vertices[(i+1)*3], vertices[(i+1)*3+2] - vertices[(i+1)*3]));
    //     normals[(i+1)*3] = norm;
    //     normals[(i+1)*3+1] = norm;
    //     normals[(i+1)*3+2] = norm;

    //     texCoord[i*3] = vsg::vec2{float(i)/triangle_count, 0.0};
    //     texCoord[i*3+1] = vsg::vec2{float(i)/triangle_count, 1.0};
    //     texCoord[i*3+2] = vsg::vec2{float(i+1)/triangle_count, 0.0};

    //     colors[(i+1)*3] = clr;
    //     colors[(i+1)*3+1] = clr;
    //     colors[(i+1)*3+2] = clr;
    // }


    vsg::DataList vertexArrays;
    gpConf->assignArray(vertexArrays, "vsg_Vertex", VK_VERTEX_INPUT_RATE_VERTEX, vertices_ptr);
    gpConf->assignArray(vertexArrays, "vsg_Normals", VK_VERTEX_INPUT_RATE_VERTEX, normals_ptr);
    gpConf->assignArray(vertexArrays, "vsg_TexCoord0", VK_VERTEX_INPUT_RATE_VERTEX, texCoord_ptr);
    gpConf->assignArray(vertexArrays, "vsg_Color", VK_VERTEX_INPUT_RATE_VERTEX, colors_ptr);
    auto vertexDraw = vsg::VertexDraw::create();
    vertexDraw->assignArrays(vertexArrays);
    vertexDraw->vertexCount = vertices_ptr->width();
    cout << "width = " << vertices_ptr->width() << endl;
    vertexDraw->instanceCount = 1;

    auto geomGroup = vsg::StateGroup::create();
    geomGroup->addChild(vertexDraw);
    // gpConf->assignDescriptor("color", vsg::vec4Array::create({vsg::vec4{1.0, .5, 0.0, 1.0}}));
    // gpConf->enableDescriptor("color");
    gpConf->copyTo(geomGroup);
    return geomGroup;
}
