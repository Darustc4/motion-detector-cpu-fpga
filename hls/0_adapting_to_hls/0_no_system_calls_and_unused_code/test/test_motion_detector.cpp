#include "test_motion_detector.hpp"
#include "test_utils.hpp"

#include <iostream>

namespace test
{
    namespace motion_detector
    {
        void test_all()
        {
            std::cout << "Testing module motion_detector..." << std::endl;

            log_test_result(test_image_constructor(), "Image constructor");
            log_test_result(test_image_assignment(), "Image operator=");
            log_test_result(test_image_indexing(), "Image operator[]");
            log_test_result(test_image_getset(), "Image getter and setter");

            log_test_result(test_motion_detector_constructor(), "Motion_detector constructor");
            log_test_result(test_motion_detector_detect_motion(), "Motion_detector detect_motion");

            log_test_result(test_uchar_to_bw(), "uchar to BW");

            std::cout << "Finished tests for module motion_detector." << std::endl << std::endl;
        }

        bool test_image_constructor()
        {
            motdet::Image<unsigned int> img0(100, 1, 3);

            bool test_img0_size = img0.get_total() == 100;
            CHECK_TRUE(test_img0_size);
            bool test_img0_width = img0.get_width() == 100;
            CHECK_TRUE(test_img0_width);
            bool test_img0_height = img0.get_height() == 1;
            CHECK_TRUE(test_img0_height);
            bool test_img0_default_val = img0[99] == 3;
            CHECK_TRUE(test_img0_default_val);
            bool test_img0 = test_img0_size && test_img0_width && test_img0_height && test_img0_default_val;

            std::vector<unsigned char> data1_in = {
               0,   0,   0,   0,   0,   0,   0,   1,   1,   1,
               0,   0,   0,   1,   1,   0,   0,   1,   1,   1,
               0,   1,   1,   1,   1,   1,   1,   1,   1,   1,
               0,   1,   1,   0,   0,   0,   1,   1,   1,   0,
               1,   1,   0,   0,   0,   0,   1,   1,   1,   0,
               1,   1,   0,   0,   0,   0,   1,   1,   1,   0,
               1,   1,   0,   0,   0,   1,   1,   0,   0,   0,
               0,   1,   1,   1,   1,   1,   0,   0,   0,   1,
               0,   0,   0,   0,   0,   0,   0,   0,   1,  10
            };

            motdet::Image<unsigned char> img1(data1_in, 10);

            bool test_img1_size = img1.get_total() == 90;
            CHECK_TRUE(test_img1_size);
            bool test_img1_width = img1.get_width() == 10;
            CHECK_TRUE(test_img1_width);
            bool test_img1_height = img1.get_height() == 9;
            CHECK_TRUE(test_img1_height);
            bool test_img1_val = img1[89] == 10;
            CHECK_TRUE(test_img1_val);
            bool test_img1 = test_img1_size && test_img1_width && test_img1_height && test_img1_val;

            motdet::Image<unsigned char> img2(img1);

            bool test_img2_size = img2.get_total() == 90;
            CHECK_TRUE(test_img2_size);
            bool test_img2_width = img2.get_width() == 10;
            CHECK_TRUE(test_img2_width);
            bool test_img2_height = img2.get_height() == 9;
            CHECK_TRUE(test_img2_height);
            bool test_img2_val = img2[89] == 10;
            CHECK_TRUE(test_img2_val);
            bool test_img2 = test_img2_size && test_img2_width && test_img2_height && test_img2_val;

            motdet::Image<unsigned char> img3(std::move(img2));

            bool test_img3_size = img3.get_total() == 90;
            CHECK_TRUE(test_img3_size);
            bool test_img3_width = img3.get_width() == 10;
            CHECK_TRUE(test_img3_width);
            bool test_img3_height = img3.get_height() == 9;
            CHECK_TRUE(test_img3_height);
            bool test_img3_val = img3[89] == 10;
            CHECK_TRUE(test_img3_val);
            bool test_img3_move_successful = img2.get_data().size() == 0;
            CHECK_TRUE(test_img3_move_successful);

            bool test_img3 = test_img3_size && test_img3_width && test_img3_height && test_img3_val && test_img3_move_successful;

            // Check exceptions

            bool test_exc0 = false;
            try
            {
                motdet::Image<unsigned char> img_exc(0, 0, 4);
            }
            catch(const std::invalid_argument &e)
            {
                test_exc0 = true;
            }
            CHECK_TRUE(test_exc0);

            bool test_exc1 = false;
            try
            {
                motdet::Image<unsigned char> img_exc(1, 0, 4);
            }
            catch(const std::invalid_argument &e)
            {
                test_exc1 = true;
            }
            CHECK_TRUE(test_exc1);

            bool test_exc2 = false;
            try
            {
                motdet::Image<unsigned char> img_exc(0, 1, 4);
            }
            catch(const std::invalid_argument &e)
            {
                test_exc2 = true;
            }
            CHECK_TRUE(test_exc2);

            bool test_exc3 = false;
            try
            {
                motdet::Image<unsigned char> img_exc(1, 1, 4);
                test_exc3 = true;
            }
            catch(const std::invalid_argument &e){}
            CHECK_TRUE(test_exc3);

            bool test_exc4 = false;
            try
            {
                motdet::Image<unsigned char> img_exc(data1_in, 4);
            }
            catch(const std::invalid_argument &e)
            {
                test_exc4 = true;
            }
            CHECK_TRUE(test_exc4);

            bool test_exc5 = false;
            try
            {
                motdet::Image<unsigned char> img_exc(data1_in, 0);
            }
            catch(const std::invalid_argument &e)
            {
                test_exc5 = true;
            }
            CHECK_TRUE(test_exc5);

            bool test_exc = test_exc0 && test_exc1 && test_exc2 && test_exc3 && test_exc4 && test_exc5;

            return test_img0 && test_img1 && test_img2 && test_img3 && test_exc;
        }

