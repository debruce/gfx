#include "MyBuilder.h"

vsg::ref_ptr<vsg::Node> MyBuilder::createFrustum(const vsg::GeometryInfo& info, const vsg::StateInfo& stateInfo, const float& pitch)
{
    using namespace vsg;

    auto& subgraph = _frustums[std::make_pair(info, stateInfo)];
    if (subgraph)
    {
        return subgraph;
    }

    uint32_t instanceCount = 1;
    auto positions = instancePositions(info, instanceCount);
    auto colors = instanceColors(info, instanceCount);

    auto dx = info.dx;
    auto dy = info.dy;
    auto dz = info.dz;
    auto origin = info.position;
    auto [t_origin, t_scale, t_top] = y_texcoord(stateInfo).value;

    vec3 v000(origin - dx - dy);
    vec3 v010(origin - dx + dy);
    vec3 v100(origin + dx - dy);
    vec3 v110(origin + dx + dy);

    vec3 v001(origin - dx * pitch - dy * pitch + dz);
    vec3 v011(origin - dx * pitch + dy * pitch + dz);
    vec3 v101(origin + dx * pitch - dy * pitch + dz);
    vec3 v111(origin + dx * pitch + dy * pitch + dz);

    vec2 t00(0.0f, t_origin);
    vec2 t01(0.0f, t_top);
    vec2 t10(1.0f, t_origin);
    vec2 t11(1.0f, t_top);

    ref_ptr<vec3Array> vertices;
    ref_ptr<vec3Array> normals;
    ref_ptr<vec2Array> texcoords;
    ref_ptr<ushortArray> indices;

    if (stateInfo.wireframe) 
    {
        vec3 n0 = normalize(v000 - v111);
        vec3 n1 = normalize(v100 - v011);
        vec3 n2 = normalize(v110 - v001);
        vec3 n3 = normalize(v010 - v101);
        vec3 n4 = -n2;
        vec3 n5 = -n3;
        vec3 n6 = -n0;
        vec3 n7 = -n1;

        // set up vertex and index arrays
        vertices = vec3Array::create(
            {v000, v100, v110, v010,
            v001, v101, v111, v011});

        normals = vec3Array::create(
            {n0, n1, n2, n3,
            n4, n5, n6, n7});

        texcoords = vec2Array::create(
            {t00, t10, t11, t01,
            t00, t10, t11, t01});

        indices = ushortArray::create(
            {0, 1, 1, 2, 2, 3, 3, 0,
            0, 4, 1, 5, 2, 6, 3, 7,
            4, 5, 5, 6, 6, 7, 7, 4});
    }
    else
    {
        vec3 n0 = normalize(cross(dx, dz));
        vec3 n1 = normalize(cross(dy, dz));
        vec3 n2 = -n0;
        vec3 n3 = -n1;
        vec3 n4 = normalize(cross(dy, dx));
        vec3 n5 = -n4;

        // set up vertex and index arrays
        vertices = vec3Array::create(
            {v000, v100, v101, v001,   // front
            v100, v110, v111, v101,   // right
            v110, v010, v011, v111,   // far
            v010, v000, v001, v011,   // left
            v010, v110, v100, v000,   // bottom
            v001, v101, v111, v011}); // top

        normals = vec3Array::create(
            {n0, n0, n0, n0,
            n1, n1, n1, n1,
            n2, n2, n2, n2,
            n3, n3, n3, n3,
            n4, n4, n4, n4,
            n5, n5, n5, n5});

        texcoords = vec2Array::create(
            {t00, t10, t11, t01,
            t00, t10, t11, t01,
            t00, t10, t11, t01,
            t00, t10, t11, t01,
            t00, t10, t11, t01,
            t00, t10, t11, t01});

        indices = ushortArray::create(
            {0, 1, 2, 0, 2, 3,
            4, 5, 6, 4, 6, 7,
            8, 9, 10, 8, 10, 11,
            12, 13, 14, 12, 14, 15,
            16, 17, 18, 16, 18, 19,
            20, 21, 22, 20, 22, 23});
    }

    if (info.transform != identity)
    {
        transform(info.transform, vertices, normals);
    }

    // setup geometry
    auto vid = VertexIndexDraw::create();

    DataList arrays;
    arrays.push_back(vertices);
    if (normals) arrays.push_back(normals);
    if (texcoords) arrays.push_back(texcoords);
    if (colors) arrays.push_back(colors);
    if (positions) arrays.push_back(positions);
    vid->assignArrays(arrays);

    vid->assignIndices(indices);
    vid->indexCount = static_cast<uint32_t>(indices->size());
    vid->instanceCount = instanceCount;

    subgraph = decorateAndCompileIfRequired(info, stateInfo, vid);
    return subgraph;
}

vsg::ref_ptr<vsg::Node> MyBuilder::createBat(const vsg::GeometryInfo& info, const vsg::StateInfo& stateInfo, vsg::vec2Array data)
{
    // using namespace vsg;

    // auto& subgraph = _frustums[std::make_pair(info, stateInfo)];
    // if (subgraph)
    // {
    //     return subgraph;
    // }

    // uint32_t instanceCount = 1;
    // auto positions = instancePositions(info, instanceCount);
    // auto colors = instanceColors(info, instanceCount);
    return nullptr;
}