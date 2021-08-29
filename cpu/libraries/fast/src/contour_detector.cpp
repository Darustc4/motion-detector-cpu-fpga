#include "contour_detector.hpp"

namespace motdet
{
    namespace imgutil
    {
        /**
         * @brief Using a pixel x,y and a neighbour id (0 to 7), gets the neighbour x,y position.
         * @param i y position of the center pixel.
         * @param j x position of the center pixel.
         * @param id id of the neighbour to get.
         * @param out_i y position of the neighbour.
         * @param out_j x position of the neighbour.
         * @return true A valid id was given and the neighbour has been set.
         * @return false The id was invalid and out_i and out_j are invalid.
         */
        bool neighbor_id_to_index_(const std::size_t i, const std::size_t j, const unsigned char id, std::size_t &out_i, std::size_t &out_j);

        /**
         * @brief Using a pixel x0,y0 and another with position x,y, gets the neighbour id of the other pixel, if valid.
         * @param i0 y position of the center pixel.
         * @param j0 x position of the center pixel.
         * @param i y position of the other pixel to check.
         * @param j x position of the other pixel to check.
         * @return signed char id of the neighbour that was checked. -1 if it was not a neighbour.
         */
        signed char neighbor_index_to_id_(const std::size_t i0, const std::size_t j0, const std::size_t i, const std::size_t j);

        /**
         * @brief Find the first 0-pixel in the neighbourhood of a pixel x0,y0. Check neighbours rotating counterclockwise.
         * @param in Image to analyse.
         * @param i0 y position of the center pixel.
         * @param j0 x position of the center pixel.
         * @param i y position of the first neighbour to check.
         * @param j x position of the first neighbour to check.
         * @param offset neighbours to skip before actually checking. Regardless of this value, all neighbours will be checked.
         * @param out_i y position of the found pixel.
         * @param out_j x position of the found pixel.
         * @return true a valid pixel was found, and out_i, out_j is valid.
         * @return false a valid pixel was not found, and out_i, out_j is not valid.
         */
        bool ccw_not0_(const Image<unsigned char> &in, const std::size_t i0, const std::size_t j0, const std::size_t i, const std::size_t j, const unsigned char offset, std::size_t &out_i, std::size_t &out_j);

        /**
         * @brief Find the first 0-pixel in the neighbourhood of a pixel x0,y0. Check neighbours rotating clockwise.
         * @param in Image to analyse.
         * @param i0 y position of the center pixel.
         * @param j0 x position of the center pixel.
         * @param i y position of the first neighbour to check.
         * @param j x position of the first neighbour to check.
         * @param offset neighbours to skip before actually checking. Regardless of this value, all neighbours will be checked.
         * @param out_i y position of the found pixel.
         * @param out_j x position of the found pixel.
         * @return true a valid pixel was found, and out_i, out_j is valid.
         * @return false a valid pixel was not found, and out_i, out_j is not valid.
         */
        bool cw_not0_(const Image<unsigned char> &in, const int i0, const int j0, const int i, const int j, const unsigned char offset, std::size_t &out_i, std::size_t &out_j);

        /**
         * @brief Follow a contour border while updating the Contour object. Modifies the input image with the ID for the contour that is being followed.
         * @param in Contour image that will be read and updated as we follow the contour.
         * @param width Length of a row in the image.
         * @param i y position of the pixel that is beign analyzed.
         * @param j x position of the pixel that is beign analyzed.
         * @param i2 y position of the first neighbour to check.
         * @param j2 x position of the first neighbour to check.
         * @param nbd the id of the current border.
         * @param lnbd the id of the prebious encoutnered border.
         * @return Contour that has been followed.
         */
        Contour follow_border(Image<unsigned char> &in, std::size_t width, const std::size_t i, const std::size_t j, std::size_t i2, std::size_t j2);



        bool neighbor_id_to_index_(const std::size_t i, const std::size_t j, const unsigned char id, std::size_t &out_i, std::size_t &out_j)
        {
            switch(id){
                case 0:  out_i = i  ; out_j = j+1; return true;
                case 1:  out_i = i-1; out_j = j+1; return true;
                case 2:  out_i = i-1; out_j = j  ; return true;
                case 3:  out_i = i-1; out_j = j-1; return true;
                case 4:  out_i = i  ; out_j = j-1; return true;
                case 5:  out_i = i+1; out_j = j-1; return true;
                case 6:  out_i = i+1; out_j = j  ; return true;
                case 7:  out_i = i+1; out_j = j+1; return true;
                default: return false;
            }
        }

        signed char neighbor_index_to_id_(const std::size_t i0, const std::size_t j0, const std::size_t i, const std::size_t j)
        {
            signed char di = i - i0;
            signed char dj = j - j0;

            if (di ==  0 && dj ==  1) return 0;
            if (di == -1 && dj ==  1) return 1;
            if (di == -1 && dj ==  0) return 2;
            if (di == -1 && dj == -1) return 3;
            if (di ==  0 && dj == -1) return 4;
            if (di ==  1 && dj == -1) return 5;
            if (di ==  1 && dj ==  0) return 6;
            if (di ==  1 && dj ==  1) return 7;

            return -1;
        }

