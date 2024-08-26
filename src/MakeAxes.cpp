#include <vsg/all.h>

static vsg::ref_ptr<vsg::MatrixTransform> makeStovePipe(vsg::ref_ptr<vsg::Builder> builder, const vsg::vec4& clr)
{
    vsg::GeometryInfo geomInfo;
    vsg::StateInfo stateInfo;

    float thickness = .05f;
    geomInfo.dx = {thickness, 0.0f, 0.0f};
    geomInfo.dy = {0.0f, thickness, 0.0f};
    geomInfo.dz = {0.0f, 0.0f, 1.0f};
    geomInfo.color = clr;
    auto z_cylinder = builder->createCylinder(geomInfo, stateInfo);
    geomInfo.dx = {.25f, 0.0f, 0.0f};
    geomInfo.dy = {0.0f, .25f, 0.0f};
    geomInfo.dz = {0.0f, 0.0f, .25f};
    geomInfo.transform = vsg::translate(0.0f, 0.0f, 0.5f);
    auto z_cone = builder->createCone(geomInfo, stateInfo);
    auto stove_pipe = vsg::MatrixTransform::create();
    stove_pipe->addChild(z_cylinder);
    stove_pipe->addChild(z_cone);
    return stove_pipe;
}

vsg::ref_ptr<vsg::Group> makeAxes(vsg::ref_ptr<vsg::Builder> builder)
{
    auto zStovePipe = makeStovePipe(builder, vsg::vec4{0.0f, 0.0f, 1.0f, 1.0f});

    auto xStovePipe = makeStovePipe(builder, vsg::vec4{1.0f, 0.0f, 0.0f, 1.0f});
    xStovePipe->matrix = vsg::rotate(vsg::radians(90.0f), 0.0f, 1.0f, 0.0f);

    auto yStovePipe = makeStovePipe(builder, vsg::vec4{0.0f, 1.0f, 0.0f, 1.0f});
    yStovePipe->matrix = vsg::rotate(vsg::radians(90.0f), 1.0f, 0.0f, 0.0f);

    auto axes = vsg::Group::create();
    axes->addChild(zStovePipe);
    axes->addChild(xStovePipe);
    axes->addChild(yStovePipe);
    return axes;
}

vsg::ref_ptr<vsg::Node> loadObject(vsg::ref_ptr<vsg::Options> options, const std::string& filepath, const vsg::dmat4& rot)
{
    auto model = vsg::read_cast<vsg::Node>(filepath, options);
    auto bounds = vsg::visit<vsg::ComputeBounds>(model).bounds;
    auto center = (bounds.min + bounds.max) / 2.0;
    auto sz = (bounds.max - bounds.min);
    auto s = 2.0 / std::max(std::max(sz[0], sz[1]), sz[2]);


    auto m = vsg::MatrixTransform::create();
    m->matrix = rot * vsg::scale(s, s, s) * vsg::translate(-center[0], -center[1], -center[2]);
    m->addChild(model);
    return m;
}