        bool test_image_assignment()
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
               0,   0,   0,   0,   0,   0,   0,   0,   1,  10
            };

            motdet::Image<unsigned char> img0(data0_in, 10);

            motdet::Image<unsigned char> img1(10, 9, {});
            img1 = img0;

            bool test_img1_size = img1.get_total() == 90;
            CHECK_TRUE(test_img1_size);
            bool test_img1_width = img1.get_width() == 10;
            CHECK_TRUE(test_img1_width);
            bool test_img1_height = img1.get_height() == 9;
            CHECK_TRUE(test_img1_height);
            bool test_img1_val = img1[89] == 10;
            CHECK_TRUE(test_img1_val);
            bool test_img1 = test_img1_size && test_img1_width && test_img1_height && test_img1_val;

            motdet::Image<unsigned char> img2(10, 9, {});
            img2 = std::move(img1);

            bool test_img2_size = img2.get_total() == 90;
            CHECK_TRUE(test_img2_size);
            bool test_img2_width = img2.get_width() == 10;
            CHECK_TRUE(test_img2_width);
            bool test_img2_height = img2.get_height() == 9;
            CHECK_TRUE(test_img2_height);
            bool test_img2_val = img2[89] == 10;
            CHECK_TRUE(test_img2_val);
            bool test_img2_move_successful = img1.get_data().size() == 0;
            CHECK_TRUE(test_img2_move_successful);
            bool test_img2 = test_img2_size && test_img2_width && test_img2_height && test_img2_val && test_img2_move_successful;

            motdet::Image<unsigned char> img3(12, 3, {});
            img3 = img2;

            bool test_img3_size = img3.get_total() == 90;
            CHECK_TRUE(test_img3_size);
            bool test_img3_width = img3.get_width() == 10;
            CHECK_TRUE(test_img3_width);
            bool test_img3_height = img3.get_height() == 9;
            CHECK_TRUE(test_img3_height);
            bool test_img3_val = img3[89] == 10;
            CHECK_TRUE(test_img3_val);
            bool test_img3 = test_img3_size && test_img3_width && test_img3_height && test_img3_val;

