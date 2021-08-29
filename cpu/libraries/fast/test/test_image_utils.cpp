#include "test_image_utils.hpp"
#include "test_utils.hpp"

#include <iostream>

namespace test
{
   namespace image_utils
   {
      void test_all()
      {
         std::cout << "Testing module image_utils..." << std::endl;

         log_test_result(test_gaussian_blur_filter(), "gaussian_blur_filter");
         log_test_result(test_double_threshold(), "double_threshold");
         log_test_result(test_hysteresis(), "hysteresis");
         log_test_result(test_image_interpolation_and_sub(), "image_interpolation_and_sub");
         log_test_result(test_dilation(), "dilation");
         log_test_result(test_downsample(), "downsample");

         std::cout << "Finished tests for module image_utils." << std::endl << std::endl;
      }

      // Test detail functions


      // Test public functions

      bool test_gaussian_blur_filter()
      {
         // Check 0: Normal

         std::vector<unsigned short> data0_in = {
             0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
           255,   0,   0,   0,   0,   0,   0,   0,   0,   0,
             0, 255,   0,   0,   0,   1,   0,   0,   0,   0,
             0,   0, 255,   0,   0,   0,   1,   0,   0,   0,
             0,   0,   0, 255,   0,   0,   2,   3,   0,   0,
             0,   0,   0, 255,   0,   0,   2,   3,   0,   0,
             0,   0, 255,   0,   0,   0,   1,   0,   0,   0,
             0, 255,   0,   0,   0,   1,   0,   0,   0,   0,
           255,   0,   0,   0,   0,   0,   0,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,   0,   0
         };

         std::vector<unsigned short> data0_expected = {
            46,  25,   7,   1,   0,   0,   0,   0,   0,   0,
            84,  58,  27,   7,   1,   0,   0,   0,   0,   0,
            70,  73,  55,  27,   7,   1,   0,   0,   0,   0,
            32,  57,  73,  58,  25,   5,   0,   0,   0,   0,
             8,  35,  73,  82,  44,  10,   0,   0,   0,   0,
             8,  35,  73,  82,  44,  10,   0,   0,   0,   0,
            32,  57,  73,  58,  25,   5,   0,   0,   0,   0,
            70,  73,  55,  27,   7,   1,   0,   0,   0,   0,
            84,  58,  27,   7,   1,   0,   0,   0,   0,   0,
            46,  25,   7,   1,   0,   0,   0,   0,   0,   0,
         };

         motdet::Image<unsigned short> img0_in(data0_in, 10), img0_expected(data0_expected, 10), img0_out(10, 10, 0);

         motdet::imgutil::gaussian_blur_filter(img0_in, img0_out);

         bool test_img0 = test_compare_vectors<unsigned short, unsigned short>(img0_out.get_data(),img0_expected.get_data());
         CHECK_TRUE(test_img0);

         // Check 1: Full saturation

         std::vector<unsigned short> data1_in = {
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255
         };

         std::vector<unsigned short> data1_expected = {
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255
         };

         motdet::Image<unsigned short> img1_in(data1_in, 10), img1_expected(data1_expected, 10), img1_out(10, 10, 0);

         motdet::imgutil::gaussian_blur_filter(img1_in, img1_out);

         bool test_img1 = test_compare_vectors<unsigned short, unsigned short>(img1_out.get_data(),img1_expected.get_data());
         CHECK_TRUE(test_img1);

         // Check 2: Bounded

         std::vector<unsigned short> data2_in = { 3 };
         std::vector<unsigned short> data2_expected = { 3 };

         motdet::Image<unsigned short> img2_in(data2_in, 1);
         motdet::Image<unsigned short> img2_expected(data2_expected, 1), img2_out(1, 1, 0);

         motdet::imgutil::gaussian_blur_filter(img2_in, img2_out);

         bool test_img2 = test_compare_vectors<unsigned short, unsigned short>(img2_out.get_data(),img2_expected.get_data());
         CHECK_TRUE(test_img2);

         return test_img0 && test_img1 && test_img2;
      }

