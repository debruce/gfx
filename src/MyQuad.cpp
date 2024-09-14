#include "MyQuad.h"

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

MyQuad::MyQuad(vsg::ref_ptr<const vsg::Options> options, const vsg::vec3& a, const vsg::vec3& b, const vsg::vec3& c, const vsg::vec3& d)
{
    using namespace vsg;

    if (!_phongShaderSet) _phongShaderSet = createPhongShaderSet(options);

    vertices = vec3Array::create({a, b, c, d});
    vertices->properties.dataVariance = vsg::DYNAMIC_DATA;
    auto texcoords = vec2Array::create({vec2{0.0, 0.0}, vec2{1.0, 0.0}, vec2{1.0, 1.0}, vec2{0.0, 1.0}});
    auto normals = vec3Array::create({calcNorm(a, b, c)});
    auto colors = vec4Array::create({vec4{1.0, 1.0, 1.0, 1.0}});
    auto indices = ushortArray::create({0, 2, 1, 2, 3, 0});

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

    DataList arrays;
    arrays.push_back(vertices);
    arrays.push_back(normals);
    arrays.push_back(texcoords);
    arrays.push_back(colors);
    // if (positions) arrays.push_back(positions);

    auto vid = VertexIndexDraw::create();
    vid->assignArrays(arrays);
    vid->assignIndices(indices);
    vid->indexCount = static_cast<uint32_t>(indices->size());
    vid->instanceCount = 1;

    auto gpConf = vsg::GraphicsPipelineConfigurator::create(_phongShaderSet);

    stateGroup = vsg::StateGroup::create();

    auto sampler = Sampler::create();
    sampler->addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler->addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    gpConf->assignTexture("diffuseMap", image, sampler);
    // gpConf->enableTexture("diffuseMap");
    auto mat = vsg::PhongMaterialValue::create();
    mat->value().diffuse.set(1.0f, 1.0f, 1.0f, 1.0f);
    mat->value().specular.set(0.0f, 0.0f, 0.0f, 1.0f);
    gpConf->assignDescriptor("material", mat);

    vsg::StateInfo si;
    SetMyPipelineStates sps(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, si);
    gpConf->accept(sps);
    gpConf->init();


    gpConf->copyTo(stateGroup);

    stateGroup->addChild(vid);

    addChild(stateGroup);
}

void MyQuad::update(const vsg::dvec3& a, const vsg::dvec3& b, const vsg::dvec3& c, const vsg::dvec3& d)
{
    vertices->at(0) = a;
    vertices->at(1) = b;
    vertices->at(2) = c;
    vertices->at(3) = d;
    vertices->dirty();
}

