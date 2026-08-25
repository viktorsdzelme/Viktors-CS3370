// Name: Viktors Dzelme
//Reflection: This assigment I had scope confusion between the class and function bodies and a lot of syntax errors such as missing semicolons and brace/parenthesis mix ups. Very needed review assignment for me
#include <stdexcept>
#ifndef RECTANGLE_H
#define RECTANGLE_H

class Rectangle {
    public:
        Rectangle(double length, double width){
            setLength(length);
            setWidth(width);
        }    
    

        double getLength() const{
            return _length;
        }
        void setLength(double length){
            if (length <= 0){
                throw std::invalid_argument("length must be positive");
            }
            _length = length;
        }

        double getWidth() const{
            return _width;
        }
        void setWidth(double width){
            if (width <= 0){
            throw std::invalid_argument("width must be positive");
            }
            _width = width;
        }

    private:
        double _length;
        double _width;
};

#endif