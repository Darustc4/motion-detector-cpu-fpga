#include "contour_detector.hpp"

namespace motdet
{
    namespace imgutil
    {
        namespace
        {
            class Disjoint_contour_connector
            {
            public:
                Disjoint_contour_connector(){}

                uint16_t add_cont(uint16_t point_x, uint16_t point_y)
                {
                    if(conts.contour_count >= motdet_max_contours) return 0xFFFF;

                    uint16_t new_tag = conts.contour_count++;
                    conts.contours[new_tag].bb_br_x = point_x;
                    conts.contours[new_tag].bb_tl_x = point_x;
                    conts.contours[new_tag].bb_br_y = point_y;
                    conts.contours[new_tag].bb_tl_y = point_y;
                    parents[new_tag] = 0xFFFF;
                    return new_tag;
                }

                void update_cont(uint16_t tag, uint16_t point_x, uint16_t point_y)
                {
                    uint16_t repr = get_repr(tag);
                    if(point_x < conts.contours[repr].bb_tl_x) conts.contours[repr].bb_tl_x = point_x;
                    else if(point_x > conts.contours[repr].bb_br_x) conts.contours[repr].bb_br_x = point_x;
                    if(point_y < conts.contours[repr].bb_tl_y) conts.contours[repr].bb_tl_y = point_y;
                    else if(point_y > conts.contours[repr].bb_br_y) conts.contours[repr].bb_br_y = point_y;
                }

                uint16_t get_repr(uint16_t c)
                {
                    uint16_t og_c = c;
                    uint16_t parent = parents[c];
                    while(parent != 0xFFFF)
                    {
#pragma HLS LOOP_TRIPCOUNT avg=1 max=2 min=0
                        c = parents[c];
                        parent = parents[c];
                    }
                    if(og_c != c) parents[og_c] = c;
                    return c;
                }

                void merge(uint16_t c0, uint16_t c1)
                {
                    uint16_t repr_c0 = get_repr(c0);
                    uint16_t repr_c1 = get_repr(c1);
                    if(repr_c0 != repr_c1) parents[repr_c0] = repr_c1;
                    update_cont(repr_c1, conts.contours[repr_c0].bb_br_x, conts.contours[repr_c0].bb_br_y);
                    update_cont(repr_c1, conts.contours[repr_c0].bb_tl_x, conts.contours[repr_c0].bb_tl_y);
                }

                void get_merged_conts(Contour_package &out)
                {
                    for(uint16_t k = 0; k < conts.contour_count; ++k)
                    {
#pragma HLS LOOP_TRIPCOUNT avg=30 max=1023 min=0
#pragma HLS PIPELINE II=2
                        if(parents[k] == 0xFFFF)
                        {
                            uint32_t area = hls::abs((conts.contours[k].bb_tl_x - conts.contours[k].bb_br_x) * (conts.contours[k].bb_tl_y - conts.contours[k].bb_br_y));
                            if(area >= motdet_min_cont_area)
                            {
                                uint16_t new_cont = out.contour_count++;
                                out.contours[new_cont] = conts.contours[k];
                            }
                        }
                    }
                }

            private:
                Contour_package conts;
                uint16_t parents[motdet_max_contours];
            };

        }

        void connected_components(hls::stream<uint8_t, MOTDET_STREAM_DEPTH> &in, Contour_package &contours)
        {
            // Find the contours in the image, we are using a one pass algorithm so in some cases it is not possible to know if 2 pixels belong to the same object.
            // This why we tag them as different contours and then "merge" them when a pixel that connects both is discovered.
            // In essence, we have a graph of blobs with a set of connections (mergers), so we can use well known graph theory algorithms here.
            // Use a disjoint-set data structures, based on a list of trees with representative nodes.

        #ifndef __SYNTHESIS__
            uint16_t *buffer = new uint16_t[motdet_width];
            Disjoint_contour_connector &dcc = *(new Disjoint_contour_connector());
        #else
            uint16_t buffer[motdet_width];
            Disjoint_contour_connector dcc;
        #endif

            // 0xFFFF will be the tag representing a null tag.
            for(uint16_t k = 0; k < motdet_width; ++k) buffer[k] = 0xFFFF;

            uint16_t prev = 0xFFFF, top_prev, tag;


            for(uint16_t i = 0; i < motdet_height; ++i)
            {
                for(uint16_t j = 0; j < motdet_width; ++j)
                {
#pragma HLS PIPELINE
                    tag = 0xFFFF;

                    uint8_t read_val = in.read();
                    if(read_val)
                    {
						if(prev != 0xFFFF){
							// Previous pix to the left exists
							tag = prev;

							top_prev = buffer[j+1 < motdet_width ? j+1 : j];
							if(top_prev != 0xFFFF)
							{
								// And top pix also exists, check for merge.
								if(top_prev != tag) dcc.merge(top_prev, tag);
							}
						}
						else
						{
							top_prev = buffer[j+1 < motdet_width ? j+1 : j];
							if(top_prev != 0xFFFF)
							{
								// Only the top pix exists.
								tag = top_prev;
							}
							else
							{
								// Neither pix exists, this is a new contour.
								tag = dcc.add_cont(i, j);
							}
                        }
                    }

                    prev = tag;
                    buffer[j] = tag;
                    if(tag != 0xFFFF) dcc.update_cont(tag, i, j);
                }
            }

            dcc.get_merged_conts(contours);

        #ifndef __SYNTHESIS__
            delete[] buffer;
            delete &dcc;
        #endif
        }

    } // namespace imgutil
} // namespace motdet
