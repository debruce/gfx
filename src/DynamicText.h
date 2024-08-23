#pragma once
#include <vsg/all.h>

class DynamicText : public vsg::Inherit<vsg::MatrixTransform, DynamicText> {
    vsg::ref_ptr<vsg::Options> options;
    vsg::ref_ptr<vsg::stringValue> label;
    vsg::ref_ptr<vsg::Text> text;
public:
    DynamicText(const std::string& s, vsg::ref_ptr<vsg::Font> font, vsg::ref_ptr<vsg::Options> options) : options(options)
    {
        label = vsg::stringValue::create(s);
        auto layout = vsg::StandardLayout::create();
        layout->glyphLayout = vsg::StandardLayout::LEFT_TO_RIGHT_LAYOUT;
        layout->horizontalAlignment = vsg::StandardLayout::CENTER_ALIGNMENT;
        layout->verticalAlignment = vsg::StandardLayout::BOTTOM_ALIGNMENT;
        layout->position = vsg::vec3(0.0, 0.0, 0.0);
        layout->horizontal = vsg::vec3(1.0, 0.0, 0.0);
        layout->vertical = vsg::vec3(0.0, 0.0, 1.0);
        layout->color = vsg::vec4(0.0, 0.0, 0.0, 1.0);

        text = vsg::Text::create();
        text->technique = vsg::GpuLayoutTechnique::create();
        text->text = label;
        text->font = font;
        text->layout = layout;
        text->setup(64);

        addChild(text);
    }

    void set(const std::string& s)
    {
        label->value() = vsg::make_string(s);
        text->setup(0, options);
    }
};