        bool ccw_not0_(const Image<unsigned char> &in, const std::size_t i0, const std::size_t j0, const std::size_t i, const std::size_t j, const unsigned char offset, std::size_t &out_i, std::size_t &out_j)
        {
            signed char id = neighbor_index_to_id_(i0,j0,i,j);
            std::size_t width = in.get_width();

            for (std::size_t k = 0; k < 8; ++k)
            {
                int kk = (k + id + offset) % 8;
                neighbor_id_to_index_(i0, j0, kk, out_i, out_j);

                if (in[out_i*width + out_j] != 0) return true;
            }
            return false;
        }


        bool cw_not0_(const Image<unsigned char> &in, const int i0, const int j0, const int i, const int j, const unsigned char offset, std::size_t &out_i, std::size_t &out_j)
        {
            signed char id = neighbor_index_to_id_(i0,j0,i,j);
            std::size_t width = in.get_width();

            for (std::size_t k = 0; k < 8; ++k)
            {
                int kk = (-k + id - offset) % 8;
                neighbor_id_to_index_(i0, j0, kk, out_i, out_j);

                if (in[out_i*width + out_j] != 0) return true;
            }
            return false;
        }

        std::vector<Contour> contour_detection(Image<unsigned char> &conts_image, bool trim_borders)
        {
            // Modified Topological Structural Analysis of Digitized Binary Images by Border Following.
            // By Suzuki, S. and Abe, K. 1985
            // Referece, by Lingdong Huang: https://github.com/LingDong-/PContour/blob/master/src/pcontour/PContour.java

            // Edited to work on a unsigned char image, without analyzing hierarchy or topology.
            // 0 = No pixel. 1 = Unexplore or not border. 2 = Explored border. 3 = Explored end of border,

            std::size_t height = conts_image.get_height(), width = conts_image.get_width();
            std::vector<Contour> detections;

            // Se the borders to zero. Required by the algorithm to avoid infinite loops and indexing errors.
            if(trim_borders)
            {
                for(std::size_t i = 0; i < height; ++i) conts_image[i*width] = conts_image[(i+1)*width-1] = 0;
                for(std::size_t i = 1; i < width-1; ++i) conts_image[i] = conts_image[(height-1)*width+i] = 0;
            }

            for(std::size_t i = 1; i < height-1; ++i)
            {
                for(std::size_t j = 1; j < width-1; ++j)
                {
                    std::size_t i2 = 0, j2 = 0;

                    // If the pixel is not a contour edge, nothing to do.
                    if (conts_image[i*width + j] == 0) continue;

                    std::size_t curr_idx = i*width + j;
                    if (conts_image[curr_idx] == 1 && conts_image[curr_idx - 1] == 0)
                    {
                        i2 = i;
                        j2 = j-1;
                    }
                    else if ((conts_image[curr_idx] == 1 || conts_image[curr_idx] == 2) && conts_image[curr_idx + 1] == 0)
                    {
                        i2 = i;
                        j2 = j+1;
                    }
                    else continue;

                    Contour detection = follow_border(conts_image, width, i, j, i2, j2);
                    detections.push_back(detection);
                }
            }

            return detections;
        }

        Contour follow_border(Image<unsigned char> &in, std::size_t width, const std::size_t i, const std::size_t j, std::size_t i2, std::size_t j2)
        {
            Contour detection(j2, i2, j2, i2);

            std::size_t curr_idx = i*width + j;
            std::size_t i1 = 0, j1 = 0;
            if(!cw_not0_(in, i, j, i2, j2, 0, i1, j1)){
                in[curr_idx] = 3;
                return detection;
            }

            // (3.2) (i2, j2) <- (i1, j1) and (i3,j3) <- (i, j).
            i2 = i1;
            j2 = j1;
            std::size_t i3 = i;
            std::size_t j3 = j;

            // (3.3) Starting from the next element of the pixel (i2, j2)
            // in the counterclockwise order, examine counterclockwise
            // the pixels in the neighborhood of the current pixel (i3, j3)
            // to find a nonzero pixel and let the first one be (i4, j4).

            while(true)
            {
                std::size_t i4 = 0, j4 = 0;
                bool found_pair4 = ccw_not0_(in, i3, j3, i2, j2, 1, i4, j4);

                // Update bbox and amount of border pixels.
                if(found_pair4)
                {
                    if(detection.bb_br_x <= j4) detection.bb_br_x = j4;
                    else if(detection.bb_tl_x >= j4) detection.bb_tl_x = j4;
                    if(detection.bb_br_y <= i4) detection.bb_br_y = i4;
                    else if(detection.bb_tl_y >= i4) detection.bb_tl_y = i4;
                }

                std::size_t curr_idx3 = i3*width + j3;
                if(in[curr_idx3 + 1] == 0) in[curr_idx3] = 3;
                else if(in[curr_idx3] == 1) in[curr_idx3] = 2;

                if(found_pair4)
                {
                    if (i4 == i && j4 == j && i3 == i1 && j3 == j1) { return detection; }
                    else
                    {
                        i2 = i3;
                        j2 = j3;
                        i3 = i4;
                        j3 = j4;
                    }
                }
            }
        }
    } // namespace imgutil
} // namespace motdet

