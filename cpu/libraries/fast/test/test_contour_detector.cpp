    #include "test_contour_detector.hpp"
#include "test_utils.hpp"

#include <iostream>

namespace test
{
    namespace contour_detector
    {
        void test_all()
        {
            std::cout << "Testing module contour_detector..." << std::endl;

            log_test_result(test_contour_detection(), "contour_detection");

            std::cout << "Finished tests for module contour_detector." << std::endl << std::endl;
        }

        bool test_contour_detection()
        {
            // Check 1: Normal

            std::vector<unsigned char> data0_in = {
               0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
               0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   0,
               0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   0,
               0,   0,   0,   0,   0,   1,   1,   1,   1,   1,   1,   0,   1,   1,   0,
               0,   0,   0,   1,   1,   1,   1,   1,   1,   1,   1,   0,   1,   1,   0,
               0,   1,   1,   1,   1,   1,   0,   0,   0,   1,   1,   0,   1,   1,   0,
               0,   1,   1,   1,   1,   0,   0,   0,   0,   1,   1,   0,   1,   1,   0,
               0,   1,   1,   1,   0,   0,   1,   0,   0,   1,   1,   0,   1,   1,   0,
               0,   1,   1,   1,   0,   1,   1,   0,   0,   1,   1,   0,   1,   1,   0,
               0,   1,   1,   1,   0,   1,   1,   0,   1,   1,   1,   0,   1,   1,   0,
               0,   1,   1,   1,   0,   1,   1,   0,   1,   1,   1,   0,   0,   1,   0,
               0,   1,   1,   1,   0,   0,   0,   0,   1,   1,   1,   0,   0,   1,   0,
               0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   1,   0,
               0,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,   0,
               0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
            };

            std::vector<unsigned char> data0_expected = {
               0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
               0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   2,   3,   0,
               0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   2,   3,   0,
               0,   0,   0,   0,   0,   2,   2,   2,   2,   2,   3,   0,   2,   3,   0,
               0,   0,   0,   2,   2,   1,   2,   2,   2,   1,   3,   0,   2,   3,   0,
               0,   2,   2,   1,   1,   3,   0,   0,   0,   2,   3,   0,   2,   3,   0,
               0,   2,   1,   1,   3,   0,   0,   0,   0,   2,   3,   0,   2,   3,   0,
               0,   2,   1,   3,   0,   0,   3,   0,   0,   2,   3,   0,   2,   3,   0,
               0,   2,   1,   3,   0,   2,   3,   0,   0,   2,   3,   0,   2,   3,   0,
               0,   2,   1,   3,   0,   2,   3,   0,   2,   1,   3,   0,   2,   3,   0,
               0,   2,   1,   3,   0,   2,   3,   0,   2,   1,   3,   0,   0,   3,   0,
               0,   2,   1,   3,   0,   0,   0,   0,   2,   1,   3,   0,   0,   3,   0,
               0,   2,   1,   1,   2,   2,   2,   2,   1,   1,   3,   0,   0,   3,   0,
               0,   0,   2,   2,   2,   2,   2,   2,   2,   2,   3,   0,   0,   0,   0,
               0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            };

            motdet::Image<unsigned char> img0(data0_in, 15), img0_expected(data0_expected, 15);

            std::vector<motdet::Contour> img0_contours = motdet::imgutil::contour_detection(img0, false); // No edge trimming

            /*
            for(const motdet::Contour &cont : img0_contours)
            {
                std::cout << "bb_tl_x " << cont.bb_tl_x << std::endl;
                std::cout << "bb_tl_y " << cont.bb_tl_y << std::endl;
                std::cout << "bb_br_x " << cont.bb_br_x << std::endl;
                std::cout << "bb_br_y " << cont.bb_br_y << std::endl << std::endl;
            }
            */

            bool test_img0_map = test_compare_vectors<unsigned char, unsigned char>(img0.get_data(),img0_expected.get_data());
            CHECK_TRUE(test_img0_map);

            motdet::Contour cont = img0_contours[0];
            bool test_img0_c2_bbox = cont.bb_tl_x == 11 && cont.bb_tl_y == 1 && cont.bb_br_x == 13 && cont.bb_br_y == 12;
            CHECK_TRUE(test_img0_c2_bbox);

            cont = img0_contours[1];
            bool test_img0_c3_bbox = cont.bb_tl_x == 1 && cont.bb_tl_y == 3 && cont.bb_br_x == 10 && cont.bb_br_y == 13;
            CHECK_TRUE(test_img0_c3_bbox);

            cont = img0_contours[2];
            bool test_img0_c4_bbox = cont.bb_tl_x == 3 && cont.bb_tl_y == 4 && cont.bb_br_x == 9 && cont.bb_br_y == 12;
            CHECK_TRUE(test_img0_c4_bbox);

            cont = img0_contours[3];
            bool test_img0_c5_bbox = cont.bb_tl_x == 5 && cont.bb_tl_y == 7 && cont.bb_br_x == 6 && cont.bb_br_y == 10;
            CHECK_TRUE(test_img0_c5_bbox);

            bool test_img0_conts = test_img0_c2_bbox && test_img0_c3_bbox && test_img0_c4_bbox && test_img0_c5_bbox;

            bool test_img0 = test_img0_map && test_img0_conts;



            // Check 2, hierarchy test

            std::vector<unsigned char> data1_in = {
               0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   0,   0,   0,   0,   0,   0,   0,   0,
               0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,
               0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,
               0,   1,   1,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   0,
               0,   1,   1,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   1,   1,   0,
               0,   1,   1,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   1,   1,   0,
               1,   1,   1,   0,   1,   1,   0,   0,   0,   0,   0,   0,   0,   1,   1,   0,   1,   1,   0,
               1,   1,   1,   0,   1,   1,   0,   1,   1,   1,   1,   0,   0,   1,   1,   0,   1,   1,   0,
               1,   1,   1,   0,   1,   1,   0,   1,   1,   1,   0,   0,   0,   1,   1,   0,   1,   1,   0,
               1,   1,   1,   0,   1,   1,   0,   1,   1,   1,   0,   1,   0,   1,   1,   0,   1,   1,   0,
               1,   1,   1,   0,   1,   1,   0,   1,   1,   1,   0,   0,   0,   1,   1,   0,   1,   1,   0,
               1,   1,   1,   0,   1,   1,   0,   1,   1,   1,   1,   0,   0,   1,   1,   0,   1,   1,   0,
               0,   1,   1,   0,   1,   1,   0,   0,   0,   0,   0,   0,   0,   1,   1,   0,   1,   1,   1,
               0,   1,   1,   0,   1,   1,   0,   1,   1,   1,   1,   1,   0,   1,   1,   0,   1,   1,   1,
               0,   1,   1,   0,   1,   1,   0,   1,   0,   1,   0,   1,   0,   1,   1,   0,   1,   1,   1,
               0,   1,   1,   0,   1,   1,   0,   1,   0,   1,   0,   1,   0,   1,   1,   0,   1,   1,   0,
               0,   1,   1,   0,   1,   1,   0,   1,   0,   1,   0,   1,   0,   1,   1,   0,   1,   1,   0,
               0,   1,   1,   0,   1,   1,   0,   1,   0,   1,   0,   1,   0,   1,   1,   0,   1,   1,   0,
               0,   1,   1,   0,   1,   1,   0,   1,   0,   1,   0,   1,   0,   1,   1,   0,   1,   1,   0,
               0,   1,   1,   0,   1,   1,   0,   1,   1,   1,   1,   1,   0,   1,   1,   0,   1,   1,   0,
               0,   1,   1,   0,   1,   1,   0,   0,   0,   0,   0,   0,   0,   1,   1,   0,   1,   1,   0,
               0,   1,   1,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   1,   1,   0,
               0,   1,   1,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   1,   1,   0,
               0,   1,   1,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   0,
               0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,
               0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
               0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   1
            };

            std::vector<unsigned char> data1_expected = {
               0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
               0,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   3,   0,
               0,   2,   1,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   1,   3,   0,
               0,   2,   3,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   2,   3,   0,
               0,   2,   3,   0,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   3,   0,   2,   3,   0,
               0,   2,   3,   0,   2,   1,   2,   2,   2,   2,   2,   2,   2,   1,   3,   0,   2,   3,   0,
               0,   2,   3,   0,   2,   3,   0,   0,   0,   0,   0,   0,   0,   2,   3,   0,   2,   3,   0,
               0,   2,   3,   0,   2,   3,   0,   2,   2,   2,   3,   0,   0,   2,   3,   0,   2,   3,   0,
               0,   2,   3,   0,   2,   3,   0,   2,   1,   3,   0,   0,   0,   2,   3,   0,   2,   3,   0,
               0,   2,   3,   0,   2,   3,   0,   2,   1,   3,   0,   3,   0,   2,   3,   0,   2,   3,   0,
               0,   2,   3,   0,   2,   3,   0,   2,   1,   3,   0,   0,   0,   2,   3,   0,   2,   3,   0,
               0,   2,   3,   0,   2,   3,   0,   2,   2,   2,   3,   0,   0,   2,   3,   0,   2,   3,   0,
               0,   2,   3,   0,   2,   3,   0,   0,   0,   0,   0,   0,   0,   2,   3,   0,   2,   3,   0,
               0,   2,   3,   0,   2,   3,   0,   2,   2,   2,   2,   3,   0,   2,   3,   0,   2,   3,   0,
               0,   2,   3,   0,   2,   3,   0,   3,   0,   3,   0,   3,   0,   2,   3,   0,   2,   3,   0,
               0,   2,   3,   0,   2,   3,   0,   3,   0,   3,   0,   3,   0,   2,   3,   0,   2,   3,   0,
               0,   2,   3,   0,   2,   3,   0,   3,   0,   3,   0,   3,   0,   2,   3,   0,   2,   3,   0,
               0,   2,   3,   0,   2,   3,   0,   3,   0,   3,   0,   3,   0,   2,   3,   0,   2,   3,   0,
               0,   2,   3,   0,   2,   3,   0,   3,   0,   3,   0,   3,   0,   2,   3,   0,   2,   3,   0,
               0,   2,   3,   0,   2,   3,   0,   2,   2,   2,   2,   3,   0,   2,   3,   0,   2,   3,   0,
               0,   2,   3,   0,   2,   3,   0,   0,   0,   0,   0,   0,   0,   2,   3,   0,   2,   3,   0,
               0,   2,   3,   0,   2,   1,   2,   2,   2,   2,   2,   2,   2,   1,   3,   0,   2,   3,   0,
               0,   2,   3,   0,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   3,   0,   2,   3,   0,
               0,   2,   3,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   2,   3,   0,
               0,   2,   1,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   1,   3,   0,
               0,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   3,   0,
               0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            };

            motdet::Image<unsigned char> img1(data1_in, 19), img1_expected(data1_expected, 19);

            std::vector<motdet::Contour> img1_contours = motdet::imgutil::contour_detection(img1); // Trimming edges

            bool test_img1_map = test_compare_vectors<unsigned char, unsigned char>(img1.get_data(),img1_expected.get_data());
            CHECK_TRUE(test_img1_map);

            cont = img1_contours[0];
            bool test_img1_c2_bbox = cont.bb_tl_x == 0 && cont.bb_tl_y == 1 && cont.bb_br_x == 17 && cont.bb_br_y == 25;
            CHECK_TRUE(test_img1_c2_bbox);

            cont = img1_contours[1];
            bool test_img1_c3_bbox = cont.bb_tl_x == 2 && cont.bb_tl_y == 2 && cont.bb_br_x == 16 && cont.bb_br_y == 24;
            CHECK_TRUE(test_img1_c3_bbox);

            cont = img1_contours[2];
            bool test_img1_c4_bbox = cont.bb_tl_x == 3 && cont.bb_tl_y == 4 && cont.bb_br_x == 14 && cont.bb_br_y == 22;
            CHECK_TRUE(test_img1_c4_bbox);

            cont = img1_contours[3];
            bool test_img1_c5_bbox = cont.bb_tl_x == 5 && cont.bb_tl_y == 5 && cont.bb_br_x == 13 && cont.bb_br_y == 21;
            CHECK_TRUE(test_img1_c5_bbox);

            cont = img1_contours[4];
            bool test_img1_c6_bbox = cont.bb_tl_x == 6 && cont.bb_tl_y == 7 && cont.bb_br_x == 10 && cont.bb_br_y == 11;
            CHECK_TRUE(test_img1_c6_bbox);

            cont = img1_contours[5];
            bool test_img1_c7_bbox = cont.bb_tl_x == 10 && cont.bb_tl_y == 9 && cont.bb_br_x == 10 && cont.bb_br_y == 9;
            CHECK_TRUE(test_img1_c7_bbox);

            cont = img1_contours[6];
            bool test_img1_c8_bbox = cont.bb_tl_x == 6 && cont.bb_tl_y == 13 && cont.bb_br_x == 11 && cont.bb_br_y == 19;
            CHECK_TRUE(test_img1_c8_bbox);

            cont = img1_contours[7];
            bool test_img1_c9_bbox = cont.bb_tl_x == 7 && cont.bb_tl_y == 13 && cont.bb_br_x == 9 && cont.bb_br_y == 19;
            CHECK_TRUE(test_img1_c9_bbox);

            bool test_img1_conts = test_img1_c2_bbox && test_img1_c3_bbox && test_img1_c4_bbox && test_img1_c5_bbox && test_img1_c6_bbox && test_img1_c7_bbox && test_img1_c8_bbox && test_img1_c9_bbox;

            bool test_img1 = test_img1_map && test_img1_conts;

            return test_img0 && test_img1;
        }
    } // namespace contour_detector
} // namespace test
