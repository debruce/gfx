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
            n4, n4, n4, n4,    // auto texcoords = vec2Array::create({
    //     { 1.0, 0.0 }, { 0.0, -1.0 }, { 0.0, 1.0 }
    // });
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


vsg::ref_ptr<vsg::Node> MyBuilder::createBat(vsg::ref_ptr<vsg::vec2Array> curve, const size_t& square_count, const vsg::GeometryInfo& info, const vsg::StateInfo& stateInfo)
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

            auto p0 = dx * (c0 * r0) + dy * (s0 * r0) + dz * z0 + origin;
            auto p1 = dx * (c0 * r1) + dy * (s0 * r1) + dz * z1 + origin;
            auto p2 = dx * (c1 * r0) + dy * (s1 * r0) + dz * z0 + origin;
            auto p3 = dx * (c1 * r1) + dy * (s1 * r1) + dz * z1 + origin;
            auto norm = -calcNorm(p0, p1, p2);

            float zCoord0 = (z0 - (*curve)[0].x) * zScale;
            float zCoord1 = (z1 - (*curve)[0].x) * zScale;            
            ndcs[iindex++] = vert;
            ndcs[iindex++] = vert + 2;
            ndcs[iindex++] = vert + 1;
            ndcs[iindex++] = vert + 1;
            ndcs[iindex++] = vert + 2;
            ndcs[iindex++] = vert + 3;

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
    return nullptr;
}

#if 0
struct PolyhedronBuilder {
    size_t current_index;
    std::vector<vsg::vec3>  vertices;
    std::vector<vsg::vec3>  normals;
    std::vector<unsigned short> indices;

    PolyhedronBuilder()
    {
        current_index = 0;
        vertices.reserve(1024);
        normals.reserve(1024);
        indices.reserve(1024);
    }

    vsg::ref_ptr<vsg::vec3Array> getVertices()
    {
        auto v = vsg::vec3Array::create(vertices.size());
        for (size_t i = 0; i < vertices.size(); i++)  v->at(i) = vertices[i];
        return v;
    }

    vsg::ref_ptr<vsg::vec3Array> getNormals()
    {
        auto n = vsg::vec3Array::create(normals.size());
        for (size_t i = 0; i < normals.size(); i++)  n->at(i) = normals[i];
        return n;
    }

    vsg::ref_ptr<vsg::ushortArray> getIndices()
    {
        auto ndc = vsg::ushortArray::create(indices.size());
        for (size_t i = 0; i < indices.size(); i++)  ndc->at(i) = indices[i];
        return ndc;
    }

    void triangle(const vsg::vec3& a, const vsg::vec3& b, const vsg::vec3& c)
    {
        indices.push_back(current_index);
        indices.push_back(current_index+1);
        indices.push_back(current_index+2);
        current_index += 3;
        auto up = calcNorm(a, b, c);
        vertices.push_back(a);
        vertices.push_back(b);
        vertices.push_back(c);
        normals.push_back(up);
        normals.push_back(up);
        normals.push_back(up);        
    }

    void quad(const vsg::vec3& a, const vsg::vec3& b, const vsg::vec3& c, const vsg::vec3& d, bool flip = false)
    {
        indices.push_back(current_index);
        indices.push_back(current_index+1);
        indices.push_back(current_index+2);

        indices.push_back(current_index);
        indices.push_back(current_index+2);
        indices.push_back(current_index+3);
        current_index += 6;

        auto up = calcNorm(a, b, c);
        if (flip) up = -up;

        vertices.push_back(a);
        vertices.push_back(b);
        vertices.push_back(c);
        vertices.push_back(d);

        normals.push_back(up);
        normals.push_back(up);
        normals.push_back(up);
        normals.push_back(up); 
    }
};
#endif