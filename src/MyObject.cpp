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

    const size_t square_count = 10;
    const size_t triangle_count = square_count * 2;

    auto vertices_ptr = vsg::vec3Array2D::create(3, triangle_count);
    auto& vertices = *vertices_ptr;

    auto normals_ptr = vsg::vec3Array2D::create(3, triangle_count);
    auto& normals = *normals_ptr;

    auto texCoord_ptr = vsg::vec2Array2D::create(3, triangle_count);
    auto& texCoord = *texCoord_ptr;

    auto colors_ptr = vsg::vec4Array2D::create(3, triangle_count);
    auto& colors = *colors_ptr;

    auto angle_step = 2 * M_PI / square_count;
    float z0 = 0.0;
    float z1 = 1.5;
    size_t v_index = 0;

    for (size_t i = 0; i < square_count; i++) {
        float percent = float(i) / square_count;
        auto clr = vsg::vec4{percent, 0.0, 1-percent, 1.0};
        float angle = 2 * M_PI * percent;
        float x0 = cos(angle) * 5.0;
        float y0 = sin(angle) * 5.0;
        float x1 = cos(angle + angle_step) * 5.0;
        float y1 = sin(angle + angle_step) * 5.0;

        vertices(0,v_index) = vsg::vec3{x0, y0, z0};
        vertices(1,v_index) = vsg::vec3{x0, y0, z1};
        vertices(2,v_index) = vsg::vec3{x1, y1, z0};

        auto norm = calcNorm(vertices(0,v_index), vertices(1,v_index), vertices(2,v_index));
        normals(0,v_index) = norm;
        normals(1,v_index) = norm;
        normals(2,v_index) = norm;

        texCoord(0,v_index) = vsg::vec2{float(i)/triangle_count, 0.0};
        texCoord(1,v_index) = vsg::vec2{float(i)/triangle_count, 1.0};
        texCoord(2,v_index) = vsg::vec2{float(i+1)/triangle_count, 0.0};

        colors(0,v_index) = clr;
        colors(1,v_index) = clr;
        colors(2,v_index) = clr;
        v_index++;
    }

    for (size_t i = 0; i < square_count; i++) {
        float percent = float(i) / square_count;
        auto clr = vsg::vec4{percent, 0.0, 1-percent, 1.0};
        float angle = 2 * M_PI * percent;
        float x0 = cos(angle) * 5.0;
        float y0 = sin(angle) * 5.0;
        float x1 = cos(angle + angle_step) * 5.0;
        float y1 = sin(angle + angle_step) * 5.0;

        vertices(0,v_index) = vsg::vec3{x0, y0, z1};
        vertices(1,v_index) = vsg::vec3{x1, y1, z0};
        vertices(2,v_index) = vsg::vec3{x1, y1, z1};

        auto norm = calcNorm(vertices(0,v_index), vertices(1,v_index), vertices(2,v_index));
        normals(0,v_index) = norm;
        normals(1,v_index) = norm;
        normals(2, v_index) = norm;

        texCoord(0, v_index) = vsg::vec2{float(i)/triangle_count, 0.0};
        texCoord(1, v_index) = vsg::vec2{float(i)/triangle_count, 1.0};
        texCoord(2, v_index) = vsg::vec2{float(i+1)/triangle_count, 0.0};

        colors(0, v_index)= clr;
        colors(1, v_index) = clr;
        colors(2, v_index) = clr;
        v_index++;
    }

    vsg::DataList vertexArrays;
    gpConf->assignArray(vertexArrays, "vsg_Vertex", VK_VERTEX_INPUT_RATE_VERTEX, vertices_ptr);
    gpConf->assignArray(vertexArrays, "vsg_Normals", VK_VERTEX_INPUT_RATE_VERTEX, normals_ptr);
    gpConf->assignArray(vertexArrays, "vsg_TexCoord0", VK_VERTEX_INPUT_RATE_VERTEX, texCoord_ptr);
    gpConf->assignArray(vertexArrays, "vsg_Color", VK_VERTEX_INPUT_RATE_VERTEX, colors_ptr);
    auto vertexDraw = vsg::VertexDraw::create();
    vertexDraw->assignArrays(vertexArrays);
    vertexDraw->vertexCount = vertices.valueCount();
    cout << "width = " << vertices.valueCount() << endl;
    vertexDraw->instanceCount = 1;

    auto geomGroup = vsg::StateGroup::create();
    geomGroup->addChild(vertexDraw);
    // gpConf->assignDescriptor("color", vsg::vec4Array::create({vsg::vec4{1.0, .5, 0.0, 1.0}}));
    // gpConf->enableDescriptor("color");
    gpConf->copyTo(geomGroup);
    return geomGroup;
}
