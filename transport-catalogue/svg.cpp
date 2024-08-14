#include "svg.h"
namespace transport_catalogue {
    namespace svg {

        using namespace std::literals;

        void Object::Render(const RenderContext& context) const {
            context.RenderIndent();

            // Делегируем вывод тега своим подклассам
            RenderObject(context);

            context.out << std::endl;
        }

        // ---------- Circle ------------------

        Circle& Circle::SetCenter(Point center)  {
            center_ = center;
            return *this;
        }

        Circle& Circle::SetRadius(double radius)  {
            radius_ = radius;
            return *this;
        }

        void Circle::RenderObject(const RenderContext& context) const {
            auto& out = context.out;
            out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
            out << "r=\""sv << radius_ << "\""sv;
            RenderAttrs(out);
            out << "/>"sv;
        }

        // ---------- Polyline ----------------

        Polyline& Polyline::AddPoint(Point point) {
            points_.emplace_back(point);
            return *this;
        }

        void Polyline::RenderObject(const RenderContext& context) const {
            auto& out = context.out;
            out << "<polyline points=\""sv;
            bool is_first = true;
            for (const auto& point : points_) {
                if (!is_first) {
                    out << " "sv;
                }
                is_first = false;
                out << point.x << ","sv << point.y;
            }
            out << "\"";
            RenderAttrs(out);
            out << "/>"sv;
        }

        // ---------- Text --------------------

        Text& Text::SetPosition(Point pos) {
            pos_ = pos;
            return *this;
        }

        Text& Text::SetOffset(Point offset) {
            offset_ = offset;
            return *this;
        }

        Text& Text::SetFontSize(uint32_t size) {
            size_ = size;
            return *this;
        }

        Text& Text::SetFontFamily(std::string font_family) {
            font_family_ = font_family;
            return *this;
        }

        Text& Text::SetFontWeight(std::string font_weight) {
            font_weight_ = font_weight;
            return *this;
        }

        Text& Text::SetData(std::string data) {
            data_ = data;
            return *this;
        }

        std::string Text::DeleteSpaces(const std::string& data) const{
            if (data.empty()) {
                return {};
            }
            auto start_pos = data.find_first_not_of(' ');
            auto end_pos = data.find_last_not_of(' ');
            return data.substr(start_pos, end_pos - start_pos + 1);
        }

        void Text::RenderObject(const RenderContext& context) const {
            auto& out = context.out;
            out << "<text"sv;
            RenderAttrs(out);
            out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
            out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" font-size=\""sv << size_;
            if (!font_family_.empty()) {
                out << "\" font-family=\""sv << font_family_;
            }
            if (!font_weight_.empty()) {
                out << "\" font-weight=\""sv << font_weight_;
            }
            out << "\">"sv;
            auto str = DeleteSpaces(data_);
            for (const auto& ch : str) {
                if (ch == '"') out << "&quot;"sv;
                else if (ch == '\'') out << "&apos;"sv;
                else if (ch == '<') out << "&lt;"sv;
                else if (ch == '>') out << "&gt;"sv;
                else if (ch == '&') out << "&amp;"sv;
                else out << ch;
            }
            out << "</text>"sv;
        }

        // ---------- Other -------------------

        void Document::AddPtr(std::unique_ptr<Object>&& obj) {
            objects_.emplace_back(std::move(obj));    
        }

        void Document::Render(std::ostream& out) const {
            RenderContext output(out, 2, 2);

            out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
            out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n";
            for (const auto& object : objects_) {
                object->Render(output);
            }
            out << "</svg>"sv;
        }

        std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap) {
            if (line_cap == StrokeLineCap::BUTT) out << "butt"sv;
            else if (line_cap == StrokeLineCap::ROUND) out << "round"sv;
            else out << "square"sv;
            return out;
        }

        std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join) {
            if (line_join == StrokeLineJoin::ARCS) out << "arcs"sv;
            else if (line_join == StrokeLineJoin::BEVEL) out << "bevel"sv;
            else if (line_join == StrokeLineJoin::MITER) out << "miter"sv;
            else if (line_join == StrokeLineJoin::MITER_CLIP) out << "miter-clip"sv;
            else out << "round"sv;
            return out;
        }

        void ColorPrinter::operator()(std::monostate) const {
            out << "none"sv;
        }

        void ColorPrinter::operator()(std::string color) const {
            out << color;
        }

        void ColorPrinter::operator()(Rgb color) const {
            out << "rgb("sv << color.red + 0 << ","sv << color.green + 0 << ","sv << color.blue + 0 << ")"sv;
        }

        void ColorPrinter::operator()(Rgba color) const {
            out << "rgba("sv << color.red + 0 << ","sv << color.green + 0 << ","sv << color.blue + 0 << ","sv << color.opacity << ")"sv;
        }

        std::ostream& operator<<(std::ostream& out, Color color) {
            std::visit(ColorPrinter{out}, color);
            return out;
        }

    }  // namespace svg
} // namespace transport_catalogue