      bool test_double_threshold()
      {
         std::vector<unsigned short> data0_in = {
              0,   0,   0,   0,   0,   0,   0, 255, 255, 255,
              0,  56, 139, 190, 190, 139,  56, 255,   0, 255,
             56, 183, 233, 231, 231, 237, 198, 255, 255, 255,
            139, 233, 155,  89,  89, 142, 233, 158,   0,   0,
            190, 231,  89,  56,  56,  76, 230, 200,   0,   0,
            200, 240,  63,  58,  58, 144, 254, 158,   0,   0,
            190, 231,  58,  70, 134, 255, 226,  70,   0,   0,
            139, 226, 220, 220, 241, 226,  70,   0,   0,   0,
             56, 139, 190, 200, 158,  70,   0,   0,   0,   0,
              0,   0,   0,   0,   0,   0,   0,   0,   0,   0
         };

         std::vector<unsigned char> data0_expected = {
            0,   0,   0,   0,   0,   0,   0,   1,   1,   1,
            0,   0,   0,   2,   2,   0,   0,   1,   0,   1,
            0,   2,   2,   2,   2,   2,   2,   1,   1,   1,
            0,   2,   2,   0,   0,   0,   2,   2,   0,   0,
            2,   2,   0,   0,   0,   0,   2,   2,   0,   0,
            2,   2,   0,   0,   0,   0,   1,   2,   0,   0,
            2,   2,   0,   0,   0,   1,   2,   0,   0,   0,
            0,   2,   2,   2,   2,   2,   0,   0,   0,   0,
            0,   0,   2,   2,   2,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
         };

         motdet::Image<unsigned short> img0_in(data0_in, 10);
         motdet::Image<unsigned char> img0_out(10, 10, 0), img0_expected(data0_expected, 10);

         motdet::imgutil::double_threshold(img0_in, img0_out, 150, 250);

         bool test_img0 = test_compare_vectors<unsigned char, unsigned char>(img0_out.get_data(),img0_expected.get_data());
         CHECK_TRUE(test_img0);

         return test_img0;
      }

      bool test_hysteresis()
      {
         std::vector<unsigned char> data0_in = {
            0,   0,   0,   0,   0,   0,   0,   1,   1,   1,
            0,   0,   0,   2,   2,   0,   0,   1,   0,   1,
            0,   2,   2,   2,   2,   2,   2,   1,   1,   1,
            0,   2,   2,   0,   0,   0,   2,   2,   0,   0,
            2,   2,   0,   0,   0,   0,   2,   2,   0,   0,
            2,   2,   0,   1,   0,   0,   1,   2,   0,   2,
            2,   2,   0,   0,   0,   1,   2,   0,   0,   2,
            0,   2,   2,   2,   2,   2,   0,   0,   2,   2,
            0,   0,   2,   2,   2,   0,   0,   0,   2,   2,
            0,   0,   0,   0,   0,   0,   0,   0,   2,   2,
         };

         std::vector<unsigned char> data0_expected = {
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   1,   1,   0,   0,   1,   0,   0,
            0,   1,   1,   1,   1,   1,   1,   1,   1,   0,
            0,   1,   1,   0,   0,   0,   1,   1,   0,   0,
            0,   1,   0,   0,   0,   0,   1,   1,   0,   0,
            0,   1,   0,   1,   0,   0,   1,   1,   0,   0,
            0,   1,   0,   0,   0,   1,   1,   0,   0,   0,
            0,   1,   1,   1,   1,   1,   0,   0,   0,   0,
            0,   0,   1,   1,   1,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
         };

         motdet::Image<unsigned char> img0_in(data0_in, 10), img0_out(10, 10, 0), img0_expected(data0_expected, 10);

         motdet::imgutil::hysteresis(img0_in, img0_out);

         bool test_img0 = test_compare_vectors<unsigned char, unsigned char>(img0_out.get_data(),img0_expected.get_data());
         CHECK_TRUE(test_img0);

         return test_img0;
      }

