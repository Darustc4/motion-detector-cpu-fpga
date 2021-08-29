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
        bool ccw_not0_(const Image<int> &in, const std::size_t i0, const std::size_t j0, const std::size_t i, const std::size_t j, const unsigned char offset, std::size_t &out_i, std::size_t &out_j);

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
        bool cw_not0_(const Image<int> &in, const int i0, const int j0, const int i, const int j, const unsigned char offset, std::size_t &out_i, std::size_t &out_j);

        /**
         * @brief Follow a contour border while updating the Contour object. Modifies the input image with the ID for the contour that is being followed.
         * @param in Contour image that will be read and updated as we follow the contour.
         * @param width Length of a row in the image.
         * @param found_contour contour object that will be filled in with the found border.
         * @param i y position of the pixel that is beign analyzed.
         * @param j x position of the pixel that is beign analyzed.
         * @param i2 y position of the first neighbour to check.
         * @param j2 x position of the first neighbour to check.
         * @param nbd the id of the current border.
         * @param lnbd the id of the prebious encoutnered border.
         */
        void follow_border(Image<int> &in, std::size_t width, Extended_contour &found_contour, const std::size_t i, const std::size_t j, std::size_t i2, std::size_t j2, const int nbd, unsigned int &lnbd);



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

        bool ccw_not0_(const Image<int> &in, const std::size_t i0, const std::size_t j0, const std::size_t i, const std::size_t j, const unsigned char offset, std::size_t &out_i, std::size_t &out_j)
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


        bool cw_not0_(const Image<int> &in, const int i0, const int j0, const int i, const int j, const unsigned char offset, std::size_t &out_i, std::size_t &out_j)
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

        void contour_detection(Image<int> &conts_image, std::vector<Extended_contour> &conts, bool trim_borders)
        {
            // Topological Structural Analysis of Digitized Binary Images by Border Following.
            // By Suzuki, S. and Abe, K. 1985
            // Referece, by Lingdong Huang: https://github.com/LingDong-/PContour/blob/master/src/pcontour/PContour.java

            std::size_t height = conts_image.get_height(), width = conts_image.get_width();

            // Se the borders to zero. Required by the algorithm to avoid infinite loops and indexing errors.
            if(trim_borders)
            {
                for(std::size_t i = 0; i < height; ++i) conts_image[i*width] = conts_image[(i+1)*width-1] = 0;
                for(std::size_t i = 1; i < width-1; ++i) conts_image[i] = conts_image[(height-1)*width+i] = 0;
            }

            int nbd = 1;           // Current contour
            unsigned int lnbd = 1; // Last found contour

            // NOTE: The great majority of the comments here are excerpts from the original paper.

            // Scan the picture with a TV raster and perform the following steps for each pixel such that fij # 0.
            // Every time we begin to scan a new row of the picture, reset LNBD to 1

            for(std::size_t i = 1; i < height-1; ++i)
            {
                lnbd = 1;

                for(std::size_t j = 1; j < width-1; ++j)
                {
                    std::size_t i2 = 0, j2 = 0;

                    // If the pixel is not a contour edge, nothing to do.
                    if (conts_image[i*width + j] == 0) continue;

                    // (a) If fij = 1 and fi, j-1 = 0, then decide that the pixel (i, j) is the border following
                    // starting point of an outer border, increment NBD, and (i2, j2) <- (i, j - 1).
                    // (b) Else if fij >= 1 and fi,j+1 = 0, then decide that the pixel (i, j) is the border following starting point of a
                    // hole border, increment NBD, (i2, j2) <- (i, j + 1), and LNBD + fij in case fij > 1.
                    // (c) Otherwise, go to (4).

                    std::size_t curr_idx = i*width + j;
                    if (conts_image[curr_idx] == 1 && conts_image[curr_idx - 1] == 0)
                    {
                        ++nbd;
                        i2 = i;
                        j2 = j-1;
                    }
                    else if (conts_image[curr_idx] >= 1 && conts_image[curr_idx + 1] == 0)
                    {
                        ++nbd;
                        i2 = i;
                        j2 = j+1;
                        if (conts_image[curr_idx] > 1) lnbd = conts_image[curr_idx];
                    }
                    else
                    {
                        // (4) If fij != 1, then LNBD <- |fij| and resume the raster scan from pixel (i,j+1).
                        // The algorithm terminates when the scan reaches the lower right corner of the picture

                        if (conts_image[curr_idx] != 1) lnbd = std::abs(conts_image[curr_idx]);
                        continue;
                    }

                    // (2) Depending on the types of the newly found border
                    // and the border with the sequential number LNBD
                    // (i.e., the last border met on the current row),
                    // decide the parent of the current border.

                    // Lets start by filling in all the initial data of the contour
                    Extended_contour found_contour;
                    found_contour.id = nbd;
                    found_contour.is_hole = (j2 == j+1);
                    found_contour.n_pixels = 1;
                    found_contour.bb_br_x = j2;
                    found_contour.bb_br_y = i2;
                    found_contour.bb_tl_x = j2;
                    found_contour.bb_tl_y = i2;

                    // Now let's find it's parent.
                    // Let's assume there is no previous contour by default since lnbd could not point at anything.

                    int prev_pos = -1;
                    for(std::size_t i = 0; i < conts.size(); ++i)
                    {
                        if(conts[i].id == lnbd) // Found contour corresponding to lnbd
                        {
                            prev_pos = i;
                            break;
                        }
                    }

                    // If there is no previous contour, the current contour has no parent.
                    if(prev_pos < 0) found_contour.parent = 0;
                    else if(conts[prev_pos].is_hole)
                    {
                        if(found_contour.is_hole) found_contour.parent = conts[prev_pos].parent;
                        else found_contour.parent = lnbd;
                    }
                    else
                    {
                        if(found_contour.is_hole) found_contour.parent = lnbd;
                        else found_contour.parent = conts[prev_pos].parent;
                    }

                    // Follow the contour while updating it's data.
                    follow_border(conts_image, width, found_contour, i, j, i2, j2, nbd, lnbd);
                    conts.push_back(std::move(found_contour));
                }
            }
        }

        void follow_border(Image<int> &in, std::size_t width, Extended_contour &found_contour, const std::size_t i, const std::size_t j, std::size_t i2, std::size_t j2, const int nbd, unsigned int &lnbd)
        {
            // (3) From the starting point (i, j), follow the detected border:
            // this is done by the following substeps (3.1) through (3.5).

            // (3.1) Starting from (i2, j2), look around clockwise the pixels
            // in the neighborhood of (i, j) and find a nonzero pixel.
            // Let (i1, j1) be the first found nonzero pixel. If no nonzero
            // pixel is found, assign -NBD to fij and go to (4).

            std::size_t curr_idx = i*width + j;
            std::size_t i1 = 0, j1 = 0;
            if(!cw_not0_(in, i, j, i2, j2, 0, i1, j1))
            {
                in[curr_idx] = -nbd;

                // Did not find a valid neighbor. Going to (4).
                // NOTE: We are not actually going to (4), just replicating step (4) here again.

                if(in[curr_idx] != 1) lnbd = std::abs(in[curr_idx]);
                return;
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
                    if(found_contour.bb_br_x <= j4) found_contour.bb_br_x = j4;
                    else if(found_contour.bb_tl_x >= j4) found_contour.bb_tl_x = j4;
                    if(found_contour.bb_br_y <= i4) found_contour.bb_br_y = i4;
                    else if(found_contour.bb_tl_y >= i4) found_contour.bb_tl_y = i4;
                }
                found_contour.n_pixels++;

                // (a) If the pixel (i3, j3 + 1) is a O-pixel examined in the
                //     substep (3.3) then fi3, j3 <-  -NBD.
                // (b) If the pixel (i3, j3 + 1) is not a O-pixel examined
                //     in the substep (3.3) and fi3,j3 = 1, then fi3,j3 <- NBD.
                // (c) Otherwise, do not change fi3, j3.

                std::size_t curr_idx3 = i3*width + j3;
                if (in[curr_idx3 + 1] == 0) in[curr_idx3] = -nbd;
                else if(in[curr_idx3] == 1) in[curr_idx3] = nbd;

                // (3.5) If (i4, j4) = (i, j) and (i3, j3) = (i1, j1)
                // (coming back to the starting point), then go to (4);
                // otherwise, (i2, j2) + (i3, j3),(i3, j3) + (i4, j4), and go back to (3.3)

                if(found_pair4)
                {
                    if (i4 == i && j4 == j && i3 == i1 && j3 == j1)
                    {
                        // Followed the border completely, going to (4).

                        if (in[curr_idx] != 1) lnbd = std::abs(in[curr_idx]);
                        return;
                    }
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

