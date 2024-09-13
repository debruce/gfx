#include "MyObject.h"

struct SetMyPipelineStates : public vsg::Visitor
{
    VkPrimitiveTopology topo;
    const vsg::StateInfo si;

    // explicit SetMyPipelineStates(const vsg::StateInfo& in) :
    //     si(in) {}

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

static vsg::vec3 calcNorm(const vsg::vec3& o, const vsg::vec3& a, const vsg::vec3& b)
{
    return vsg::normalize(vsg::cross(a - o, b - o));
}

static vsg::ref_ptr<vsg::VertexIndexDraw> makeVID(vsg::ref_ptr<vsg::vec3Array2D> mesh)
{
    using namespace vsg;

    auto vertices = vec3Array::create(4 * mesh->width() * mesh->height());
    auto& v = *vertices;

    auto normals = vec3Array::create(4 * mesh->width() * mesh->height());
    auto& n = *normals;

    auto texcoords = vec2Array::create(4 * mesh->width() * mesh->height());
    auto& t = *texcoords;

    auto indices = ushortArray::create(3 * 2 * mesh->width() * mesh->height());
    auto& ndcs = *indices;

    size_t vert = 0;
    size_t iindex = 0;

    for (size_t row = 0; row < mesh->width() - 1; row++) {
        for (size_t col = 0; col < mesh->height() - 1; col++) {
            auto p0 = (*mesh)(row, col);
            auto p1 = (*mesh)(row, col+1);
            auto p2 = (*mesh)(row+1, col);
            auto p3 = (*mesh)(row+1, col+1);
            auto norm = -calcNorm(p0, p1, p2);

            ndcs[iindex++] = vert;
            ndcs[iindex++] = vert + 2;
            ndcs[iindex++] = vert + 1;
            ndcs[iindex++] = vert + 1;
            ndcs[iindex++] = vert + 2;
            ndcs[iindex++] = vert + 3;

            v[vert] = p0;
            n[vert] = norm;
            t[vert] = vec2{0.0f, 0.0f};
            vert++;

            v[vert] = p1;
            n[vert] = norm;
            t[vert] = vec2{0.0f, 0.0f};
            vert++;

            v[vert] = p2;
            n[vert] = norm;
            t[vert] = vec2{0.0f, 0.0f}; 
            vert++;

            v[vert] = p3;
            n[vert] = norm;
            t[vert] = vec2{0.0f, 0.0f};
            vert++;
        }
    }

    auto vid = VertexIndexDraw::create();

    DataList arrays;
    arrays.push_back(vertices);
    arrays.push_back(normals);
    arrays.push_back(texcoords);
    // if (colors) arrays.push_back(colors);
    // if (positions) arrays.push_back(positions);
    vid->assignArrays(arrays);

    vid->assignIndices(indices);
    vid->indexCount = static_cast<uint32_t>(indices->size());
    vid->instanceCount = 1;

    return vid;
}

MyObject::MyObject(vsg::ref_ptr<const vsg::Options> options, vsg::ref_ptr<vsg::vec3Array2D> mesh)
{
    using namespace vsg;

    if (!_phongShaderSet) _phongShaderSet = createPhongShaderSet(options);
    
    auto vid = makeVID(mesh);

    auto gpConf = vsg::GraphicsPipelineConfigurator::create(_phongShaderSet);

    auto stateGroup = vsg::StateGroup::create();
    vsg::StateInfo si;
    SetMyPipelineStates sps(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, si);
    gpConf->accept(sps);
    gpConf->init();
    gpConf->copyTo(stateGroup);

    stateGroup->addChild(vid);

    addChild(stateGroup);
}