      bool test_image_interpolation_and_sub()
      {
         std::vector<unsigned short> data0_in_from = {
            0,   0,   0,   0,   0,   0,   0,
            0,  56, 139, 190, 190, 139,  56,
            56, 183, 233, 231, 231, 237, 198,
            139, 233, 155,  89,  89, 142, 233
         };

         std::vector<unsigned short> data0_in_to = {
            0,   0,   0,  15,   0,   0,   0,
            0,  79, 167,  40, 216,  89, 177,
            79,  79, 236,  71, 185,  43, 206,
            247, 177,  79, 205,  51, 217,  79
         };

         std::vector<unsigned short> data0_expected_07 = {
            0,   0,   0,  10,   0,   0,   0,
            0,  72, 158,  85, 208, 104, 140,
            72, 110, 235, 119, 198, 101, 203,
            214, 193, 101, 170,  62, 194, 125
         };

         std::vector<unsigned short> data0_expected_1 = {
            0,   0,   0,  15,   0,   0,   0,
            0,  79, 167,  40, 216,  89, 177,
            79,  79, 236,  71, 185,  43, 206,
            247, 177,  79, 205,  51, 217,  79
         };

         std::vector<unsigned short> data0_expected_0 = {
            0,   0,   0,   0,   0,   0,   0,
            0,  56, 139, 190, 190, 139,  56,
            56, 183, 233, 231, 231, 237, 198,
            139, 233, 155,  89,  89, 142, 233
         };

         std::vector<unsigned short> data0_expected_subbed = {
            0, 0, 0, 15, 0, 0, 0,
            0, 23, 28, 150, 26, 50, 121,
           23, 104, 3, 160, 46, 194, 8,
          108, 56, 76, 116, 38, 75, 154,
         };

         motdet::Image<unsigned short> img0_in_from(data0_in_from, 7), img0_in_to(data0_in_to, 7), img0_out_07(7, 4, 0), img0_out_0(7, 4, 0), img0_out_1(7, 4, 0), img0_out_subbed(7, 4, 0), img0_expected_07(data0_expected_07, 7), img0_expected_0(data0_expected_0, 7), img0_expected_1(data0_expected_1, 7), img0_expected_subbed(data0_expected_subbed, 7);

         motdet::imgutil::image_interpolation_and_sub(img0_in_from, img0_in_to, img0_out_07, img0_out_subbed, 0.7);

         bool test_img0_subbed_0 = test_compare_vectors<unsigned short, unsigned short>(img0_out_subbed.get_data(), img0_expected_subbed.get_data());
         CHECK_TRUE(test_img0_subbed_0);

         motdet::imgutil::image_interpolation_and_sub(img0_in_from, img0_in_to, img0_out_0, img0_out_subbed, 0);

         bool test_img0_subbed_07 = test_compare_vectors<unsigned short, unsigned short>(img0_out_subbed.get_data(), img0_expected_subbed.get_data());
         CHECK_TRUE(test_img0_subbed_07);

         motdet::imgutil::image_interpolation_and_sub(img0_in_from, img0_in_to, img0_out_1, img0_out_subbed, 1);

         bool test_img0_subbed_1 = test_compare_vectors<unsigned short, unsigned short>(img0_out_subbed.get_data(), img0_expected_subbed.get_data());
         CHECK_TRUE(test_img0_subbed_1);

         bool test_img0_07 = test_compare_vectors<unsigned short, unsigned short>(img0_out_07.get_data(),img0_expected_07.get_data());
         CHECK_TRUE(test_img0_07);
         bool test_img0_0 = test_compare_vectors<unsigned short, unsigned short>(img0_out_0.get_data(),img0_expected_0.get_data());
         CHECK_TRUE(test_img0_0);
         bool test_img0_1 = test_compare_vectors<unsigned short, unsigned short>(img0_out_1.get_data(),img0_expected_1.get_data());
         CHECK_TRUE(test_img0_1);

         bool test_img0 = test_img0_07 && test_img0_0 && test_img0_1 && test_img0_subbed_0 && test_img0_subbed_07 && test_img0_subbed_1;

         return test_img0;

        return false;
      }

      bool test_dilation()
      {
         std::vector<unsigned char> data0_in = {
            0,   0,   0,   0,   0,   0,   0,   1,   1,   1,
            0,   0,   0,   1,   1,   0,   0,   1,   0,   1,
            0,   1,   1,   1,   1,   1,   1,   1,   0,   1,
            0,   1,   1,   0,   0,   0,   1,   1,   0,   0,
            1,   1,   0,   0,   0,   0,   1,   1,   0,   0,
            1,   1,   0,   0,   0,   0,   1,   1,   0,   0,
            1,   1,   0,   0,   0,   1,   1,   0,   0,   0,
            0,   1,   1,   1,   1,   1,   0,   0,   0,   1,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   1,
            0,   0,   0,   0,   0,   0,   0,   0,   1,   1,
         };

         std::vector<unsigned char> data0_expected = {
            0,   0,   1,   1,   1,   1,   1,   1,   1,   1,
            1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
            1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
            1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
            1,   1,   1,   1,   0,   1,   1,   1,   1,   0,
            1,   1,   1,   0,   1,   1,   1,   1,   1,   0,
            1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
            1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
            1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
            0,   0,   0,   0,   0,   0,   0,   1,   1,   1,
         };

         motdet::Image<unsigned char> img0_in(data0_in, 10), img0_out(10, 10, 0), img0_expected(data0_expected, 10);

         motdet::imgutil::dilation(img0_in, img0_out);

         bool test_img0 = test_compare_vectors<unsigned char, unsigned char>(img0_out.get_data(),img0_expected.get_data());
         CHECK_TRUE(test_img0);

         return test_img0;
      }

      bool test_downsample()
      {
         std::vector<unsigned short> data0_in = {
            0,   0,   0,   0,   0,   0,   0,   6,
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,  40,  50,  50,  40,   0,   0,
            0,  40,  50,  60,  60,  50,  50,   0,
            0,  50,  60,  70,  70,  60,  50,   0,
            0,  50,  60,  70,  70,  60,  50,   0,
            0,  50,  70,  50,  60,  50,   0,   0,
         };

         std::vector<unsigned short> data0_expected = {
            4,  15,   1,
           34,  63,  25,
           40,  53,   0
         };

         motdet::Image<unsigned short> img0_in(data0_in, 8), img0_out(3, 3, 0), img0_expected(data0_expected, 3);

         motdet::imgutil::downsample(img0_in, img0_out, 3);

         bool test_img0 = test_compare_vectors<unsigned short, unsigned short>(img0_out.get_data(),img0_expected.get_data());
         CHECK_TRUE(test_img0);

         return test_img0;
      }

   } // namespace test
} // namespace image_utils
