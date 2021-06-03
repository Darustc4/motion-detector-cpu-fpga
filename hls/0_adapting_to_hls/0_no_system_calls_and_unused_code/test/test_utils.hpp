#ifndef __TEST_UTILS_HPP__
#define __TEST_UTILS_HPP__

#include <cstring>
#include <iostream>
#include <iomanip>

namespace test
{

    #define CHECK_TRUE(x) { if (!(x)) std::cout << __FUNCTION__ << " failed on line " << __LINE__ << std::endl; }

    inline void log_test_result(bool passed, std::string func_name)
    {
        if(passed) std::cout << "[PASS] : " << func_name << std::endl;
        else std::cout << "> [FAIL] : " << func_name << std::endl;
    }

    template<typename T1, typename T2>
    bool test_compare_vectors(const std::vector<T1> &v1, const std::vector<T2> &v2)
    {
        if(&v1 == &v2) return true;

        if(v1.size() != v2.size()){
            std::cout << "Vector compare: Different sizes" << std::endl;
            return false;
        }

        for(std::size_t i = 0; i < v1.size(); ++i)
            if(v1[i] != v2[i]){
                std::cout << "Vector compare: At index " << i << ", " << (int)v1[i] << " != " << (int)v2[i] << std::endl;
                return false;
            }
        return true;
    }

    template<typename T>
    void print_image(const motdet::Image<T> img, unsigned int spacing)
    {
        std::size_t height = img.get_height(), width = img.get_width();
        for(std::size_t i = 0; i < height; ++i)
        {
            for(std::size_t j = 0; j < width; ++j)
            {
                std::cout << std::setw(spacing) << (long long)img[i*width + j] << ", ";
            }
            std::cout << std::endl;
        }
    }

} // namespace test

#endif // __TEST_UTILS_HPP__
