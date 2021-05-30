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

         log_test_result(test_detail_square_convolution(), "square_convolution");
         log_test_result(test_detail_vline_convolution(), "vline_convolution");
         log_test_result(test_detail_hline_convolution(), "hline_convolution");
         log_test_result(test_gaussian_blur_filter(), "gaussian_blur_filter");
         log_test_result(test_median_filter(), "median_filter");
         log_test_result(test_image_subtraction(), "image_subtraction");
         log_test_result(test_canny_edge_detection_8b(), "canny_edge_detection_8b");
         log_test_result(test_sobel_edge_detection_8b(), "sobel_edge_detection_8b");
         log_test_result(test_non_max_suppression(), "non_max_suppression");
         log_test_result(test_double_threshold(), "double_threshold");
         log_test_result(test_single_threshold(), "single_threshold");
         log_test_result(test_hysteresis(), "hysteresis");
         log_test_result(test_image_interpolation(), "image_interpolation");
         log_test_result(test_dilation(), "dilation");
         log_test_result(test_erosion(), "erosion");
         log_test_result(test_reescale_pix_length(), "reescale_pix_length");
         log_test_result(test_downsample(), "downsample");
         log_test_result(test_upsample(), "upsample");

         std::cout << "Finished tests for module image_utils." << std::endl << std::endl;
      }

      // Test detail functions

      bool test_detail_square_convolution()
      {
         // Very small kernel

         std::vector<unsigned char> data0_in = {
            0,   0,   0,  15,   0,   0,   0,
            0,  79, 167,  40, 216,  89, 177,
            79,  79, 236,  71, 185,  43, 206,
            247, 177,  79, 205,  51, 217,  79
         };

         std::vector<unsigned char> data0_expected = {
            255, 255, 255, 240, 255, 255, 255,
            255, 176,  88, 215,  39, 166,  78,
            176, 176,  19, 184,  70, 212,  49,
            8,  78, 176,  50, 204,  38, 176,
         };

         auto negative_filter = [](std::array<unsigned char, 1> &pix)
         {
            return 255 - pix[0];
         };

         motdet::Image<unsigned char> img0_in(data0_in, 7), img0_out(7, 4, 0), img0_expected(data0_expected, 7);

         motdet::imgutil::detail::square_convolution<unsigned char, unsigned char, 1>(img0_in, img0_out, negative_filter);

         bool test_img0 = test_compare_vectors<unsigned char, unsigned char>(img0_out.get_data(),img0_expected.get_data());
         CHECK_TRUE(test_img0);

         // Very wide kernel

         std::vector<unsigned char> data1_in = {
            0,    0,    0,   15,    0,    0,    0,
            0,   79,  167,   40,  216,   89,  177,
            79,   79,  236,   71,  185,   43,  206,
            247,  177,   79,  205,   51,  217,   79
         };

         std::vector<unsigned short> data1_expected = {
         2497, 2623, 2646, 2782, 2918, 3045, 3025,
         3931, 3861, 3854, 3822, 3790, 3819, 3799,
         5365, 5099, 5062, 4862, 4662, 4593, 4573,
         6799, 6337, 6270, 5902, 5534, 5367, 5347
         };

         // Using 7x7 kernel which does not fit in the input matrix
         auto sum_filter = [](std::array<unsigned char, 49> &pix)
         {
            unsigned short tot = 0;
            for(unsigned char p : pix) tot += p;
            return tot;
         };

         motdet::Image<unsigned char> img1_in(data1_in, 7);
         motdet::Image<unsigned short> img1_out(7, 4, 0), img1_expected(data1_expected, 7);

         motdet::imgutil::detail::square_convolution<unsigned char, unsigned short, 49>(img1_in, img1_out, sum_filter);

         bool test_img1 = test_compare_vectors<unsigned short, unsigned short>(img1_out.get_data(),img1_expected.get_data());
         CHECK_TRUE(test_img1);

         return test_img0 && test_img1;
      }

      bool test_detail_vline_convolution()
      {
         // Very small kernel

         std::vector<unsigned char> data0_in = {
            0,   0,   0,  15,   0,   0,   0,
            0,  79, 167,  40, 216,  89, 177,
            79,  79, 236,  71, 185,  43, 206,
            247, 177,  79, 205,  51, 217,  79
         };

         std::vector<unsigned char> data0_expected = {
            255, 255, 255, 240, 255, 255, 255,
            255, 176,  88, 215,  39, 166,  78,
            176, 176,  19, 184,  70, 212,  49,
            8,  78, 176,  50, 204,  38, 176,
         };

         auto negative_filter = [](std::array<unsigned char, 1> &pix)
         {
            return 255 - pix[0];
         };

         motdet::Image<unsigned char> img0_in(data0_in, 7), img0_out(7, 4, 0), img0_expected(data0_expected, 7);

         motdet::imgutil::detail::vline_convolution<unsigned char, unsigned char, 1>(img0_in, img0_out, negative_filter);

         bool test_img0 = test_compare_vectors<unsigned char, unsigned char>(img0_out.get_data(),img0_expected.get_data());
         CHECK_TRUE(test_img0);

         // Very wide kernel

         std::vector<unsigned char> data1_in = {
              0,    0,    0,   15,    0,    0,    0,
              0,   79,  167,   40,  216,   89,  177,
             79,   79,  236,   71,  185,   43,  206,
            247,  177,   79,  205,   51,  217,   79
         };

         std::vector<unsigned short> data1_expected = {
           326,  335,  482,  376,  452,  349,  462,
           573,  512,  561,  566,  503,  566,  541,
           820,  689,  640,  756,  554,  783,  620,
          1067,  866,  719,  946,  605, 1000,  699,
         };

         // Using 7pix high kernel which does not fit in the input matrix
         auto sum_filter = [](std::array<unsigned char, 7> &pix)
         {
            unsigned short tot = 0;
            for(unsigned char p : pix) tot += p;
            return tot;
         };

         motdet::Image<unsigned char> img1_in(data1_in, 7);
         motdet::Image<unsigned short> img1_out(7, 4, 0), img1_expected(data1_expected, 7);

         motdet::imgutil::detail::vline_convolution<unsigned char, unsigned short, 7>(img1_in, img1_out, sum_filter);

         bool test_img1 = test_compare_vectors<unsigned short, unsigned short>(img1_out.get_data(),img1_expected.get_data());
         CHECK_TRUE(test_img1);

         return test_img0 && test_img1;
      }

      bool test_detail_hline_convolution()
      {
         // Very small kernel

         std::vector<unsigned char> data0_in = {
            0,   0,   0,  15,   0,   0,   0,
            0,  79, 167,  40, 216,  89, 177,
           79,  79, 236,  71, 185,  43, 206,
          247, 177,  79, 205,  51, 217,  79
         };

         std::vector<unsigned char> data0_expected = {
          255, 255, 255, 240, 255, 255, 255,
          255, 176,  88, 215,  39, 166,  78,
          176, 176,  19, 184,  70, 212,  49,
            8,  78, 176,  50, 204,  38, 176,
         };

         auto negative_filter = [](std::array<unsigned char, 1> &pix)
         {
            return 255 - pix[0];
         };

         motdet::Image<unsigned char> img0_in(data0_in, 7), img0_out(7, 4, 0), img0_expected(data0_expected, 7);

         motdet::imgutil::detail::hline_convolution<unsigned char, unsigned char, 1>(img0_in, img0_out, negative_filter);

         bool test_img0 = test_compare_vectors<unsigned char, unsigned char>(img0_out.get_data(),img0_expected.get_data());
         CHECK_TRUE(test_img0);

         // Very wide kernel

         std::vector<unsigned char> data1_in = {
             0,    0,    0,   15,    0,    0,    0,
             0,   79,  167,   40,  216,   89,  177,
            79,   79,  236,   71,  185,   43,  206,
           247,  177,   79,  205,   51,  217,   79
         };

         std::vector<unsigned short> data1_expected = {
            15,   15,   15,   15,   15,   15,   15,
           591,  768,  945, 1122, 1299, 1476, 1653,
          1088, 1215, 1342, 1469, 1596, 1723, 1850,
          2211, 2043, 1875, 1707, 1539, 1371, 1203,
         };

         // Using 11pix wide kernel which does not fit in the input matrix
         auto sum_filter = [](std::array<unsigned char, 11> &pix)
         {
            unsigned short tot = 0;
            for(unsigned char p : pix) tot += p;
            return tot;
         };

         motdet::Image<unsigned char> img1_in(data1_in, 7);
         motdet::Image<unsigned short> img1_out(7, 4, 0), img1_expected(data1_expected, 7);

         motdet::imgutil::detail::hline_convolution<unsigned char, unsigned short, 11>(img1_in, img1_out, sum_filter);

         bool test_img1 = test_compare_vectors<unsigned short, unsigned short>(img1_out.get_data(),img1_expected.get_data());
         CHECK_TRUE(test_img1);

         return test_img0 && test_img1;
      }

      // Test public functions

      bool test_gaussian_blur_filter()
      {
         // Check 0: Normal

         std::vector<unsigned char> data0_in = {
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

         std::vector<unsigned char> data0_expected = {
            47,  25,   7,   0,   0,   0,   0,   0,   0,   0,
            84,  58,  27,   7,   1,   0,   0,   0,   0,   0,
            71,  73,  56,  27,   7,   1,   0,   0,   0,   0,
            32,  57,  73,  58,  25,   5,   0,   0,   0,   0,
             8,  35,  73,  82,  44,  10,   1,   1,   0,   0,
             8,  35,  73,  82,  44,  10,   1,   1,   0,   0,
            32,  57,  73,  58,  25,   5,   0,   0,   0,   0,
            71,  73,  56,  27,   7,   1,   0,   0,   0,   0,
            84,  58,  27,   7,   1,   0,   0,   0,   0,   0,
            47,  25,   7,   0,   0,   0,   0,   0,   0,   0,
         };

         motdet::Image<unsigned char> img0_in(data0_in, 10), img0_expected(data0_expected, 10), img0_out(10, 10, 0);

         motdet::imgutil::gaussian_blur_filter<unsigned char, unsigned char>(img0_in, img0_out);

         bool test_img0 = test_compare_vectors<unsigned char, unsigned char>(img0_out.get_data(),img0_expected.get_data());
         CHECK_TRUE(test_img0);

         // Check 1: Full saturation

         std::vector<long> data1_in = {
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

         std::vector<unsigned char> data1_expected = {
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

         motdet::Image<long> img1_in(data1_in, 10);
         motdet::Image<unsigned char> img1_expected(data1_expected, 10), img1_out(10, 10, 0);

         motdet::imgutil::gaussian_blur_filter<long, unsigned char>(img1_in, img1_out);

         bool test_img1 = test_compare_vectors<unsigned char, unsigned char>(img1_out.get_data(),img1_expected.get_data());
         CHECK_TRUE(test_img1);

         // Check 2: Bounded

         std::vector<unsigned char> data2_in = { 3 };
         std::vector<char> data2_expected = { 3 };

         motdet::Image<unsigned char> img2_in(data2_in, 1);
         motdet::Image<char> img2_expected(data2_expected, 1), img2_out(1, 1, 0);

         motdet::imgutil::gaussian_blur_filter<unsigned char, char>(img2_in, img2_out);

         bool test_img2 = test_compare_vectors<char, char>(img2_out.get_data(),img2_expected.get_data());
         CHECK_TRUE(test_img2);

         return test_img0 && test_img1 && test_img2;
      }

      bool test_median_filter()
      {
         // Check 0: Normal

         std::vector<unsigned char> data0_in = {
            0,   0,   0,   0,   0,   0,   0,   0,   3,   3,
          255,   1,   1,   0,   0,   0,   0,   0,   3,   3,
            0, 255,   1,   0,   0,   1,   0,   0,   0,   0,
            0,   0, 255,   0,   0,   0,   1,   0,   0,   0,
            0,   0,   0, 255,   0,   0,   2,   3,   0,   0,
            0,   0,   0, 255,   0,   0,   2,   3,   0,   0,
            0,   0, 255,   0,   0,   0,   1,   0,   0,   0,
            0, 255,   0,   0,   0,   1,   0,   0,   0,   0,
          255,   0,   0,   0,   0,   0,   0,   0,   3,   3,
            0,   0,   0,   0,   0,   0,   0,   0,   3,   3
         };

         std::vector<unsigned char> data0_expected = {
            0,   0,   0,   0,   0,   0,   0,   0,   3,   3,
            0,   1,   0,   0,   0,   0,   0,   0,   0,   3,
            0,   1,   1,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   1,   1,   0,   0,
            0,   0,   0,   0,   0,   0,   1,   1,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   3,
            0,   0,   0,   0,   0,   0,   0,   0,   3,   3,
         };

         motdet::Image<unsigned char> img0_in(data0_in, 10), img0_expected(data0_expected, 10), img0_out(10, 10, 0);

         motdet::imgutil::median_filter<unsigned char, unsigned char>(img0_in, img0_out);

         bool test_img0 = test_compare_vectors<unsigned char, unsigned char>(img0_out.get_data(),img0_expected.get_data());
         CHECK_TRUE(test_img0);

         // Check 1: Full saturation

         std::vector<long> data1_in = {
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

         std::vector<unsigned char> data1_expected = {
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

         motdet::Image<long> img1_in(data1_in, 10);
         motdet::Image<unsigned char> img1_expected(data1_expected, 10), img1_out(10, 10, 0);

         motdet::imgutil::median_filter<long, unsigned char>(img1_in, img1_out);

         bool test_img1 = test_compare_vectors<unsigned char, unsigned char>(img1_out.get_data(),img1_expected.get_data());
         CHECK_TRUE(test_img1);

         // Check 2: Bounded

         std::vector<unsigned char> data2_in = {
            3, 2,
            2, 2
         };
         std::vector<char> data2_expected = {
            2, 2,
            2, 2
         };

         motdet::Image<unsigned char> img2_in(data2_in, 2);
         motdet::Image<char> img2_expected(data2_expected, 2), img2_out(2, 2, 0);

         motdet::imgutil::median_filter<unsigned char, char>(img2_in, img2_out);

         bool test_img2 = test_compare_vectors<char, char>(img2_out.get_data(),img2_expected.get_data());
         CHECK_TRUE(test_img2);

         return test_img0 && test_img1 && test_img2;
      }

      bool test_image_subtraction()
      {
         // Check 0: Normal

         std::vector<unsigned char> data0_in0 = {
            0,   0,  20,   0,
          255,   1,   1,   0,
            0, 255,   1,   0,
            0,   0, 255,   0

         };

         std::vector<unsigned char> data0_in1 = {
          255, 122, 239,   4,
            0,   1,   1,   0,
            2, 255,   1,   2,
            1,   2, 255,   0

         };

         std::vector<unsigned short> data0_expected = {
          255, 122, 219,   4,
          255,   0,   0,   0,
            2,   0,   0,   2,
            1,   2,   0,   0
         };

         motdet::Image<unsigned char> img0_in0(data0_in0, 4), img0_in1(data0_in1, 4);
         motdet::Image<unsigned short> img0_expected(data0_expected, 4), img0_out(4, 4, 0);

         motdet::imgutil::image_subtraction<unsigned char, unsigned short>(img0_in0, img0_in1, img0_out);

         bool test_img0 = test_compare_vectors<unsigned short, unsigned short>(img0_out.get_data(),img0_expected.get_data());
         CHECK_TRUE(test_img0);

         // Check 1: Negatives

         std::vector<int> data1_in0 = {
           -3,   0,  23,  -0,
          225,   1,   1,   0,
            0, 215,   1,   0,
            0,   0, 155,   0

         };

         std::vector<int> data1_in1 = {
           -5, 122,-232,   4,
           -3,   1,   1,   0,
           -2, 255,   1,   2,
            1,   2, 255,   0

         };

         std::vector<unsigned long> data1_expected = {
            2,  122, 255,   4,
            228,  0,   0,   0,
            2,   40,   0,   2,
            1,    2, 100,   0
         };

         motdet::Image<int> img1_in0(data1_in0, 4), img1_in1(data1_in1, 4);
         motdet::Image<unsigned long> img1_expected(data1_expected, 4), img1_out(4, 4, 0);

         motdet::imgutil::image_subtraction<int, unsigned long>(img1_in0, img1_in1, img1_out);

         bool test_img1 = test_compare_vectors<unsigned long, unsigned long>(img1_out.get_data(),img1_expected.get_data());
         CHECK_TRUE(test_img1);

         return test_img0 && test_img1;
      }

      bool test_canny_edge_detection_8b()
      {
         // Note this test depends on Sobel edge detection, non-max supression and hysteresis

         //Check 0: Normal

         std::vector<unsigned char> data0_in = {
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0, 255,   0,
            0,   0,  40,  50,  50,  40,   0,   0,   0,   0,
            0,  40,  50,  60,  60,  50,  50,   0,   0,   0,
            0,  50,  60,  70,  70,  60,  50,   0,   0,   0,
            0,  50,  60,  70,  70,  60,  50,   0,   0,   0,
            0,  50,  60,  50,  60,  50,   0,   0,   0,   0,
            0,  40,  50,  50,  50,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0
         };

         std::vector<unsigned char> data0_expected = {
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   1,   1,   0,   0,   1,   0,   0,
            0,   0,   1,   0,   0,   1,   0,   1,   1,   0,
            0,   1,   0,   0,   0,   0,   1,   0,   0,   0,
            0,   1,   0,   0,   0,   0,   1,   0,   0,   0,
            0,   1,   0,   0,   0,   0,   1,   0,   0,   0,
            0,   1,   0,   0,   0,   1,   0,   0,   0,   0,
            0,   1,   0,   0,   1,   0,   0,   0,   0,   0,
            0,   0,   0,   1,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
         };

         motdet::Image<unsigned char> img0_in(data0_in, 10), img0_out(10, 10, 0), img0_expected(data0_expected, 10);

         motdet::imgutil::canny_edge_detection_8b<unsigned char>(img0_in, img0_out, 30, 120);

         bool test_img0 = test_compare_vectors<unsigned char, unsigned char>(img0_out.get_data(),img0_expected.get_data());
         CHECK_TRUE(test_img0);

         // Check 1: Drop

         std::vector<unsigned char> data1_in = {
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0, 255, 255,   0,   0,   0,   0,
            0,   0,   0,   0, 255, 255,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0
         };

         std::vector<unsigned char> data1_expected = {
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   1,   1,   1,   1,   0,   0,   0,
            0,   0,   0,   1,   1,   1,   1,   0,   0,   0,
            0,   0,   0,   1,   1,   1,   1,   0,   0,   0,
            0,   0,   0,   1,   1,   1,   1,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0
         };


         motdet::Image<unsigned char> img1_in(data1_in, 10), img1_out(10, 10, 0), img1_expected(data1_expected, 10);

         motdet::imgutil::canny_edge_detection_8b<unsigned char>(img1_in, img1_out, 30, 120);

         bool test_img1 = test_compare_vectors<unsigned char, unsigned char>(img1_out.get_data(),img1_expected.get_data());
         CHECK_TRUE(test_img1);

         // Check 2: Thresholding test

         std::vector<unsigned char> data2_in = {
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0, 100, 100,   0,   0,   0,   0,  30,  30,   0,
            0, 100, 100,   0,   0,   0,   0,  30,  30,   0,
            0,   0, 100,   0,   0,   0,   0,   0,   0,   0,
            0,   0, 100,   0,   0,   0,   0,   0,   0,   0,
            0,   0, 100,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,  30,  30,  30,   0,   0,   0,  10,  10,   0,
            0,  30,  30,  30,   0,   0,   0,  10,  10,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0
         };

         std::vector<int> data2_expected = {
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   1,   1,   1,   0,   0,   0,   0,   0,   0,
            0,   1,   1,   1,   0,   0,   0,   0,   0,   0,
            0,   1,   0,   1,   0,   0,   0,   0,   0,   0,
            0,   1,   0,   1,   0,   0,   0,   0,   0,   0,
            0,   1,   0,   1,   0,   0,   0,   0,   0,   0,
            0,   1,   0,   1,   0,   0,   0,   0,   0,   0,
            0,   1,   0,   1,   0,   0,   0,   0,   0,   0,
            0,   1,   0,   1,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0
         };


         motdet::Image<unsigned char> img2_in(data2_in, 10);
         motdet::Image<int> img2_expected(data2_expected, 10), img2_out(10, 10, 0);

         motdet::imgutil::canny_edge_detection_8b<int>(img2_in, img2_out, 100, 200);

         bool test_img2 = test_compare_vectors<int, int>(img2_out.get_data(),img2_expected.get_data());
         CHECK_TRUE(test_img2);

         return test_img0 && test_img1 && test_img2;
      }

      bool test_sobel_edge_detection_8b()
      {
         //Check 0: Normal

         std::vector<unsigned char> data0_in = {
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0, 255,   0,
            0,   0,  40,  50,  50,  40,   0,   0,   0,   0,
            0,  40,  50,  60,  60,  50,  50,   0,   0,   0,
            0,  50,  60,  70,  70,  60,  50,   0,   0,   0,
            0,  50,  60,  70,  70,  60,  50,   0,   0,   0,
            0,  50,  60,  50,  60,  50,   0,   0,   0,   0,
            0,  40,  50,  50,  50,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0
         };

         std::vector<unsigned char> data0_expected_mag = {
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

         std::vector<unsigned char> data0_expected_deg = {
              0,   0,   0,   0,   0,   0,   0,  79, 237, 177,
              0,  79, 167,  40, 216,  89, 177, 138,   0, 118,
             79,  79, 236,  71, 185,  43, 206, 197,   0,  59,
            247, 177,  79, 205,  51, 217,  79, 247,   0,   0,
            117,  86, 208,  79, 177,  25, 136, 118,   0,   0,
            138, 138,  11,  40, 216, 138, 195, 245,   0,   0,
            159, 190,  40, 200, 186,  59,  59,  59,   0,   0,
             29, 197, 220,   0, 171,  59,  59,   0,   0,   0,
            197, 109, 235,   0, 129,  59,   0,   0,   0,   0,
              0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
         };

         motdet::Image<unsigned char> img0_in(data0_in, 10), img0_out_mag(10, 10, 0), img0_out_deg(10, 10, 0), img0_expected_mag(data0_expected_mag, 10), img0_expected_deg(data0_expected_deg, 10);

         motdet::imgutil::sobel_edge_detection_8b<unsigned char>(img0_in, img0_out_mag, img0_out_deg);

         bool test_img0_mag = test_compare_vectors<unsigned char, unsigned char>(img0_out_mag.get_data(),img0_expected_mag.get_data());
         CHECK_TRUE(test_img0_mag);

         bool test_img0_deg = test_compare_vectors<unsigned char, unsigned char>(img0_out_deg.get_data(),img0_expected_deg.get_data());
         CHECK_TRUE(test_img0_deg);

         bool test_img0 = test_img0_mag && test_img0_deg;

         // Check 1: Drop

         std::vector<unsigned char> data1_in = {
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0, 255, 255,   0,   0,   0,   0,
            0,   0,   0,   0, 255, 255,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0
         };

         std::vector<unsigned char> data1_expected_mag = {
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0, 255, 255, 255, 255,   0,   0,   0,
            0,   0,   0, 255, 255, 255, 255,   0,   0,   0,
            0,   0,   0, 255, 255, 255, 255,   0,   0,   0,
            0,   0,   0, 255, 255, 255, 255,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0
         };

         std::vector<unsigned char> data1_expected_deg = {
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,  79, 148, 108, 177,   0,   0,   0,
            0,   0,   0,   9,  79, 177, 247,   0,   0,   0,
            0,   0,   0,  11, 197,  59, 245,   0,   0,   0,
            0,   0,   0, 197, 127, 129,  59,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
         };

         motdet::Image<unsigned char> img1_in(data1_in, 10), img1_out_mag(10, 10, 0), img1_out_deg(10, 10, 0), img1_expected_mag(data1_expected_mag, 10), img1_expected_deg(data1_expected_deg, 10);

         motdet::imgutil::sobel_edge_detection_8b<unsigned char>(img1_in, img1_out_mag, img1_out_deg);

         bool test_img1_mag = test_compare_vectors<unsigned char, unsigned char>(img1_out_mag.get_data(),img1_expected_mag.get_data());
         CHECK_TRUE(test_img1_mag);

         bool test_img1_deg = test_compare_vectors<unsigned char, unsigned char>(img1_out_deg.get_data(),img1_expected_deg.get_data());
         CHECK_TRUE(test_img1_deg);

         bool test_img1 = test_img1_mag && test_img1_deg;

         return test_img0 && test_img1;
      }

      bool test_non_max_suppression()
      {
         std::vector<unsigned char> data0_in_mag = {
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

         std::vector<unsigned char> data0_in_deg = {
              0,   0,   0,   0,   0,   0,   0,  79, 237, 177,
              0,  79, 167,  40, 216,  89, 177, 138,   0, 118,
             79,  79, 236,  71, 185,  43, 206, 197,   0,  59,
            247, 177,  79, 205,  51, 217,  79, 247,   0,   0,
            117,  86, 208,  79, 177,  25, 136, 118,   0,   0,
            138, 138,  11,  40, 216, 138, 195, 245,   0,   0,
            159, 190,  40, 200, 186,  59,  59,  59,   0,   0,
             29, 197, 220,   0, 171,  59,  59,   0,   0,   0,
            197, 109, 235,   0, 129,  59,   0,   0,   0,   0,
              0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
         };

         std::vector<unsigned char> data0_expected = {
            0,   0,   0,   0,   0,   0,   0, 255, 255,   0,
            0,   0,   0, 190, 190,   0,   0, 255,   0,   0,
            0,   0, 233,   0,   0, 237,   0, 255, 255,   0,
            0, 233,   0,   0,   0,   0, 233,   0,   0,   0,
            0, 231,   0,   0,   0,   0, 230,   0,   0,   0,
            0, 240,   0,   0,   0,   0, 254,   0,   0,   0,
            0, 231,   0,   0,   0, 255,   0,   0,   0,   0,
            0, 226,   0,   0, 241,   0,   0,   0,   0,   0,
            0,   0,   0, 200,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
         };

         motdet::Image<unsigned char> img0_in_mag(data0_in_mag, 10), img0_in_deg(data0_in_deg, 10), img0_out(10, 10, 0), img0_expected(data0_expected, 10);

         motdet::imgutil::non_max_suppression<unsigned char, unsigned char>(img0_in_mag, img0_in_deg, img0_out);

         bool test_img0 = test_compare_vectors<unsigned char, unsigned char>(img0_out.get_data(),img0_expected.get_data());
         CHECK_TRUE(test_img0);

         return test_img0;
      }

      bool test_double_threshold()
      {
         std::vector<unsigned char> data0_in = {
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

         motdet::Image<unsigned char> img0_in(data0_in, 10), img0_out(10, 10, 0), img0_expected(data0_expected, 10);

         motdet::imgutil::double_threshold<unsigned char, unsigned char>(img0_in, img0_out, 150, 250);

         bool test_img0 = test_compare_vectors<unsigned char, unsigned char>(img0_out.get_data(),img0_expected.get_data());
         CHECK_TRUE(test_img0);

         return test_img0;
      }

      bool test_single_threshold()
      {
         std::vector<unsigned char> data0_in = {
              0,   0,   0,   0,   0,   0,   0, 255, 255, 255,
              0,  56, 139, 190, 190, 139,  56, 255,   0, 255,
             56, 183, 233, 231, 231, 237, 198, 255, 255, 255,
            139, 233, 155,  89,  89, 142, 233, 158,   0,   0,
            190, 231,  89,  56,  56,  76, 230, 200,   0,   0,
            200, 240,  63,  58,  58, 144, 254, 158,   0,   0,
            190, 231,  58,  70, 134, 255, 226,  70,   0,   0,
            139, 226, 220, 220, 241, 226,  70,   0,   0,   0,
            56,  139, 190, 200, 158,  70,   0,   0,   0,   0,
            0,     0,   0,   0,   0,   0,   0,   0,   0,   0
         };

         std::vector<unsigned char> data0_expected = {
            0,   0,   0,   0,   0,   0,   0,   1,   1,   1,
            0,   0,   0,   1,   1,   0,   0,   1,   0,   1,
            0,   1,   1,   1,   1,   1,   1,   1,   1,   1,
            0,   1,   1,   0,   0,   0,   1,   1,   0,   0,
            1,   1,   0,   0,   0,   0,   1,   1,   0,   0,
            1,   1,   0,   0,   0,   0,   1,   1,   0,   0,
            1,   1,   0,   0,   0,   1,   1,   0,   0,   0,
            0,   1,   1,   1,   1,   1,   0,   0,   0,   0,
            0,   0,   1,   1,   1,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
         };

         motdet::Image<unsigned char> img0_in(data0_in, 10), img0_out(10, 10, 0), img0_expected(data0_expected, 10);

         motdet::imgutil::single_threshold<unsigned char, unsigned char>(img0_in, img0_out, 150);

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

         motdet::imgutil::hysteresis<unsigned char, unsigned char>(img0_in, img0_out);

         bool test_img0 = test_compare_vectors<unsigned char, unsigned char>(img0_out.get_data(),img0_expected.get_data());
         CHECK_TRUE(test_img0);

         return test_img0;
      }

      bool test_image_interpolation()
      {
         std::vector<unsigned char> data0_in_from = {
            0,   0,   0,   0,   0,   0,   0,
            0,  56, 139, 190, 190, 139,  56,
            56, 183, 233, 231, 231, 237, 198,
            139, 233, 155,  89,  89, 142, 233
         };

         std::vector<unsigned char> data0_in_to = {
            0,   0,   0,  15,   0,   0,   0,
            0,  79, 167,  40, 216,  89, 177,
            79,  79, 236,  71, 185,  43, 206,
            247, 177,  79, 205,  51, 217,  79
         };

         std::vector<unsigned char> data0_expected_07 = {
            0,   0,   0,  10,   0,   0,   0,
            0,  72, 158,  85, 208, 104, 140,
            72, 110, 235, 119, 198, 101, 203,
            214, 193, 101, 170,  62, 194, 125
         };

         std::vector<unsigned char> data0_expected_1 = {
            0,   0,   0,  15,   0,   0,   0,
            0,  79, 167,  40, 216,  89, 177,
            79,  79, 236,  71, 185,  43, 206,
            247, 177,  79, 205,  51, 217,  79
         };

         std::vector<unsigned char> data0_expected_0 = {
            0,   0,   0,   0,   0,   0,   0,
            0,  56, 139, 190, 190, 139,  56,
            56, 183, 233, 231, 231, 237, 198,
            139, 233, 155,  89,  89, 142, 233
         };

         motdet::Image<unsigned char> img0_in_from(data0_in_from, 7), img0_in_to(data0_in_to, 7), img0_out_07(7, 4, 0), img0_out_0(7, 4, 0), img0_out_1(7, 4, 0), img0_expected_07(data0_expected_07, 7), img0_expected_0(data0_expected_0, 7), img0_expected_1(data0_expected_1, 7);

         motdet::imgutil::image_interpolation<unsigned char, unsigned char>(img0_in_from, img0_in_to, img0_out_07, 0.7);
         motdet::imgutil::image_interpolation<unsigned char, unsigned char>(img0_in_from, img0_in_to, img0_out_0, 0);
         motdet::imgutil::image_interpolation<unsigned char, unsigned char>(img0_in_from, img0_in_to, img0_out_1, 1);

         bool test_img0_07 = test_compare_vectors<unsigned char, unsigned char>(img0_out_07.get_data(),img0_expected_07.get_data());
         CHECK_TRUE(test_img0_07);
         bool test_img0_0 = test_compare_vectors<unsigned char, unsigned char>(img0_out_0.get_data(),img0_expected_0.get_data());
         CHECK_TRUE(test_img0_0);
         bool test_img0_1 = test_compare_vectors<unsigned char, unsigned char>(img0_out_1.get_data(),img0_expected_1.get_data());
         CHECK_TRUE(test_img0_1);
         bool test_img0 = test_img0_07 && test_img0_0 && test_img0_1;

         return test_img0;
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

         motdet::imgutil::dilation<unsigned char, unsigned char>(img0_in, img0_out);

         bool test_img0 = test_compare_vectors<unsigned char, unsigned char>(img0_out.get_data(),img0_expected.get_data());
         CHECK_TRUE(test_img0);

         return test_img0;
      }

      bool test_erosion()
      {
         std::vector<unsigned char> data0_in = {
            0,   0,   0,   0,   0,   0,   0,   1,   1,   1,
            0,   0,   0,   1,   1,   0,   0,   1,   1,   1,
            0,   1,   1,   1,   1,   1,   1,   1,   1,   1,
            0,   1,   1,   0,   0,   0,   1,   1,   1,   0,
            1,   1,   0,   0,   0,   0,   1,   1,   1,   0,
            1,   1,   0,   0,   0,   0,   1,   1,   1,   0,
            1,   1,   0,   0,   0,   1,   1,   0,   0,   0,
            0,   1,   1,   1,   1,   1,   0,   0,   0,   1,
            0,   0,   0,   0,   0,   0,   0,   0,   1,   1,
            0,   0,   0,   0,   0,   0,   0,   0,   1,   1,
         };

         std::vector<unsigned char> data0_expected = {
            0,   0,   0,   0,   0,   0,   0,   0,   1,   1,
            0,   0,   0,   0,   0,   0,   0,   0,   1,   1,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   1,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   1,   0,   0,
            1,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,   1,
         };

         motdet::Image<unsigned char> img0_in(data0_in, 10), img0_out(10, 10, 0), img0_expected(data0_expected, 10);

         motdet::imgutil::erosion<unsigned char, unsigned char>(img0_in, img0_out);

         bool test_img0 = test_compare_vectors<unsigned char, unsigned char>(img0_out.get_data(),img0_expected.get_data());
         CHECK_TRUE(test_img0);

         return test_img0;
      }

      bool test_reescale_pix_length()
      {
         // Check 0: Binary to arbitrary grayscale

         std::vector<unsigned char> data0_in = {
            0,   0,   0,   0,   0,   0,   0,   1,   1,   1,
            0,   0,   0,   1,   1,   0,   0,   1,   1,   1,
            0,   1,   1,   1,   1,   1,   1,   1,   1,   1,
            0,   1,   1,   0,   0,   0,   1,   1,   1,   0,
            1,   1,   0,   0,   0,   0,   1,   1,   1,   0,
            1,   1,   0,   0,   0,   0,   1,   1,   1,   0,
            1,   1,   0,   0,   0,   1,   1,   0,   0,   0,
            0,   1,   1,   1,   1,   1,   0,   0,   0,   1,
            0,   0,   0,   0,   0,   0,   0,   0,   1,   1,
            0,   0,   0,   0,   0,   0,   0,   0,   1,   1,
         };

         std::vector<unsigned char> data0_expected = {
            0,   0,   0,   0,   0,   0,   0, 255, 255, 255,
            0,   0,   0, 255, 255,   0,   0, 255, 255, 255,
            0, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            0, 255, 255,   0,   0,   0, 255, 255, 255,   0,
            255, 255,   0,   0,   0,   0, 255, 255, 255,   0,
            255, 255,   0,   0,   0,   0, 255, 255, 255,   0,
            255, 255,   0,   0,   0, 255, 255,   0,   0,   0,
            0, 255, 255, 255, 255, 255,   0,   0,   0, 255,
            0,   0,   0,   0,   0,   0,   0,   0, 255, 255,
            0,   0,   0,   0,   0,   0,   0,   0, 255, 255,
         };

         motdet::Image<unsigned char> img0_in(data0_in, 10), img0_out(10, 10, 0), img0_expected(data0_expected, 10);

         motdet::imgutil::reescale_pix_length<unsigned char, unsigned char>(img0_in, img0_out, 1, 255);

         bool test_img0 = test_compare_vectors<unsigned char, unsigned char>(img0_out.get_data(),img0_expected.get_data());
         CHECK_TRUE(test_img0);

         // Check 1: Reescale 8b to 16b and back

         std::vector<unsigned char> data1_in = {
             0,    0,   0,  15,   0,   0,   0,
             0,   79, 167,  40, 216,  89, 177,
             79,  79, 236,  71, 185,  43, 206,
            247, 177,  79, 205,  51, 217,  79
         };

         std::vector<unsigned short> data1_expected = {
              0,     0,     0,  3855,     0,     0,     0,
              0, 20303, 42919, 10280, 55512, 22873, 45489,
          20303, 20303, 60652, 18247, 47545, 11051, 52942,
          63479, 45489, 20303, 52685, 13107, 55769, 20303
         };

         motdet::Image<unsigned char> img1_in(data1_in, 7), img1_out_from(7, 4, 0);
         motdet::Image<unsigned short> img1_out_to(7, 4, 0), img1_expected(data1_expected, 7);

         motdet::imgutil::reescale_pix_length<unsigned char, unsigned short>(img1_in, img1_out_to, 255, 65535);

         bool test_img1_to = test_compare_vectors<unsigned short, unsigned short>(img1_out_to.get_data(), img1_expected.get_data());
         CHECK_TRUE(test_img1_to);

         motdet::imgutil::reescale_pix_length<unsigned short, unsigned char>(img1_out_to, img1_out_from, 65535, 255);

         bool test_img1_from = test_compare_vectors<unsigned char, unsigned char>(img1_out_from.get_data(), img1_in.get_data());
         CHECK_TRUE(test_img1_from);
         bool test_img1 = test_img1_to && test_img1_from;

         return test_img0 && test_img1;
      }

      bool test_downsample()
      {
         std::vector<unsigned char> data0_in = {
            0,   0,   0,   0,   0,   0,   0,   6,
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,  40,  50,  50,  40,   0,   0,
            0,  40,  50,  60,  60,  50,  50,   0,
            0,  50,  60,  70,  70,  60,  50,   0,
            0,  50,  60,  70,  70,  60,  50,   0,
            0,  50,  70,  50,  60,  50,   0,   0,
         };

         std::vector<unsigned char> data0_expected = {
            4,  15,   1,
           34,  63,  25,
           40,  53,   0
         };

         motdet::Image<unsigned char> img0_in(data0_in, 8), img0_out(3, 3, 0), img0_expected(data0_expected, 3);

         motdet::imgutil::downsample<unsigned char, unsigned char>(img0_in, img0_out, 3);

         bool test_img0 = test_compare_vectors<unsigned char, unsigned char>(img0_out.get_data(),img0_expected.get_data());
         CHECK_TRUE(test_img0);

         return test_img0;
      }

      bool test_upsample()
      {
         std::vector<unsigned char> data0_in = {
            3,   0,  255,
            0,   0,   0,
            7,  10,  40
         };

         std::vector<unsigned char> data0_expected = {
            3,   3,   3,   0,   0,   0, 255, 255, 255,
            3,   3,   3,   0,   0,   0, 255, 255, 255,
            3,   3,   3,   0,   0,   0, 255, 255, 255,
            0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,   0,   0,   0,   0,   0,   0,   0,
            7,   7,   7,  10,  10,  10,  40,  40,  40,
            7,   7,   7,  10,  10,  10,  40,  40,  40,
            7,   7,   7,  10,  10,  10,  40,  40,  40
         };

         motdet::Image<unsigned char> img0_in(data0_in, 3), img0_out(9, 9, 0), img0_expected(data0_expected, 9);

         motdet::imgutil::upsample<unsigned char, unsigned char>(img0_in, img0_out, 3);

         bool test_img0 = test_compare_vectors<unsigned char, unsigned char>(img0_out.get_data(),img0_expected.get_data());
         CHECK_TRUE(test_img0);

         return test_img0;
      }

   } // namespace test
} // namespace image_utils
