#include "MyBuilder.h"
#include <iostream>

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

static vsg::vec3 calcNorm(const vsg::vec3& o, const vsg::vec3& a, const vsg::vec3& b)
{
    return vsg::normalize(vsg::cross(a - o, b - o));
}


vsg::ref_ptr<vsg::Node> MyBuilder::createBat(vsg::ref_ptr<vsg::vec2Array> curve, const vsg::GeometryInfo& info, const vsg::StateInfo& stateInfo)
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

    const size_t square_count = 50;

    size_t row_count = curve->valueCount() - 1;

    auto vertices = vec3Array::create(4 * square_count * row_count);
    auto& v = *vertices;

    auto normals = vec3Array::create(4 * square_count * row_count);
    auto& n = *normals;

    // auto texcoords = vec2Array::create(4 * square_count * row_count);
    // auto& t = *texcoords;

    auto indices = ushortArray::create(3 * 2 * square_count * row_count);
    auto& ndcs = *indices;

    size_t vert = 0;
    size_t iindex = 0;

    for (size_t row = 0; row < row_count; row++) {
        using namespace std;
        auto z0 = (*curve)[row].x;
        auto r0 = (*curve)[row].y;
        auto z1 = (*curve)[row+1].x;
        auto r1 = (*curve)[row+1].y;
        cout << "z0=" << z0 << " r0=" << r0 << " z1=" << z1 << " r1=" << r1 << endl;

        double angle0 = 0.0;

        for (size_t i = 0; i < square_count; i++) {
            double angle1 = 2 * M_PI * (i+1) / square_count;
            auto p0 = vec3{float(cos(angle0) * r0), float(sin(angle0) * r0), z0};
            auto p1 = vec3{float(cos(angle0) * r1), float(sin(angle0) * r1), z1};
            auto p2 = vec3{float(cos(angle1) * r0), float(sin(angle1) * r0), z0};
            auto p3 = vec3{float(cos(angle1) * r1), float(sin(angle1) * r1), z1};
            auto norm = calcNorm(p0, p1, p2);

            ndcs[iindex++] = vert;
            ndcs[iindex++] = vert + 1;
            ndcs[iindex++] = vert + 2;
            ndcs[iindex++] = vert + 1;
            ndcs[iindex++] = vert + 3;
            ndcs[iindex++] = vert + 2;

            v[vert] = p0;
            n[vert++] = norm;

            v[vert] = p1;
            n[vert++] = norm;

            v[vert] = p2;
            n[vert++] = norm;

            v[vert] = p3;
            n[vert++] = norm;

            angle0 = angle1;
        }
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
    // if (texcoords) arrays.push_back(texcoords);
    // if (colors) arrays.push_back(colors);
    if (positions) arrays.push_back(positions);
    vid->assignArrays(arrays);

    vid->assignIndices(indices);
    vid->indexCount = static_cast<uint32_t>(indices->size());
    vid->instanceCount = instanceCount;

    subgraph = decorateAndCompileIfRequired(info, stateInfo, vid);
    return subgraph;
    return nullptr;
}