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
        bool neighbor_id_to_index_(const uint16_t i, const uint16_t j, const uint8_t id, uint16_t &out_i, uint16_t &out_j);

        /**
         * @brief Using a pixel x0,y0 and another with position x,y, gets the neighbour id of the other pixel, if valid.
         * @param i0 y position of the center pixel.
         * @param j0 x position of the center pixel.
         * @param i y position of the other pixel to check.
         * @param j x position of the other pixel to check.
         * @return char id of the neighbour that was checked. -1 if it was not a neighbour.
         */
        uint8_t neighbor_index_to_id_(const uint16_t i0, const uint16_t j0, const uint16_t i, const uint16_t j);

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
        bool ccw_not0_(const Image<uint8_t, motdet_total> &in, const uint16_t i0, const uint16_t j0, const uint16_t i, const uint16_t j, const uint8_t offset, uint16_t &out_i, uint16_t &out_j);

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
        bool cw_not0_(const Image<uint8_t, motdet_total> &in, const uint16_t i0, const uint16_t j0, const uint16_t i, const uint16_t j, const uint8_t offset, uint16_t &out_i, uint16_t &out_j);

        /**
         * @brief Follow a contour border while updating the Contour object. Modifies the input image with the ID for the contour that is being followed.
         * @param in Contour image that will be read and updated as we follow the contour.
         * @param width Length of a row in the image.
         * @param found_contour contour object that will be filled in with the found border.
         * @param i y position of the pixel that is beign analyzed.
         * @param j x position of the pixel that is beign analyzed.
         * @param i2 y position of the first neighbour to check.
         * @param j2 x position of the first neighbour to check.
         */
        void follow_border(Image<uint8_t, motdet_total> &in, uint16_t width, Contour &found_contour, const uint16_t i, const uint16_t j, uint16_t i2, uint16_t j2);



       bool neighbor_id_to_index_(const uint16_t i, const uint16_t j, const uint8_t id, uint16_t &out_i, uint16_t &out_j)
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

        uint8_t neighbor_index_to_id_(const uint16_t i0, const uint16_t j0, const uint16_t i, const uint16_t j)
        {
            int8_t di = i - i0;
            int8_t dj = j - j0;

            if (di ==  0 && dj ==  1) return 0;
            if (di == -1 && dj ==  1) return 1;
            if (di == -1 && dj ==  0) return 2;
            if (di == -1 && dj == -1) return 3;
            if (di ==  0 && dj == -1) return 4;
            if (di ==  1 && dj == -1) return 5;
            if (di ==  1 && dj ==  0) return 6;
            return 7;
        }

        bool ccw_not0_(const Image<uint8_t, motdet_total> &in, const uint16_t i0, const uint16_t j0, const uint16_t i, const uint16_t j, const uint8_t offset, uint16_t &out_i, uint16_t &out_j)
        {
            uint8_t id = neighbor_index_to_id_(i0,j0,i,j);

            for (uint8_t k = 0; k < 8; ++k)
            {
                uint8_t kk = (k + id + offset) % 8;
                neighbor_id_to_index_(i0, j0, kk, out_i, out_j);

                if (in[out_i*motdet_width + out_j] != 0) return true;
            }
            return false;
        }


        bool cw_not0_(const Image<uint8_t, motdet_total> &in, const uint16_t i0, const uint16_t j0, const uint16_t i, const uint16_t j, const uint8_t offset, uint16_t &out_i, uint16_t &out_j)
        {
            uint8_t id = neighbor_index_to_id_(i0,j0,i,j);

            for (uint8_t k = 0; k < 8; ++k)
            {
                uint8_t kk = (-k + id - offset) % 8;
                neighbor_id_to_index_(i0, j0, kk, out_i, out_j);

                if (in[out_i*motdet_width + out_j] != 0) return true;
            }
            return false;
        }

        void contour_detection(Image<uint8_t, motdet_total> &conts_image, Contour_package &conts, bool trim_borders)
        {
            // Topological Structural Analysis of Digitized Binary Images by Border Following.
            // By Suzuki, S. and Abe, K. 1985
            // Referece, by Lingdong Huang: https://github.com/LingDong-/PContour/blob/master/src/pcontour/PContour.java

            // Se the borders to zero. Required by the algorithm to avoid infinite loops and indexing errors.
            if(trim_borders)
            {
                for(uint16_t i = 0; i < motdet_height; ++i) conts_image[i*motdet_width] = conts_image[(i+1)*motdet_width-1] = 0;
                for(uint16_t i = 1; i < motdet_width-1; ++i) conts_image[i] = conts_image[(motdet_height-1)*motdet_width+i] = 0;
            }

            // NOTE: The great majority of the comments here are excerpts from the original paper.
            // Scan the picture with a TV raster and perform the following steps for each pixel such that fij # 0.
            // Every time we begin to scan a new row of the picture, reset LNBD to 1

            for(uint16_t i = 1; i < motdet_height-1; ++i)
            {
                for(uint16_t j = 1; j < motdet_width-1; ++j)
                {
                    uint16_t i2 = 0, j2 = 0;

                    // If the pixel is not a contour edge, nothing to do.
                    if (conts_image[i*motdet_width + j] == 0) continue;

                    // (a) If fij = 1 and fi, j-1 = 0, then decide that the pixel (i, j) is the border following
                    // starting point of an outer border, increment NBD, and (i2, j2) <- (i, j - 1).
                    // (b) Else if fij >= 1 and fi,j+1 = 0, then decide that the pixel (i, j) is the border following starting point of a
                    // hole border, increment NBD, (i2, j2) <- (i, j + 1), and LNBD + fij in case fij > 1.
                    // (c) Otherwise, go to (4).

                    uint32_t curr_idx = i*motdet_width + j;
                    if (conts_image[curr_idx] == 1 && conts_image[curr_idx - 1] == 0)
                    {
                        i2 = i;
                        j2 = j-1;
                    }
                    else if (conts_image[curr_idx] == 2 && conts_image[curr_idx + 1] == 0)
                    {
                        i2 = i;
                        j2 = j+1;
                    }
                    else continue;


                    // (2) Depending on the types of the newly found border
                    // and the border with the sequential number LNBD
                    // (i.e., the last border met on the current row),
                    // decide the parent of the current border.

                    // Lets start by filling in all the initial data of the contour
                    Contour &found_contour = conts.contours[conts.contour_count];
                    found_contour.bb_br_x = j2;
                    found_contour.bb_br_y = i2;
                    found_contour.bb_tl_x = j2;
                    found_contour.bb_tl_y = i2;

                    // Follow the contour while updating it's data.
                    follow_border(conts_image, motdet_width, found_contour, i, j, i2, j2);
                    ++conts.contour_count;
                    if(conts.contour_count >= max_contours) return;
                }
            }
        }

        void follow_border(Image<uint8_t, motdet_total> &in, uint16_t width, Contour &found_contour, const uint16_t i, const uint16_t j, uint16_t i2, uint16_t j2)
        {
            // (3) From the starting point (i, j), follow the detected border:
            // this is done by the following substeps (3.1) through (3.5).

            // (3.1) Starting from (i2, j2), look around clockwise the pixels
            // in the neighborhood of (i, j) and find a nonzero pixel.
            // Let (i1, j1) be the first found nonzero pixel. If no nonzero
            // pixel is found, assign -NBD to fij and go to (4).

            uint32_t curr_idx = i*width + j;
            uint16_t i1 = 0, j1 = 0;
            if(!cw_not0_(in, i, j, i2, j2, 0, i1, j1))
            {
                in[curr_idx] = 3;
                return;
            }

            // (3.2) (i2, j2) <- (i1, j1) and (i3,j3) <- (i, j).
            i2 = i1;
            j2 = j1;
            uint16_t i3 = i;
            uint16_t j3 = j;

            // (3.3) Starting from the next element of the pixel (i2, j2)
            // in the counterclockwise order, examine counterclockwise
            // the pixels in the neighborhood of the current pixel (i3, j3)
            // to find a nonzero pixel and let the first one be (i4, j4).

            while(true)
            {
                uint16_t i4 = 0, j4 = 0;
                bool found_pair4 = ccw_not0_(in, i3, j3, i2, j2, 1, i4, j4);

                // Update bbox and amount of border pixels.
                if(found_pair4)
                {
                    if(found_contour.bb_br_x <= j4) found_contour.bb_br_x = j4;
                    else if(found_contour.bb_tl_x >= j4) found_contour.bb_tl_x = j4;
                    if(found_contour.bb_br_y <= i4) found_contour.bb_br_y = i4;
                    else if(found_contour.bb_tl_y >= i4) found_contour.bb_tl_y = i4;
                }

                // (a) If the pixel (i3, j3 + 1) is a O-pixel examined in the
                //     substep (3.3) then fi3, j3 <-  -NBD.
                // (b) If the pixel (i3, j3 + 1) is not a O-pixel examined
                //     in the substep (3.3) and fi3,j3 = 1, then fi3,j3 <- NBD.
                // (c) Otherwise, do not change fi3, j3.

                uint32_t curr_idx3 = i3*width + j3;
                if (in[curr_idx3 + 1] == 0) in[curr_idx3] = 3;
                else if(in[curr_idx3] == 1) in[curr_idx3] = 2;

                // (3.5) If (i4, j4) = (i, j) and (i3, j3) = (i1, j1)
                // (coming back to the starting point), then go to (4);
                // otherwise, (i2, j2) + (i3, j3),(i3, j3) + (i4, j4), and go back to (3.3)

                if(found_pair4)
                {
                    if (i4 == i && j4 == j && i3 == i1 && j3 == j1) return;
                    i2 = i3;
                    j2 = j3;
                    i3 = i4;
                    j3 = j4;
                }
            }
        }
    } // namespace imgutil
} // namespace motdet
