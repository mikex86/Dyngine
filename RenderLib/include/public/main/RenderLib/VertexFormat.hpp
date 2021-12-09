#pragma once

#include <string>
#include <vector>

namespace RenderLib {

    enum VertexAttributeType {
        UBYTE,
        BYTE,
        USHORT,
        SHORT,
        INT,
        UINT,
        FLOAT,
        DOUBLE,
        MAT2,
        MAT3,
        MAT4,
        UBYTE_VEC2,
        UBYTE_VEC3,
        UBYTE_VEC4,
        BYTE_VEC2,
        BYTE_VEC3,
        BYTE_VEC4,
        USHORT_VEC2,
        USHORT_VEC3,
        USHORT_VEC4,
        SHORT_VEC2,
        SHORT_VEC3,
        SHORT_VEC4,
        INT_VEC2,
        INT_VEC3,
        INT_VEC4,
        UINT_VEC2,
        UINT_VEC3,
        UINT_VEC4,
        FLOAT_VEC2,
        FLOAT_VEC3,
        FLOAT_VEC4,
        DOUBLE_VEC2,
        DOUBLE_VEC3,
        DOUBLE_VEC4
    };

    size_t GetAttributeTypeSize(VertexAttributeType type);

    class VertexAttribute {
    private:
        uint32_t location;
        std::string name;
        VertexAttributeType dataType;
        size_t offset;
    public:
        VertexAttribute(uint32_t location, std::string name, VertexAttributeType type, size_t offset);

        virtual ~VertexAttribute();

        [[nodiscard]] const std::string &getName() const;

        [[nodiscard]] VertexAttributeType getDataType() const;

        [[nodiscard]] size_t getOffset() const;

        [[nodiscard]] uint32_t getLocation() const;
    };

    class VertexFormat {

    private:
        std::vector<VertexAttribute> attributes;

    public:
        void appendAttribute(const std::string &name, VertexAttributeType type);

        [[nodiscard]] const std::vector<VertexAttribute> &getAttributes() const;

        [[nodiscard]] size_t getStride() const;

    };

}