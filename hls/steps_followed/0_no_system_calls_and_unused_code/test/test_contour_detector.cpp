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

            std::vector<int> data0_in = {
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

            std::vector<int> data0_expected = {
               0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
               0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   2,  -2,   0,
               0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   2,  -2,   0,
               0,   0,   0,   0,   0,   3,   3,   3,   3,   3,  -3,   0,   2,  -2,   0,
               0,   0,   0,   3,   3,   1,   4,   4,   4,   1,  -3,   0,   2,  -2,   0,
               0,   3,   3,   1,   1,  -4,   0,   0,   0,   4,  -3,   0,   2,  -2,   0,
               0,   3,   1,   1,  -4,   0,   0,   0,   0,   4,  -3,   0,   2,  -2,   0,
               0,   3,   1,  -4,   0,   0,  -5,   0,   0,   4,  -3,   0,   2,  -2,   0,
               0,   3,   1,  -4,   0,   5,  -5,   0,   0,   4,  -3,   0,   2,  -2,   0,
               0,   3,   1,  -4,   0,   5,  -5,   0,   4,   1,  -3,   0,   2,  -2,   0,
               0,   3,   1,  -4,   0,   5,  -5,   0,   4,   1,  -3,   0,   0,  -2,   0,
               0,   3,   1,  -4,   0,   0,   0,   0,   4,   1,  -3,   0,   0,  -2,   0,
               0,   3,   1,   1,   4,   4,   4,   4,   1,   1,  -3,   0,   0,  -2,   0,
               0,   0,   3,   3,   3,   3,   3,   3,   3,   3,  -3,   0,   0,   0,   0,
               0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            };

            motdet::Image<int> img0(data0_in, 15), img0_expected(data0_expected, 15);
            std::vector<motdet::Motion_detector::Contour> img0_contours;

            motdet::imgutil::contour_detection(img0, img0_contours, false); // No edge trimming

            bool test_img0_map = test_compare_vectors<int, int>(img0.get_data(),img0_expected.get_data());
            CHECK_TRUE(test_img0_map);

            motdet::Motion_detector::Contour cont = img0_contours[0];
            bool test_img0_c2_id = cont.id == 2;
            bool test_img0_c2_parent = cont.parent == 0;
            bool test_img0_c2_hole = cont.is_hole == false;
            bool test_img0_c2_pixels = cont.n_pixels == 24;
            bool test_img0_c2_bbox = cont.bb_tl_x == 11 && cont.bb_tl_y == 1 && cont.bb_br_x == 13 && cont.bb_br_y == 12;
            bool test_img0_c2 = test_img0_c2_id && test_img0_c2_parent && test_img0_c2_hole && test_img0_c2_pixels && test_img0_c2_bbox;
            CHECK_TRUE(test_img0_c2);

            cont = img0_contours[1];
            bool test_img0_c3_id = cont.id == 3;
            bool test_img0_c3_parent = cont.parent == 0;
            bool test_img0_c3_hole = cont.is_hole == false;
            bool test_img0_c3_pixels = cont.n_pixels == 36;
            bool test_img0_c3_bbox = cont.bb_tl_x == 1 && cont.bb_tl_y == 3 && cont.bb_br_x == 10 && cont.bb_br_y == 13;
            bool test_img0_c3 = test_img0_c3_id && test_img0_c3_parent && test_img0_c3_hole && test_img0_c3_pixels && test_img0_c3_bbox;
            CHECK_TRUE(test_img0_c3);

            cont = img0_contours[2];
            bool test_img0_c4_id = cont.id == 4;
            bool test_img0_c4_parent = cont.parent == 3;
            bool test_img0_c4_hole = cont.is_hole == true;
            bool test_img0_c4_pixels = cont.n_pixels == 22;
            bool test_img0_c4_bbox = cont.bb_tl_x == 3 && cont.bb_tl_y == 4 && cont.bb_br_x == 9 && cont.bb_br_y == 12;
            bool test_img0_c4 = test_img0_c4_id && test_img0_c4_parent && test_img0_c4_hole && test_img0_c4_pixels && test_img0_c4_bbox;
            CHECK_TRUE(test_img0_c4);

            cont = img0_contours[3];
            bool test_img0_c5_id = cont.id == 5;
            bool test_img0_c5_parent = cont.parent == 4;
            bool test_img0_c5_hole = cont.is_hole == false;
            bool test_img0_c5_pixels = cont.n_pixels == 8;
            bool test_img0_c5_bbox = cont.bb_tl_x == 5 && cont.bb_tl_y == 7 && cont.bb_br_x == 6 && cont.bb_br_y == 10;
            bool test_img0_c5 = test_img0_c5_id && test_img0_c5_parent && test_img0_c5_hole && test_img0_c5_pixels && test_img0_c5_bbox;
            CHECK_TRUE(test_img0_c5);

            bool test_img0_conts = test_img0_c2 && test_img0_c3 && test_img0_c4 && test_img0_c5;

            bool test_img0 = test_img0_map && test_img0_conts;

            /* Useful for debugging
            for(const motdet::Motion_detector::Contour &cont : img0_contours)
            {
                std::cout << "id " << cont.id << std::endl;
                std::cout << "parent " << cont.parent << std::endl;
                std::cout << "is_hole " << cont.is_hole << std::endl;
                std::cout << "n_pixels " << cont.n_pixels << std::endl;
                std::cout << "bb_tl_x " << cont.bb_tl_x << std::endl;
                std::cout << "bb_tl_y " << cont.bb_tl_y << std::endl;
                std::cout << "bb_br_x " << cont.bb_br_x << std::endl;
                std::cout << "bb_br_y " << cont.bb_br_y << std::endl << std::endl;
            }
            */

            // Check 2, hierarchy test

            std::vector<int> data1_in = {
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

            std::vector<int> data1_expected = {
               0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
               0,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,  -2,   0,
               0,   2,   1,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   1,  -2,   0,
               0,   2,  -3,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   3,  -2,   0,
               0,   2,  -3,   0,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,  -4,   0,   3,  -2,   0,
               0,   2,  -3,   0,   4,   1,   5,   5,   5,   5,   5,   5,   5,   1,  -4,   0,   3,  -2,   0,
               0,   2,  -3,   0,   4,  -5,   0,   0,   0,   0,   0,   0,   0,   5,  -4,   0,   3,  -2,   0,
               0,   2,  -3,   0,   4,  -5,   0,   6,   6,   6,  -6,   0,   0,   5,  -4,   0,   3,  -2,   0,
               0,   2,  -3,   0,   4,  -5,   0,   6,   1,  -6,   0,   0,   0,   5,  -4,   0,   3,  -2,   0,
               0,   2,  -3,   0,   4,  -5,   0,   6,   1,  -6,   0,  -7,   0,   5,  -4,   0,   3,  -2,   0,
               0,   2,  -3,   0,   4,  -5,   0,   6,   1,  -6,   0,   0,   0,   5,  -4,   0,   3,  -2,   0,
               0,   2,  -3,   0,   4,  -5,   0,   6,   6,   6,  -6,   0,   0,   5,  -4,   0,   3,  -2,   0,
               0,   2,  -3,   0,   4,  -5,   0,   0,   0,   0,   0,   0,   0,   5,  -4,   0,   3,  -2,   0,
               0,   2,  -3,   0,   4,  -5,   0,   8,   8,   8,   8,  -8,   0,   5,  -4,   0,   3,  -2,   0,
               0,   2,  -3,   0,   4,  -5,   0,  -9,   0,  -9,   0,  -8,   0,   5,  -4,   0,   3,  -2,   0,
               0,   2,  -3,   0,   4,  -5,   0,  -9,   0,  -9,   0,  -8,   0,   5,  -4,   0,   3,  -2,   0,
               0,   2,  -3,   0,   4,  -5,   0,  -9,   0,  -9,   0,  -8,   0,   5,  -4,   0,   3,  -2,   0,
               0,   2,  -3,   0,   4,  -5,   0,  -9,   0,  -9,   0,  -8,   0,   5,  -4,   0,   3,  -2,   0,
               0,   2,  -3,   0,   4,  -5,   0,  -9,   0,  -9,   0,  -8,   0,   5,  -4,   0,   3,  -2,   0,
               0,   2,  -3,   0,   4,  -5,   0,   8,   8,   8,   8,  -8,   0,   5,  -4,   0,   3,  -2,   0,
               0,   2,  -3,   0,   4,  -5,   0,   0,   0,   0,   0,   0,   0,   5,  -4,   0,   3,  -2,   0,
               0,   2,  -3,   0,   4,   1,   5,   5,   5,   5,   5,   5,   5,   1,  -4,   0,   3,  -2,   0,
               0,   2,  -3,   0,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,  -4,   0,   3,  -2,   0,
               0,   2,  -3,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   3,  -2,   0,
               0,   2,   1,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   1,  -2,   0,
               0,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,  -2,   0,
               0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
            };

            motdet::Image<int> img1(data1_in, 19), img1_expected(data1_expected, 19);
            std::vector<motdet::Motion_detector::Contour> img1_contours;

            motdet::imgutil::contour_detection(img1, img1_contours); // Trimming edges

            bool test_img1_map = test_compare_vectors<int, int>(img1.get_data(),img1_expected.get_data());
            CHECK_TRUE(test_img1_map);

            cont = img1_contours[0];
            bool test_img1_c2_id = cont.id == 2;
            bool test_img1_c2_parent = cont.parent == 0;
            bool test_img1_c2_hole = cont.is_hole == false;
            bool test_img1_c2_pixels = cont.n_pixels == 81;
            bool test_img1_c2_bbox = cont.bb_tl_x == 0 && cont.bb_tl_y == 1 && cont.bb_br_x == 17 && cont.bb_br_y == 25;
            bool test_img1_c2 = test_img1_c2_id && test_img1_c2_parent && test_img1_c2_hole && test_img1_c2_pixels && test_img1_c2_bbox;
            CHECK_TRUE(test_img1_c2);

            cont = img1_contours[1];
            bool test_img1_c3_id = cont.id == 3;
            bool test_img1_c3_parent = cont.parent == 2;
            bool test_img1_c3_hole = cont.is_hole == true;
            bool test_img1_c3_pixels = cont.n_pixels == 69;
            bool test_img1_c3_bbox = cont.bb_tl_x == 2 && cont.bb_tl_y == 2 && cont.bb_br_x == 16 && cont.bb_br_y == 24;
            bool test_img1_c3 = test_img1_c3_id && test_img1_c3_parent && test_img1_c3_hole && test_img1_c3_pixels && test_img1_c3_bbox;
            CHECK_TRUE(test_img1_c3);

            cont = img1_contours[2];
            bool test_img1_c4_id = cont.id == 4;
            bool test_img1_c4_parent = cont.parent == 3;
            bool test_img1_c4_hole = cont.is_hole == false;
            bool test_img1_c4_pixels = cont.n_pixels == 57;
            bool test_img1_c4_bbox = cont.bb_tl_x == 3 && cont.bb_tl_y == 4 && cont.bb_br_x == 14 && cont.bb_br_y == 22;
            bool test_img1_c4 = test_img1_c4_id && test_img1_c4_parent && test_img1_c4_hole && test_img1_c4_pixels && test_img1_c4_bbox;
            CHECK_TRUE(test_img1_c4);

            cont = img1_contours[3];
            bool test_img1_c5_id = cont.id == 5;
            bool test_img1_c5_parent = cont.parent == 4;
            bool test_img1_c5_hole = cont.is_hole == true;
            bool test_img1_c5_pixels = cont.n_pixels == 45;
            bool test_img1_c5_bbox = cont.bb_tl_x == 5 && cont.bb_tl_y == 5 && cont.bb_br_x == 13 && cont.bb_br_y == 21;
            bool test_img1_c5 = test_img1_c5_id && test_img1_c5_parent && test_img1_c5_hole && test_img1_c5_pixels && test_img1_c5_bbox;
            CHECK_TRUE(test_img1_c5);

            cont = img1_contours[4];
            bool test_img1_c6_id = cont.id == 6;
            bool test_img1_c6_parent = cont.parent == 5;
            bool test_img1_c6_hole = cont.is_hole == false;
            bool test_img1_c6_pixels = cont.n_pixels == 15;
            bool test_img1_c6_bbox = cont.bb_tl_x == 6 && cont.bb_tl_y == 7 && cont.bb_br_x == 10 && cont.bb_br_y == 11;
            bool test_img1_c6 = test_img1_c6_id && test_img1_c6_parent && test_img1_c6_hole && test_img1_c6_pixels && test_img1_c6_bbox;
            CHECK_TRUE(test_img1_c6);

            cont = img1_contours[5];
            bool test_img1_c7_id = cont.id == 7;
            bool test_img1_c7_parent = cont.parent == 5;
            bool test_img1_c7_hole = cont.is_hole == false;
            bool test_img1_c7_pixels = cont.n_pixels == 1;
            bool test_img1_c7_bbox = cont.bb_tl_x == 10 && cont.bb_tl_y == 9 && cont.bb_br_x == 10 && cont.bb_br_y == 9;
            bool test_img1_c7 = test_img1_c7_id && test_img1_c7_parent && test_img1_c7_hole && test_img1_c7_pixels && test_img1_c7_bbox;
            CHECK_TRUE(test_img1_c7);

            cont = img1_contours[6];
            bool test_img1_c8_id = cont.id == 8;
            bool test_img1_c8_parent = cont.parent == 5;
            bool test_img1_c8_hole = cont.is_hole == false;
            bool test_img1_c8_pixels = cont.n_pixels == 21;
            bool test_img1_c8_bbox = cont.bb_tl_x == 6 && cont.bb_tl_y == 13 && cont.bb_br_x == 11 && cont.bb_br_y == 19;
            bool test_img1_c8 = test_img1_c8_id && test_img1_c8_parent && test_img1_c8_hole && test_img1_c8_pixels && test_img1_c8_bbox;
            CHECK_TRUE(test_img1_c8);

            cont = img1_contours[7];
            bool test_img1_c9_id = cont.id == 9;
            bool test_img1_c9_parent = cont.parent == 5;
            bool test_img1_c9_hole = cont.is_hole == false;
            bool test_img1_c9_pixels = cont.n_pixels == 13;
            bool test_img1_c9_bbox = cont.bb_tl_x == 7 && cont.bb_tl_y == 13 && cont.bb_br_x == 9 && cont.bb_br_y == 19;
            bool test_img1_c9 = test_img1_c9_id && test_img1_c9_parent && test_img1_c9_hole && test_img1_c9_pixels && test_img1_c9_bbox;
            CHECK_TRUE(test_img1_c9);

            bool test_img1_conts = test_img1_c2 && test_img1_c3 && test_img1_c4 && test_img1_c5 && test_img1_c6 && test_img1_c7 && test_img1_c8 && test_img1_c9;

            bool test_img1 = test_img1_map && test_img1_conts;

            return test_img0 && test_img1;
        }
    } // namespace contour_detector
} // namespace test
