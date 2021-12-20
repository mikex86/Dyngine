#include <iostream>
#include <Stream/FileDataReadStream.hpp>
#include <DAsset/Asset.hpp>

void Print(const DAsset::Asset &asset);

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <dasset_file>" << std::endl;
        return 1;
    }

    DAsset::Asset asset = DAsset::ReadAsset(Stream::FileDataReadStream::Open(argv[1]));
    Print(asset);
    return 0;
}

void Indent(int depth) {
    for (int i = 0; i < depth; i++) {
        std::cout << "\t";
    }
}

void PrintNode(int depth, const DAsset::Node &node) {
    std::cout << "Node: " << node.name << std::endl;
    Indent(depth);
    std::cout << "Translation: (" << node.translation.x << ", " << node.translation.y << ", " << node.translation.z
              << ")" << std::endl;
    Indent(depth);
    std::cout << "Rotation: (" << node.rotation.x << ", " << node.rotation.y << ", " << node.rotation.z << ", "
              << node.rotation.w << ")" << std::endl;
    Indent(depth);
    std::cout << "Scale: (" << node.scale.x << ", " << node.scale.y << ", " << node.scale.z << ")" << std::endl;
    Indent(depth);
    std::cout << "Mesh: " << std::endl;
    for (auto &meshPart: node.mesh.meshParts) {
        Indent(depth + 1);
        std::cout << "MeshPart: " << std::endl;
        Indent(depth + 2);
        std::cout << "Attributes: " << std::endl;
        for (auto &[attributeType, bufferView]: meshPart.attributeBufferViews) {
            Indent(depth + 3);
            std::cout << "Attribute: " << DAsset::GetAttributeTypeName(attributeType) << std::endl;
            Indent(depth + 3);
            std::cout << "BufferView: " << std::endl;
            Indent(depth + 4);
            std::cout << "DataType: " << DAsset::GetDataTypeName(bufferView.dataType) << std::endl;
            Indent(depth + 4);
            std::cout << "ComponentType: " << DAsset::GetComponentTypeName(bufferView.componentType) << std::endl;
            Indent(depth + 4);
            std::cout << "ByteOffset: " << bufferView.offset << std::endl;
            Indent(depth + 4);
            std::cout << "ByteLength: " << bufferView.length << std::endl;
            Indent(depth + 4);
            std::cout << "ByteStride: " << bufferView.stride << std::endl;
        }
    }

    if (!node.children.empty()) {
        Indent(depth);
        std::cout << "Children:" << std::endl;
        for (const DAsset::Node &child: node.children) {
            PrintNode(depth + 1, child);
        }
    }
}

void Print(const DAsset::Asset &asset) {
    PrintNode(0, asset.rootNode);
}
