#include "svg.h"

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
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

    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point)
    {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const
    {
        bool is_first = true;
        auto& out = context.out;
        out << "<polyline points=\""sv;
        for (const auto& point : points_) {
            if (is_first) {
                is_first = false;
                out << point.x << ',' << point.y;
                continue;
            }
            out << " " << point.x << ',' << point.y;
        }
        out << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Text ------------------

    Text& Text::SetPosition(Point pos)
    {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset)
    {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size)
    {
        size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family)
    {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight)
    {
        font_weight_ = font_weight;
        return *this;
    }

    void StringReplacer(std::string& inputStr, const std::string& src, const std::string& dst)
    {
        size_t pos = inputStr.find(src);
        while (pos != std::string::npos) {
            inputStr.replace(pos, src.size(), dst);
            pos = inputStr.find(src, pos);
        }
    }

    std::string_view Trim(std::string_view string) {
        const auto start = string.find_first_not_of(' ');
        if (start == string.npos) {
            return {};
        }
        return string.substr(start, string.find_last_not_of(' ') + 1 - start);
    }

    Text& Text::SetData(std::string data)
    {
        StringReplacer(data, "<"s, "&lt;");
        StringReplacer(data, ">"s, "&gt;");
        StringReplacer(data, "&"s, "&amp;");
        StringReplacer(data, "\'"s, "&apos;");
        StringReplacer(data, "\""s, "&quot;");
        data_ = std::string(Trim(data));
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;
        out << "<text";

        RenderAttrs(out);

        out <<" x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv
            << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv
            << "font-size=\""sv << size_ << "\""sv;
        if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\"";
        }
        out << ">"sv << data_ << "</text>"sv;
    }

    // --------- Document ----------





    void Document::AddPtr(std::unique_ptr<Object>&& obj)
    {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const
    {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

        for (const auto& obj : objects_) {
            obj->Render({ out, 2, 2 });
        }

        out << "</svg>"sv << std::endl;
    }

}  // namespace svg