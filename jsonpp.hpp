/*
 * MIT License
 *
 * Copyright (c) 2016 Nicholas Samson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef TCAT_JSONPP_HPP
#define TCAT_JSONPP_HPP

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iterator>
#include <stdint.h>

namespace jsonpp {

    std::string escape_str(const std::string& str) {
        std::string out;

        for (auto& c : str) {
            char ca[4] = {};

            switch (c) {
                default:
                    ca[0] = c;
                    break;

                case '"':
                case '\\':
                case '/':
                    ca[0] = '\\';
                    ca[1] = c;
                    break;

                case '\b':
                    ca[0] = '\\';
                    ca[1] = 'b';
                    break;

                case '\f':
                    ca[0] = '\\';
                    ca[1] = 'f';
                    break;

                case '\n':
                    ca[0] = '\\';
                    ca[1] = 'n';
                    break;

                case '\r':
                    ca[0] = '\\';
                    ca[1] = 'r';
                    break;

                case '\t':
                    ca[0] = '\\';
                    ca[1] = 't';
                    break;
            }

            out.append(ca);
        }

        return out;
    }

    std::string parse_str(const std::string &str) {
        return "";
    }

    class JSONValue {
    public:
        virtual std::string to_string() const = 0;
        virtual JSONValue* create() const = 0;
        virtual JSONValue* clone() const = 0;
        virtual ~JSONValue() {}
    };

    JSONValue* clone(const JSONValue* that) {
        return that->clone();
    }

    class JSONNullType : public JSONValue {
    public:
        JSONNullType() : JSONValue() {}
        std::string to_string() const {return "null";}

        JSONNullType* create() const {
            return new JSONNullType();
        }

        JSONNullType* clone() const {
            return new JSONNullType();
        }
    };

    class JSONBooleanType : public JSONValue {

        bool value;

    public:
        JSONBooleanType(bool type) : JSONValue(), value(type) {}
        JSONBooleanType() : JSONValue(), value(false) {}

        std::string to_string() const {return value ? "true" : "false";}

        JSONBooleanType* create() const {
            return new JSONBooleanType();
        }

        JSONBooleanType* clone() const {
            return new JSONBooleanType(*this);
        }
    };

    class JSONArray : public JSONValue {

        std::vector<JSONValue*> values;

        void swap(const JSONArray& that) { std::swap(this->values, that.values); }

    public:
        JSONArray() : JSONValue() {}

        template <typename InputIterator>
        JSONArray(InputIterator begin, InputIterator end) : JSONValue(), values(begin, end) {}

        JSONArray(const JSONArray& that) {
            std::transform(that.begin(), that.end(),
                           std::back_insert_iterator<std::vector<JSONValue*> >(values),
                           jsonpp::clone
            );
        }

        typedef std::vector<JSONValue*>::iterator iterator;
        typedef std::vector<JSONValue*>::const_iterator const_iterator;

        iterator begin() { return values.begin(); }
        iterator end() {return values.end(); }
        const_iterator begin() const { return values.begin(); }
        const_iterator end() const { return values.end(); }

        JSONValue*& operator[](size_t index) { return values[index]; }
        JSONValue* const & operator[](size_t index) const { return values[index]; }

        size_t size() const {return values.size(); }

        JSONArray& operator=(JSONArray that) {
            swap(that);
            return *this;
        }

        JSONArray* create() const {
            return new JSONArray();
        }

        JSONArray* clone() const {
            return new JSONArray(*this);
        }

        ~JSONArray() {
            for (JSONValue*& ptr : values) {
                delete ptr;
            }

            values.clear();
        }

        std::string to_string() const {
            std::string out = "[";

            for (auto& val : values) {
                out.append(val->to_string());
                out.append(", ");
            }

            out.erase(out.size() - 2);

            out.append("]");
            return out;
        }

    };

    class JSONString : public JSONValue {
        std::string value;

    public:
        JSONString() : JSONValue() {}
        JSONString(const JSONString& that) {
            value = std::string(that.value);
        }

        JSONString(const std::string& str, bool parse=false) {
            if (!parse) {
                value = std::string(str);
            } else {
                value = jsonpp::parse_str(str);
            }
        }

        operator std::string() const {
            return std::string(value);
        }

        JSONString& operator=(JSONString that) {
            std::swap(value, that.value);
            return *this;
        }

        std::string to_string() const {
            std::string out = "\"";
            out.append(jsonpp::escape_str(value));
            out.append("\"");
            return out;
        }

        JSONString* create() const {
            return new JSONString();
        }

        JSONString* clone() const {
            return new JSONString(*this);
        }
    };

    enum class NumberType {
        FLOAT,
        INTEGER
    };

    class JSONNumber : public JSONValue {
        NumberType type;
        union {
            double dbl;
            int64_t integer;
        } val;

        template <typename T>
        T get() const {

        }
    };

    class JSONObject : public JSONValue {
        std::map<JSONString, JSONValue*> values;

        void swap(const JSONObject& that) { std::swap(values, that.values); }

    public:
        JSONObject() : JSONValue() {}

        template <typename InputIterator>
        JSONObject(InputIterator begin, InputIterator end): JSONValue(), values(begin, end) {}

        JSONObject(const JSONObject& that) {
            for (auto& pa : that.values) {
                values[pa.first] = pa.second->clone();
            }
        }

        typedef std::map<JSONString, JSONValue*>::iterator iterator;
        typedef std::map<JSONString, JSONValue*>::const_iterator const_iterator;

        iterator begin() { return values.begin(); }
        iterator end() {return values.end(); }
        const_iterator begin() const { return values.begin(); }
        const_iterator end() const { return values.end(); }

        JSONValue*& operator[](std::string index) { return values[index]; }
        JSONValue* const & operator[](std::string index) const { return values[index]; }

        bool contains(const std::string& key) const { return std::find(begin(), end(), key) != end(); }
        size_t size() const {return values.size(); }

        JSONObject& operator=(JSONObject that) {
            swap(that);
            return *this;
        }

        JSONObject* create() const {
            return new JSONObject();
        }

        JSONObject* clone() const {
            return new JSONObject(*this);
        }

        ~JSONObject() {
            for (auto& pa : values) {
                delete pa.second;
            }

            values.clear();
        }

        std::string to_string() const {
            std::string out = "{";

            for (auto& pa : this->values) {
                out.append(pa.first.to_string());
                out.append(pa.second->to_string());
            }

            out.erase(out.length() - 2);

            out.append("}");
            return out;
        }

    };

}

#endif //TCAT_JSONPP_HPP
