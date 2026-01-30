#pragma once

#include <string>
#include <vector>
#include <map>

class TopoDS_Shape;

namespace Base
{
    class TextShape
    {
    public:
        static std::vector<std::string> InitOcctFonts();
        static std::map<std::string, std::string> GetOcctFontsMaps();
        static bool MakeTextShape(const char* text, const char* font, const float textHeight, const float thickness, bool isBold, bool isItalic, TopoDS_Shape& resultShape, double& dTextWidth);
    };
}

//struct TextResult
//{
//    TriangleMesh text_mesh;
//    double       text_width;
//};


