#pragma once

#include <cstdint>
#include <deque>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>

namespace transport_catalogue {
    namespace svg {

        struct Point {
            Point() = default;
            Point(double x_, double y_)
                : x(x_)
                , y(y_) {
            }
            
            
            double x = 0;
            double y = 0;
        };

        /*
        * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
        * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
        */
        struct RenderContext {
            RenderContext(std::ostream& out_)
                : out(out_) {
            }

            RenderContext(std::ostream& out_, int indent_step_, int indent_ = 0)
                : out(out_)
                , indent_step(indent_step_)
                , indent(indent_) {
            }

            RenderContext Indented() const {
                return {out, indent_step, indent + indent_step};
            }

            void RenderIndent() const {
                for (int i = 0; i < indent; ++i) {
                    out.put(' ');
                }
            }

            std::ostream& out;
            int indent_step = 0;
            int indent = 0;
        };

        struct Rgb {
            Rgb() = default;
            Rgb(uint8_t red_, uint8_t green_, uint8_t blue_)
                : red(red_), green(green_), blue(blue_)
                {}
            virtual ~Rgb() = default;

            uint8_t red = 0;
            uint8_t green = 0;
            uint8_t blue = 0;
        };

        struct Rgba : public Rgb {
            Rgba() : Rgb(), opacity(1.) {}
            Rgba(uint8_t red_, uint8_t green_, uint8_t blue_, double opacity_)
                : Rgb(red_, green_, blue_), opacity(opacity_)
                {}

            double opacity;
        };

        using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

        // Объявив в заголовочном файле константу со спецификатором inline,
        // мы сделаем так, что она будет одной на все единицы трансляции,
        // которые подключают этот заголовок.
        // В противном случае каждая единица трансляции будет использовать свою копию этой константы
        inline const Color NoneColor{"none"};

        struct ColorPrinter {
            std::ostream& out;
            void operator()(std::monostate) const;
            void operator()(std::string) const;
            void operator()(Rgb) const;
            void operator()(Rgba) const;
        };

        std::ostream& operator<<(std::ostream& out, Color color);

        enum class StrokeLineCap {
            BUTT,
            ROUND,
            SQUARE,
        };

        enum class StrokeLineJoin {
            ARCS,
            BEVEL,
            MITER,
            MITER_CLIP,
            ROUND,
        };

        template <typename Owner>
        class PathProps {
        public:
            Owner& SetFillColor(Color color);
            Owner& SetStrokeColor(Color color);
            Owner& SetStrokeWidth(double width);
            Owner& SetStrokeLineCap(StrokeLineCap line_cap);
            Owner& SetStrokeLineJoin(StrokeLineJoin line_join);

        protected:
            ~PathProps() = default;

            // Метод RenderAttrs выводит в поток общие для всех путей атрибуты fill и stroke
            void RenderAttrs(std::ostream& out) const;

        private:
            Owner& AsOwner();

            std::optional<Color> fill_color_ = std::nullopt;
            std::optional<Color> stroke_color_ = std::nullopt;
            std::optional<double> width_ = std::nullopt;
            std::optional<StrokeLineCap> line_cap_ = std::nullopt;
            std::optional<StrokeLineJoin> line_join_ = std::nullopt;
        };

        /*
        * Абстрактный базовый класс Object служит для унифицированного хранения
        * конкретных тегов SVG-документа
        * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
        */
        class Object {
        public:
            void Render(const RenderContext& context) const;

            virtual ~Object() = default;

        private:
            virtual void RenderObject(const RenderContext& context) const = 0;
        };

        /*
        * Класс Circle моделирует элемент <circle> для отображения круга
        * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
        */
        class Circle final : public Object, public PathProps<Circle> {
        public:
            Circle& SetCenter(Point center);
            Circle& SetRadius(double radius);

        private:
            void RenderObject(const RenderContext& context) const override;

            Point center_;
            double radius_ = 1.0;
        };

        /*
        * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
        * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
        */
        class Polyline final : public Object, public PathProps<Polyline> {
        public:
            // Добавляет очередную вершину к ломаной линии
            Polyline& AddPoint(Point point);


        private:
            void RenderObject(const RenderContext& context) const override;

            std::deque<Point> points_;
        };

        /*
        * Класс Text моделирует элемент <text> для отображения текста
        * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
        */
        class Text final : public Object, public PathProps<Text> {
        public:
            // Задаёт координаты опорной точки (атрибуты x и y)
            Text& SetPosition(Point pos);

            // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
            Text& SetOffset(Point offset);

            // Задаёт размеры шрифта (атрибут font-size)
            Text& SetFontSize(uint32_t size);

            // Задаёт название шрифта (атрибут font-family)
            Text& SetFontFamily(std::string font_family);

            // Задаёт толщину шрифта (атрибут font-weight)
            Text& SetFontWeight(std::string font_weight);

            // Задаёт текстовое содержимое объекта (отображается внутри тега text)
            Text& SetData(std::string data);

        private:
            void RenderObject(const RenderContext& context) const override;
            std::string DeleteSpaces(const std::string& data) const;

            Point pos_;
            Point offset_;
            uint32_t size_ = 1;
            std::string font_family_;
            std::string font_weight_;
            std::string data_;
        };


        class ObjectContainer {
        public:
            template <typename T>
            void Add(T obj);

            virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
        };

        class Document final : public ObjectContainer {
        public:
            // Добавляет в svg-документ объект-наследник svg::Object
            void AddPtr(std::unique_ptr<Object>&& obj) override;

            // Выводит в ostream svg-представление документа
            void Render(std::ostream& out) const;

        private:
            std::deque<std::unique_ptr<Object>> objects_;
        };

        class Drawable {
        public:
            virtual void Draw(ObjectContainer& container) const = 0;
            virtual ~Drawable() = default;
        };

        std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap);
        std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join);

        // -- определение шаблонных методов --

        template <typename T>
        void ObjectContainer::Add(T obj) {
            AddPtr(std::make_unique<T>(std::move(obj)));
        }

        template <typename Owner>
        Owner& PathProps<Owner>::SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }

        template <typename Owner>
        Owner& PathProps<Owner>::SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }

        template <typename Owner>
        Owner& PathProps<Owner>::SetStrokeWidth(double width) {
            width_ = width;
            return AsOwner();
        }

        template <typename Owner>
        Owner& PathProps<Owner>::SetStrokeLineCap(StrokeLineCap line_cap) {
            line_cap_ = line_cap;
            return AsOwner();
        }

        template <typename Owner>
        Owner& PathProps<Owner>::SetStrokeLineJoin(StrokeLineJoin line_join) {
            line_join_ = line_join;
            return AsOwner();
        }

        template <typename Owner>
        void PathProps<Owner>::RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) {
                out << " fill=\""sv << *fill_color_ << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv << *stroke_color_ << "\""sv;
            }
            if (width_) {
                out << " stroke-width=\""sv << *width_ << "\""sv;
            }
            if (line_cap_) {
                out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
            }
            if (line_join_) {
                out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
            }
        }

        template <typename Owner>
        Owner& PathProps<Owner>::AsOwner() {
            // static_cast безопасно преобразует *this к Owner&,
            // если класс Owner — наследник PathProps
            return static_cast<Owner&>(*this);
        }

    } // namespace svg
} // namespace transport_catalogue