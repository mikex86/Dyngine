#include <RenderLib/VertexFormat.hpp>
#include <utility>

namespace RenderLib {

    VertexAttribute::VertexAttribute(uint32_t location, std::string name, VertexAttributeType type,
                                     size_t offset) :
            location(location),
            name(std::move(name)),
            dataType(type),
            offset(offset) {
    }

    void VertexFormat::appendAttribute(const std::string &name, VertexAttributeType type) {
        size_t offset = 0;
        if (!attributes.empty()) {
            auto back = attributes.back();
            offset = back.getOffset() + GetAttributeTypeSize(back.getDataType());
        }
        auto location = static_cast<uint32_t>(attributes.size());
        attributes.emplace_back(location, name, type, offset);
    }

    const std::vector<VertexAttribute> &VertexFormat::getAttributes() const {
        return attributes;
    }

    size_t VertexFormat::getStride() const {
        if (attributes.empty()) {
            return 0;
        }
        return attributes.back().getOffset() + GetAttributeTypeSize(attributes.back().getDataType());
    }

    const std::string &VertexAttribute::getName() const {
        return name;
    }

    VertexAttributeType VertexAttribute::getDataType() const {
        return dataType;
    }

    size_t VertexAttribute::getOffset() const {
        return offset;
    }

    uint32_t VertexAttribute::getLocation() const {
        return location;
    }

    VertexAttribute::~VertexAttribute() = default;

    size_t GetAttributeTypeSize(VertexAttributeType type) {
        switch (type) {
            case VertexAttributeType::UBYTE:
            case VertexAttributeType::BYTE:
                return 1;
            case VertexAttributeType::UBYTE_VEC2:
            case VertexAttributeType::BYTE_VEC2:
                return 2;
            case VertexAttributeType::UBYTE_VEC3:
            case VertexAttributeType::BYTE_VEC3:
                return 3;
            case VertexAttributeType::UBYTE_VEC4:
            case VertexAttributeType::BYTE_VEC4:
                return 4;
            case VertexAttributeType::USHORT:
            case VertexAttributeType::SHORT:
                return 2;
            case VertexAttributeType::USHORT_VEC2:
            case VertexAttributeType::SHORT_VEC2:
                return 4;
            case VertexAttributeType::USHORT_VEC3:
            case VertexAttributeType::SHORT_VEC3:
                return 6;
            case VertexAttributeType::USHORT_VEC4:
            case VertexAttributeType::SHORT_VEC4:
                return 8;
            case VertexAttributeType::INT:
            case VertexAttributeType::UINT:
                return 4;
            case VertexAttributeType::INT_VEC2:
            case VertexAttributeType::UINT_VEC2:
                return 8;
            case VertexAttributeType::INT_VEC3:
            case VertexAttributeType::UINT_VEC3:
                return 12;
            case VertexAttributeType::INT_VEC4:
            case VertexAttributeType::UINT_VEC4:
                return 16;
            case VertexAttributeType::FLOAT:
                return 4;
            case VertexAttributeType::FLOAT_VEC2:
                return 8;
            case VertexAttributeType::FLOAT_VEC3:
                return 12;
            case VertexAttributeType::FLOAT_VEC4:
                return 16;
            case VertexAttributeType::DOUBLE:
                return 8;
            case VertexAttributeType::DOUBLE_VEC2:
                return 16;
            case VertexAttributeType::DOUBLE_VEC3:
                return 24;
            case VertexAttributeType::DOUBLE_VEC4:
                return 32;
            case VertexAttributeType::MAT2:
                return 4 * 2 * 2;
            case VertexAttributeType::MAT3:
                return 4 * 3 * 3;
            case VertexAttributeType::MAT4:
                return 4 * 4 * 4;
            default:
                return 0;
        }
    }
}