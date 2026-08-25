// Name: Viktors Dzelme
//Reflection: This assigment I had scope confusion between the class and function bodies and a lot of syntax errors such as missing semicolons and brace/parenthesis mix ups. Very needed review assignment for me

#include <iostream>
#include "rectangularfrustrum.h"

int main() {
    double lTop, wTop, lBottom, wBottom, height;

    std::cout << "Enter top rectangle length: ";
    std::cin >> lTop;
    std::cout << "Enter top rectangle width: ";
    std::cin >> wTop;
    std::cout << "Enter bottom rectangle length: ";
    std::cin >> lBottom;
    std::cout << "Enter bottom rectangle width: ";
    std::cin >> wBottom;
    std::cout << "Enter height: ";
    std::cin >> height;


    try {
        RectangularFrustrum frustrum(lTop, wTop, lBottom, wBottom, height);
        std::cout << "Volume: " << frustrum.getVolume() << std::endl;
        std::cout << "Surface Area: " << frustrum.getSurfaceArea() << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }

    return 0;
}