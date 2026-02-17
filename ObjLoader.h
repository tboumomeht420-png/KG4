#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <DirectXMath.h>

using namespace DirectX;

class ObjLoader
{
public:
    struct ObjVertex
    {
        XMFLOAT3 position;
        XMFLOAT3 normal;
        XMFLOAT2 texCoord;
    };

    static bool LoadObjFile(const std::string& filename,
        std::vector<XMFLOAT3>& positions,
        std::vector<XMFLOAT3>& normals,
        std::vector<uint32_t>& indices)
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            return false;
        }

        std::vector<XMFLOAT3> tempPositions;
        std::vector<XMFLOAT3> tempNormals;
        std::vector<XMFLOAT2> tempTexCoords;

        struct FaceVertex
        {
            int posIndex;
            int texIndex;
            int normIndex;

            bool operator==(const FaceVertex& other) const
            {
                return posIndex == other.posIndex &&
                    texIndex == other.texIndex &&
                    normIndex == other.normIndex;
            }
        };

        struct FaceVertexHash
        {
            size_t operator()(const FaceVertex& fv) const
            {
                return std::hash<int>()(fv.posIndex) ^
                    (std::hash<int>()(fv.texIndex) << 1) ^
                    (std::hash<int>()(fv.normIndex) << 2);
            }
        };

        std::unordered_map<FaceVertex, uint32_t, FaceVertexHash> vertexMap;

        std::string line;
        while (std::getline(file, line))
        {
            if (line.empty() || line[0] == '#')
                continue;

            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;

            if (prefix == "v")
            {
                XMFLOAT3 pos;
                iss >> pos.x >> pos.y >> pos.z;
                tempPositions.push_back(pos);
            }
            else if (prefix == "vn")
            {
                XMFLOAT3 norm;
                iss >> norm.x >> norm.y >> norm.z;
                tempNormals.push_back(norm);
            }
            else if (prefix == "vt")
            {
                XMFLOAT2 tex;
                iss >> tex.x >> tex.y;
                tempTexCoords.push_back(tex);
            }
            else if (prefix == "f")
            {
                std::string vertex1, vertex2, vertex3;
                iss >> vertex1 >> vertex2 >> vertex3;

                auto parseFaceVertex = [](const std::string& str) -> FaceVertex
                    {
                        FaceVertex fv = { -1, -1, -1 };
                        size_t pos1 = str.find('/');

                        if (pos1 == std::string::npos)
                        {
                            fv.posIndex = std::stoi(str);
                        }
                        else
                        {
                            fv.posIndex = std::stoi(str.substr(0, pos1));
                            size_t pos2 = str.find('/', pos1 + 1);

                            if (pos2 != std::string::npos)
                            {
                                if (pos2 > pos1 + 1)
                                    fv.texIndex = std::stoi(str.substr(pos1 + 1, pos2 - pos1 - 1));
                                if (pos2 + 1 < str.length())
                                    fv.normIndex = std::stoi(str.substr(pos2 + 1));
                            }
                            else if (pos1 + 1 < str.length())
                            {
                                fv.texIndex = std::stoi(str.substr(pos1 + 1));
                            }
                        }
                        return fv;
                    };

                FaceVertex fv1 = parseFaceVertex(vertex1);
                FaceVertex fv2 = parseFaceVertex(vertex2);
                FaceVertex fv3 = parseFaceVertex(vertex3);

                auto processVertex = [&](const FaceVertex& fv)
                    {
                        auto it = vertexMap.find(fv);
                        if (it != vertexMap.end())
                        {
                            indices.push_back(it->second);
                        }
                        else
                        {
                            int posIdx = fv.posIndex - 1;
                            int normIdx = fv.normIndex - 1;

                            if (posIdx >= 0 && posIdx < tempPositions.size())
                            {
                                uint32_t newIndex = static_cast<uint32_t>(positions.size());
                                positions.push_back(tempPositions[posIdx]);


                                if (normIdx >= 0 && normIdx < tempNormals.size())
                                {
                                    normals.push_back(tempNormals[normIdx]);
                                }
                                else
                                {

                                    normals.push_back(XMFLOAT3(0.0f, 1.0f, 0.0f));
                                }

                                vertexMap[fv] = newIndex;
                                indices.push_back(newIndex);
                            }
                        }
                    };

                processVertex(fv1);
                processVertex(fv2);
                processVertex(fv3);
            }
        }

        file.close();
        return !positions.empty();
    }

};
