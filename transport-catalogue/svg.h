#pragma once
#define _USE_MATH_DEFINES

#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>


namespace svg {
    using Color = std::string;
    inline const Color NoneColor{"none"};

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    std::ostream &operator<<(std::ostream &out, StrokeLineCap line_cap);

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream &operator<<(std::ostream &out, StrokeLineJoin line_join);

    struct Point {
        Point() = default;

        Point(double x, double y)
            : x(x), y(y) {
        }

        double x = 0;
        double y = 0;
    };

    template<typename Owner>
    class PathProps {
    public:
        Owner &SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }

        Owner &SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }

        Owner &SetStrokeWidth(double width) {
            stroke_width_ = width;
            return AsOwner();
        }

        Owner &SetStrokeLineCap(StrokeLineCap line_cap) {
            stroke_line_cap_ = line_cap;
            return AsOwner();
        }

        Owner &SetStrokeLineJoin(StrokeLineJoin line_join) {
            stroke_line_join_ = line_join;
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        // Метод RenderAttrs выводит в поток общие для всех путей атрибуты fill и stroke
        void RenderAttrs(std::ostream &out) const {
            using namespace std::literals;
            if (fill_color_) {
                out << R"( fill=")" << *fill_color_ << R"(")";
            }
            if (stroke_color_) {
                out << R"( stroke=")" << *stroke_color_ << R"(")";
            }
            if (stroke_width_) {
                out << R"( stroke-width=")" << *stroke_width_ << R"(")";
            }
            if (stroke_line_cap_) {
                out << R"( stroke-linecap=")" << *stroke_line_cap_ << R"(")";
            }
            if (stroke_line_join_) {
                out << R"( stroke-linejoin=")" << *stroke_line_join_ << R"(")";
            }
        }

    private:
        Owner &AsOwner() {
            // static_cast безопасно преобразует *this к Owner&,
            // если класс Owner — наследник PathProps
            return static_cast<Owner &>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_line_cap_;
        std::optional<StrokeLineJoin> stroke_line_join_;
    };

    /*
     * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
     * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
     */
    struct RenderContext {
        RenderContext(std::ostream &out)
            : out(out) {
        }

        RenderContext(std::ostream &out, int indent_step, int indent = 0)
            : out(out), indent_step(indent_step), indent(indent) {
        }

        [[nodiscard]] RenderContext Indented() const {
            return {out, indent_step, indent + indent_step};
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream &out;
        int indent_step = 0;
        int indent = 0;
    };

    /*
     * Абстрактный базовый класс Object служит для унифицированного хранения
     * конкретных тегов SVG-документа
     * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
     */
    class Object {
    public:
        void Render(const RenderContext &context) const;
        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext &context) const = 0;
    };

    /*
     * Класс Circle моделирует элемент <circle> для отображения круга
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */
    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle &SetCenter(Point center);
        Circle &SetRadius(double radius);

    private:
        void RenderObject(const RenderContext &context) const override;
        Point center_;
        double radius_ = 1.0;
    };

    /*
     * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
    class Polyline : public Object, public PathProps<Polyline> {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline &AddPoint(Point point);
        void RenderObject(const svg::RenderContext &context) const override;
        /*
         * Прочие методы и данные, необходимые для реализации элемента <polyline>
         */
    private:
        std::vector<Point> points_;
    };

    /*
     * Класс Text моделирует элемент <text> для отображения текста
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text : public Object, public PathProps<Text> {
    public:
        // Задаёт координаты опорной точки (атрибуты x и y)
        Text &SetPosition(Point pos);
        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text &SetOffset(Point offset);
        // Задаёт размеры шрифта (атрибут font-size)
        Text &SetFontSize(uint32_t size);
        // Задаёт название шрифта (атрибут font-family)
        Text &SetFontFamily(std::string font_family);
        // Задаёт толщину шрифта (атрибут font-weight)
        Text &SetFontWeight(std::string font_weight);
        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text &SetData(std::string data);
        void RenderObject(const svg::RenderContext &context) const override;
        // Прочие данные и методы, необходимые для реализации элемента <text>
    private:
        Point pos_{0., 0.};
        Point offset_{0., 0.};
        uint32_t font_size_{1};
        std::optional<std::string> font_weight_;
        std::optional<std::string> font_family_;
        std::string data_;
    };

    class ObjectContainer {
    public:
        template<typename Obj>
        void Add(Obj obj) {
            AddPtr(std::make_unique<Obj>(std::move(obj)));
        }

        virtual void AddPtr(std::unique_ptr<Object> &&obj) = 0;

    protected:
        virtual ~ObjectContainer() = default;
    };

    class Document : public ObjectContainer {
    public:
        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object> &&obj) override;
        // Выводит в ostream svg-представление документа
        void Render(std::ostream &out) const;

        // Прочие методы и данные, необходимые для реализации класса Document
    private:
        std::vector<std::unique_ptr<Object> > objects_;
    };

    class Drawable {
    public:
        virtual void Draw(ObjectContainer &container) const = 0;
        virtual ~Drawable() = default;
    };
} // namespace svg

namespace shapes {
    class Triangle : public svg::Drawable {
    public:
        Triangle(svg::Point p1, svg::Point p2, svg::Point p3)
            : p1_(p1), p2_(p2), p3_(p3) {
        }

        // Реализует метод Draw интерфейса svg::Drawable
        void Draw(svg::ObjectContainer &container) const override;

    private:
        svg::Point p1_, p2_, p3_;
    };

    class Star : public svg::Drawable {
    public:
        Star(svg::Point center, double outer_rad, double inner_rad, int num_rays)
            : center_(center), outer_rad_(outer_rad), inner_rad_(inner_rad), num_rays_(num_rays) {
        };
        [[nodiscard]] static svg::Polyline
        CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays);
        void Draw(svg::ObjectContainer &container) const override;

    private:
        svg::Point center_;
        double outer_rad_;
        double inner_rad_;
        int num_rays_;
    };

    class Snowman : public svg::Drawable {
    public:
        Snowman(svg::Point point, double radius) : head_point_(point), radius_(radius) {
        }

        void Draw(svg::ObjectContainer &container) const override;

    private:
        svg::Point head_point_;
        double radius_;
    };
}