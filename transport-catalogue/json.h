#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {
    using namespace std::literals;

    class Document;
    class Node;
    // Сохраните объявления Dict и Array без изменения
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    class Node :public Value {
    public:
        /* Реализуйте Node, используя std::variant */
        using Value::variant;

        bool IsInt() const;
        bool IsDouble() const;// Возвращает true, если в Node хранится int либо double.
        bool IsPureDouble() const; //Возвращает true, если в Node хранится double.
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const; //.Возвращает значение типа double, если внутри хранится double либо int.В последнем случае возвращается приведённое в double значение.
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

        const Value& GetValue() const { return *this; }
    };

    bool operator==(const Node& first, const Node& second);

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

    bool operator==(const Document& first, const Document& second);
    bool operator!=(const Document& first, const Document& second);

} // namespace json