#include <iostream>

#include "test_image_utils.hpp"
#include "test_motion_detector.hpp"
#include "test_contour_detector.hpp"

int main()
{
    std::cout << "Starting all module tests..." << std::endl << std::endl;

    test::image_utils::test_all();
    test::motion_detector::test_all();
    test::contour_detector::test_all();

    std::cout << "Finished all module tests." << std::endl;

    return 0;
}
