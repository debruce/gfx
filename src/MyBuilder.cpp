#include "MyBuilder.h"
#include <iostream>

static vsg::vec3 calcNorm(const vsg::vec3& o, const vsg::vec3& a, const vsg::vec3& b)
{
    return vsg::normalize(vsg::cross(a - o, b - o));
}


vsg::ref_ptr<vsg::Node> MyBuilder::createLathe(vsg::ref_ptr<vsg::vec2Array> curve, const size_t& square_count, const vsg::GeometryInfo& info, const vsg::StateInfo& stateInfo, const double& phase)
{
    using namespace vsg;

    auto& subgraph = _lathes[std::make_pair(info, stateInfo)];
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

            float c0 = cosf(2 * M_PI * percent0 + phase);
            float s0 = sinf(2 * M_PI * percent0 + phase);
            float c1 = cosf(2 * M_PI * percent1 + phase);
            float s1 = sinf(2 * M_PI * percent1 + phase);

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
}

vsg::ref_ptr<vsg::Node> MyBuilder::createMesh(vsg::ref_ptr<vsg::vec3Array2D> mesh, const vsg::GeometryInfo& info, const vsg::StateInfo& stateInfo)
{
    using namespace vsg;

    auto& subgraph = _meshes[std::make_pair(info, stateInfo)];
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
        for (size_t col = 0; col < mesh->height(); col++) {
            auto p0 = (*mesh)(row, col) + origin;
            auto p1 = (*mesh)(row, (col+1) % mesh->height()) + origin;
            auto p2 = (*mesh)(row+1, col) + origin;
            auto p3 = (*mesh)(row+1, (col+1) % mesh->height()) + origin;
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

            // using namespace std;
            // cout << "r=" << row << " c=" << col << " = "
            //     << " p0=" << p0
            //     << " p1=" << p1
            //     << " p2=" << p2
            //     << " p3=" << p3
            //     << endl;
        }
    }
    // double zScale = 1.0 / (*curve)[curve->valueCount()-1].x - (*curve)[0].x;

    // for (size_t row = 0; row < row_count; row++) {
    //     using namespace std;
    //     auto z0 = (*curve)[row].x;
    //     auto r0 = (*curve)[row].y;
    //     auto z1 = (*curve)[row+1].x;
    //     auto r1 = (*curve)[row+1].y;

    //     for (size_t i = 0; i < square_count; i++) {
    //         float percent0 = float(i) / square_count;
    //         float percent1 = float(i+1) / square_count;

    //         float c0 = cosf(2 * M_PI * percent0);
    //         float s0 = sinf(2 * M_PI * percent0);
    //         float c1 = cosf(2 * M_PI * percent1);
    //         float s1 = sinf(2 * M_PI * percent1);

    //         auto p0 = dx * (c0 * r0) + dy * (s0 * r0) + dz * z0 + origin;
    //         auto p1 = dx * (c0 * r1) + dy * (s0 * r1) + dz * z1 + origin;
    //         auto p2 = dx * (c1 * r0) + dy * (s1 * r0) + dz * z0 + origin;
    //         auto p3 = dx * (c1 * r1) + dy * (s1 * r1) + dz * z1 + origin;
    //         auto norm = -calcNorm(p0, p1, p2);

    //         float zCoord0 = (z0 - (*curve)[0].x) * zScale;
    //         float zCoord1 = (z1 - (*curve)[0].x) * zScale;            
    //         ndcs[iindex++] = vert;
    //         ndcs[iindex++] = vert + 2;
    //         ndcs[iindex++] = vert + 1;
    //         ndcs[iindex++] = vert + 1;
    //         ndcs[iindex++] = vert + 2;
    //         ndcs[iindex++] = vert + 3;

    //         v[vert] = p0;
    //         n[vert] = norm;
    //         t[vert] = vec2{percent0, zCoord0};
    //         vert++;

    //         v[vert] = p1;
    //         n[vert] = norm;
    //         t[vert] = vec2{percent1, zCoord0};
    //         vert++;

    //         v[vert] = p2;
    //         n[vert] = norm;
    //         t[vert] = vec2{percent0, zCoord1}; 
    //         vert++;

    //         v[vert] = p3;
    //         n[vert] = norm;
    //         t[vert] = vec2{percent1, zCoord1};
    //         vert++;
    //     }
    // }

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