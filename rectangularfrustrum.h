// Name: Viktors Dzelme
//Reflection: This assigment I had scope confusion between the class and function bodies and a lot of syntax errors such as missing semicolons and brace/parenthesis mix ups. Very needed review assignment for me
#include "rectangle.h"
#include <cmath>
#include <stdexcept>
#ifndef RECTANGULARFRUSTRUM_H
#define RECTANGULARFRUSTRUM_H

class RectangularFrustrum{
    public:
        RectangularFrustrum(double lTop, double wTop, double lBottom, double wBottom, double height)
        : _rTop(lTop, wTop), _rBottom(lBottom, wBottom)
        {
        if (height <= 0) {
            throw std::invalid_argument("height must be positive");
        }
        _height = height;
        }

        double getVolume() const {
            double A1 = _rTop.getLength() * _rTop.getWidth();
            double A2 = _rBottom.getLength() * _rBottom.getWidth();
            double V = (_height/3.0) * (A1 + A2 + sqrt(A1 * A2));
            return V;
        }

        double getSurfaceArea() const {
             double SA = (_rTop.getLength() * _rTop.getWidth()) + (_rBottom.getLength() * _rBottom.getWidth()) + _height * (_rTop.getLength() + _rBottom.getLength()) + _height * (_rTop.getWidth() + _rBottom.getWidth());
             return SA;
        }
    private:
        Rectangle _rTop;
        Rectangle _rBottom;
        double _height;
};
    #endif