            return test_img1 && test_img2 && test_img3;
        }

        bool test_image_indexing()
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
               0,   0,   0,   0,   0,   0,   0,   0,   1,  10
            };

            motdet::Image<unsigned char> img0(data0_in, 10);

            const unsigned char const_elem = img0[9];
            bool test_img0_const_index = const_elem == 1;
            CHECK_TRUE(test_img0_const_index);

            img0[9] = 223;
            bool test_img0_edit_index = img0[9] == 223;
            CHECK_TRUE(test_img0_edit_index);

            return test_img0_const_index && test_img0_edit_index;
        }

        bool test_image_getset()
        {
            std::vector<unsigned char> data0_in_0 = {
               0,   0,   0,   0,   0,   0,   0,   1,   1,   1,
               0,   0,   0,   1,   1,   0,   0,   1,   1,   1
            };

            std::vector<unsigned char> data0_in_1 = {
               1,   0,   0,   0,   0,   0,   0,   2,   0,
               0,   3,   0,   0,   0,   0,   0,   2,   0
            };

            motdet::Image<unsigned char> img0(data0_in_0, 10);

            bool test_img0_size = img0.get_total() == 20;
            CHECK_TRUE(test_img0_size);
            bool test_img0_width = img0.get_width() == 10;
            CHECK_TRUE(test_img0_width);
            bool test_img0_height = img0.get_height() == 2;
            CHECK_TRUE(test_img0_height);

            bool test_img0_get_data = img0.get_data() == data0_in_0;
            CHECK_TRUE(test_img0_get_data);

            img0.set_data(data0_in_1, 9);
            bool test_img0_set_data = img0.get_data() == data0_in_1;
            CHECK_TRUE(test_img0_set_data);

            std::vector<unsigned char> data0_in_0_copy = data0_in_0;
            img0.set_data(std::move(data0_in_0), 10);
            bool test_img0_set_data_move = img0.get_data() == data0_in_0_copy;
            CHECK_TRUE(test_img0_set_data_move);
            bool test_img0_set_data_move_successful = data0_in_0.size() == 0;
            CHECK_TRUE(test_img0_set_data_move_successful);

            bool test_img0 = test_img0_size && test_img0_width && test_img0_height && test_img0_get_data && test_img0_set_data && test_img0_set_data_move && test_img0_set_data_move_successful;

            // Test exceptions

            bool test_exc0 = false;
            try
            {
                img0.set_data(data0_in_1, 8);
            }
            catch(const std::invalid_argument &e)
            {
                test_exc0 = true;
            }
            CHECK_TRUE(test_exc0);

            bool test_exc1_throw = false;
            try
            {
                img0.set_data(std::move(data0_in_1), 8);
            }
            catch(const std::invalid_argument &e)
            {
                test_exc1_throw = true;
            }
            CHECK_TRUE(test_exc1_throw);
            bool test_exc1_move = data0_in_1.size() != 0;
            CHECK_TRUE(test_exc1_move);

            bool test_exc1 = test_exc1_throw && test_exc1_move;

            bool test_exc = test_exc0 && test_exc1;

            return test_img0 && test_exc;
        }


        bool test_motion_detector_constructor()
        {
            motdet::Motion_detector motdet0(10, 15);
            motdet::Motion_detector motdet1(15, 10, 2, 0.002);

            // Check exceptions

            bool test_exc0 = false;
            try
            {
                motdet::Motion_detector motdet(3, 10, 2, 0.002);
            }
            catch(const std::invalid_argument &e)
            {
                test_exc0 = true;
            }
            CHECK_TRUE(test_exc0);

            bool test_exc1 = false;
            try
            {
                motdet::Motion_detector motdet(22, 9, 2, 0.002);
            }
            catch(const std::invalid_argument &e)
            {
                test_exc1 = true;
            }
            CHECK_TRUE(test_exc1);


            bool test_exc2 = false;
            try
            {
                motdet::Motion_detector motdet(30, 10, 0, 0.002);
            }
            catch(const std::invalid_argument &e)
            {
                test_exc2 = true;
            }
            CHECK_TRUE(test_exc2);

            bool test_exc = test_exc0 && test_exc1 && test_exc2;

            return test_exc;
        }

        bool test_motion_detector_detect_motion()
        {
            motdet::Motion_detector motdet0(15, 15);

            std::vector<unsigned short> data0_in0 = {
             0,     0,     0, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000,     0,     0,     0,     0,
             0,     0, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000,     0,     0,     0,
             0, 13000, 13000, 13000, 13000, 13000, 23000, 13000, 13000, 13000, 13000, 13000, 13000,     0,     0,
             0, 13000, 13000, 13000, 13000, 13000, 23000, 23000, 13000, 13000, 13000, 13000, 13000, 13000,     0,
             0, 13000, 13000, 13000, 13000, 13000, 23000, 23000, 13000, 13000, 13000, 13000, 13000, 13000,     0,
             0,     0, 13000, 13000, 13000, 13000, 23000, 23000, 13000, 13000, 13000, 13000, 13000, 13000,     0,
             0,     0, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000,     0, 13000, 13000,     0,
             0,     0, 13000, 13000, 13000, 13000, 13000, 13000, 13000,     0,     0,     0,     0,     0,     0,
             0,     0, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000,     0,     0,     0,     0,
             0,     0, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000,     0,     0,
             0,     0,     0, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000,     0,
             0,     0,     0, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000,     0,
             0,     0,     0, 13000, 13000, 13000, 13000,     0, 13000, 13000, 13000, 13000, 13000, 13000,     0,
             0,     0,     0, 13000, 13000, 13000, 13000,     0,     0, 13000, 13000, 13000, 13000, 13000,     0,
             0,     0,     0, 13000, 13000, 13000,     0,     0,     0,     0, 13000, 13000, 13000, 13000,     0
            };

            std::vector<unsigned short> data0_in1 = {
             0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
             0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 13000,     0,     0,     0,
             0, 13000,     0,     0,     0,     0,     0,     0,     0,     0,     0, 13000, 13000,     0,     0,
             0, 13000, 13000,     0,     0,     0,     0,     0,     0,     0,     0, 13000, 13000, 13000,     0,
             0, 13000, 13000,     0,     0,     0,     0,     0,     0,     0,     0, 13000, 13000, 13000,     0,
             0,     0, 13000,     0,     0,     0,     0,     0,     0,     0, 13000, 13000, 13000, 13000,     0,
             0,     0, 13000,     0,     0,     0,     0,     0,     0, 13000, 13000,     0, 13000, 13000,     0,
             0,     0, 13000, 13000,     0,     0,     0,     0, 13000,     0,     0,     0,     0,     0,     0,
             0,     0, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 63000,     0,     0,     0,     0,
             0,     0, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 63000, 60000, 63000,     0,     0,
             0,     0,     0, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 63000, 63000, 63000,     0,
             0,     0,     0, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 63000, 63000, 63000, 63000,     0,
             0,     0,     0, 13000, 13000, 13000, 13000,     0, 13000, 13000, 63000, 63000, 63000, 63000,     0,
             0,     0,     0, 13000, 13000, 13000, 13000,     0,     0, 13000, 63000, 63000, 63000, 63000,     0,
             0,     0,     0, 13000, 13000, 13000,     0,     0,     0,     0, 63000, 63000, 63000, 13000,     0
            };

            std::vector<unsigned short> data0_in2 = {
             0,     0,     0, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000,     0,     0,     0,     0,
             0,     0, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000,     0,     0,     0,
             0, 13000, 13000, 13000, 13000, 13000, 23000, 13000, 13000, 13000, 13000, 13000, 13000,     0,     0,
             0, 13000, 13000, 13000, 13000, 13000, 23000, 23000, 13000, 13000, 13000, 13000, 13000, 13000,     0,
             0, 13000, 13000, 13000, 13000, 13000, 23000, 23000, 13000, 13000, 13000, 13000, 13000, 13000,     0,
             0,     0, 13000, 13000, 13000, 13000, 23000, 23000, 13000, 13000, 13000, 13000, 13000, 13000,     0,
             0,     0, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000,     0, 13000, 13000,     0,
             0,     0, 13000, 13000, 13000, 13000, 13000, 13000, 13000,     0,     0,     0,     0,     0,     0,
             0,     0, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000,     0,     0,     0,     0,
             0,     0, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000,     0,     0,
             0,     0,     0, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000,     0,
             0,     0,     0, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000, 13000,     0,
             0,     0,     0, 13000, 13000, 13000, 13000,     0, 13000, 13000, 13000, 13000, 13000, 13000,     0,
             0,     0,     0, 13000, 13000, 13000, 13000,     0,     0, 13000, 13000, 13000, 13000, 13000,     0,
             0,     0,     0, 13000, 13000, 13000,     0,     0,     0,     0, 13000, 13000, 13000, 13000,     0
            };

            motdet::Image<unsigned short> img0_in0(data0_in0, 15);
            motdet::Image<unsigned short> img0_in1(data0_in1, 15);
            motdet::Image<unsigned short> img0_in2(data0_in2, 15);


            motdet::Motion_detector::Detection cnt0_out0, cnt0_out1, cnt0_out2;

            cnt0_out0 = motdet0.detect_motion(img0_in0);
            bool test_motdet0_detection0 = !cnt0_out0.has_detections;
            cnt0_out1 = motdet0.detect_motion(img0_in1);
            bool test_motdet0_detection1 = cnt0_out1.has_detections;
            cnt0_out2 = motdet0.detect_motion(img0_in2);
            bool test_motdet0_detection2 = !cnt0_out2.has_detections;

            bool test_motdet0_detection = test_motdet0_detection0 && test_motdet0_detection1 && test_motdet0_detection2;

            // Test exceptions

            bool test_exc = false;
            try
            {
                motdet::Motion_detector motdetexc(10, 10, 1, 2);

                motdet::Image<unsigned short> img0_in0(data0_in0, 15);

                motdetexc.detect_motion(img0_in0);
            }
            catch(const std::invalid_argument &e)
            {
                test_exc = true;
            }
            CHECK_TRUE(test_exc);

            return test_motdet0_detection && test_exc;
        }

        bool test_uchar_to_bw()
        {
            // Each pixel is represented by 3 consecutive RGB values
            std::unique_ptr<unsigned char[]> data0_in(new unsigned char[81]{
               0, 255,   0,   0,   0,   0,   0, 255, 255,
               0,   0,   0, 255, 255,   0,   0, 255, 255,
               0, 255, 255, 255, 255, 255, 255, 255, 255,
               0, 255, 255,   0,   0,   0, 255, 255, 255,
             255, 255,   0,   0,   0,   0, 255, 255, 255,
             255, 255,   0,   0,   0,   0, 255, 255, 255,
             255, 255,   0,   0,   0, 255, 255,   0,   0,
               0, 255, 200, 255, 255, 255,   0,   0,   0,
               0,   0,   0,   0,   0,   0,   0,   0, 255
            });

            std::vector<unsigned short> data0_expected = {
             38169,     0, 45582,
                 0, 57612, 45582,
             45582, 65025, 65025,
             45582,     0, 65025,
             57612,     0, 65025,
             57612,     0, 65025,
             57612,  7412, 19442,
             43983, 65025,     0,
                 0,     0,  7412
            };

            motdet::Image<unsigned short> img0_out(3, 9, 0), img0_expected(data0_expected, 3);

            motdet::uchar_to_bw(data0_in.get(), 27, img0_out);

            bool test_img0 = test_compare_vectors<unsigned short, unsigned short>(img0_out.get_data(),img0_expected.get_data());
            CHECK_TRUE(test_img0);

            return test_img0;
        }

    } // namespace image_utils
} // namespace test
