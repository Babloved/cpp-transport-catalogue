#include "svg.h"
#include <iomanip>

using namespace std;
using namespace literals;

namespace svg {
    void Object::Render(const RenderContext &context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);
        context.out << endl;
    }

    // ---------- Circle ------------------

    Circle &Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle &Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext &context) const {
        auto &out = context.out;
        out << R"(<circle cx=")" << center_.x << R"(" cy=")" << center_.y << R"(" )";
        out << R"(r=")" << radius_ << R"(")";
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------
    Polyline &Polyline::AddPoint(Point point) {
        {
            points_.push_back(point);
            return *this;
        }
    }

    void Polyline::RenderObject(const RenderContext &context) const {
        auto &out = context.out;
        out << R"(<polyline points=")";
        for (const auto &point: points_) {
            if (&point != &points_.front()) {
                out << ' ';
            }
            out << point.x << ","sv << point.y;
        }
        out << R"(")";
        RenderAttrs(out);
        out << R"(/>)";
    }

    // ---------- Text ------------------
    Text &Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text &Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text &Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text &Text::SetFontFamily(string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text &Text::SetFontWeight(string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text &Text::SetData(string data) {
        data_ = std::move(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext &context) const {
        //Hello, <UserName>. Would you like some "M&M's"?
        //<text>Hello, &lt;UserName&gt;. Would you like some &quot;M&amp;M&apos;s&quot;?</text>
        auto &out = context.out;
        out << "<text";
        RenderAttrs(out);
        out << R"( x=")" << pos_.x << R"(" y=")" << pos_.y
                << R"(" dx=")" << offset_.x << R"(" dy=")" << offset_.y
                << R"(" font-size=")" << font_size_ << R"(")";
        if (font_family_) {
            out << R"( font-family=")" << font_family_->data() << R"(")";
        }
        if (font_weight_) {
            out << R"( font-weight=")" << font_weight_->data() << R"(")";
        }
        out << ">";
        for (const auto &ch: data_) {
            switch (ch) {
                case '\"':
                    out << "&quot;";
                    break;
                case '\'':
                    out << "&apos;";
                    break;
                case '<':
                    out << "&lt;";
                    break;
                case '>':
                    out << "&gt;";
                    break;
                case '&':
                    out << "&amp;";
                    break;
                default:
                    out << ch;
                    break;
            }
        }
        out << "</text>";
    }

    // ---------- Document ------------------
    void Document::AddPtr(unique_ptr<Object> &&obj) {
        objects_.push_back(std::move(obj));
    }

    void Document::Render(ostream &out) const {
        out << R"(<?xml version="1.0" encoding="UTF-8" ?>)" << endl;
        out << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1">)" << endl;
        for (const auto &object: objects_) {
            object->Render(RenderContext(out));
        }
        out << "</svg>"sv;
    }

    // ---------- StrokeLineCap ------------------

    ostream &operator<<(ostream &out, StrokeLineCap line_cap) {
        switch (line_cap) {
            case StrokeLineCap::BUTT:
                out << "butt";
                break;
            case StrokeLineCap::ROUND:
                out << "round";
                break;
            case StrokeLineCap::SQUARE:
                out << "square";
                break;
        }
        return out;
    }

    // ---------- StrokeLineJoin ------------------
    ostream &operator<<(ostream &out, StrokeLineJoin line_join) {
        switch (line_join) {
            case StrokeLineJoin::ARCS:
                out << "arcs";
                break;
            case StrokeLineJoin::BEVEL:
                out << "bevel";
                break;
            case StrokeLineJoin::MITER:
                out << "miter";
                break;
            case StrokeLineJoin::MITER_CLIP:
                out << "miter-clip";
                break;
            case StrokeLineJoin::ROUND:
                out << "round";
                break;
        }
        return out;
    }
} // namespace svg
namespace shapes {
    // ---------- Triangle ------------------
    void Triangle::Draw(svg::ObjectContainer &container) const {
        container.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
    }

    // ---------- Star ------------------
    svg::Polyline Star::CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays) {
        svg::Polyline polyline;
        for (int i = 0; i <= num_rays; ++i) {
            double angle = 2 * M_PI * (i % num_rays) / num_rays;
            polyline.AddPoint({center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle)});
            if (i == num_rays) {
                break;
            }
            angle += M_PI / num_rays;
            polyline.AddPoint({center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle)});
        }
        return polyline;
    }

    void Star::Draw(svg::ObjectContainer &container) const {
        container.Add(
            CreateStar(center_, outer_rad_, inner_rad_, num_rays_).SetFillColor("red"s).SetStrokeColor("black"s));
    }

    // ---------- Snowman ------------------
    void Snowman::Draw(svg::ObjectContainer &container) const {
        container.Add(svg::Circle()
            .SetCenter({head_point_.x, head_point_.y + 5 * radius_})
            .SetRadius(radius_ * 2)
            .SetFillColor("rgb(240,240,240)")
            .SetStrokeColor("black"));
        container.Add(svg::Circle()
            .SetCenter({head_point_.x, head_point_.y + 2 * radius_})
            .SetRadius(radius_ * 1.5)
            .SetFillColor("rgb(240,240,240)")
            .SetStrokeColor("black"));
        container.Add(svg::Circle()
            .SetCenter(head_point_)
            .SetRadius(radius_)
            .SetFillColor("rgb(240,240,240)")
            .SetStrokeColor("black"));
    }
} // namespace shapes
