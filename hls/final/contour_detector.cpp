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

                uint16_t add_cont(ap_uint<9> point_x, ap_uint<9> point_y)
                {
                    if(conts.contour_count >= MOTDET_MAX_CONTOURS) return 0x3FF;

                    ap_uint<10> new_tag = conts.contour_count++;
                    conts.contours[new_tag].bb_br_x = point_x;
                    conts.contours[new_tag].bb_tl_x = point_x;
                    conts.contours[new_tag].bb_br_y = point_y;
                    conts.contours[new_tag].bb_tl_y = point_y;
                    parents[new_tag] = 0x3FF;
                    return new_tag;
                }

                void update_cont(ap_uint<10> tag, ap_uint<9> point_x, ap_uint<9> point_y)
                {
                	ap_uint<10> repr = get_repr(tag);
                    if(point_x < conts.contours[repr].bb_tl_x) conts.contours[repr].bb_tl_x = point_x;
                    else if(point_x > conts.contours[repr].bb_br_x) conts.contours[repr].bb_br_x = point_x;
                    if(point_y < conts.contours[repr].bb_tl_y) conts.contours[repr].bb_tl_y = point_y;
                    else if(point_y > conts.contours[repr].bb_br_y) conts.contours[repr].bb_br_y = point_y;
                }

                uint16_t get_repr(ap_uint<10> c)
                {
                	ap_uint<10> og_c = c;
                	ap_uint<10> parent = parents[c];
                    while(parent != 0x3FF)
                    {
#pragma HLS LOOP_TRIPCOUNT avg=1 max=2 min=0
                        c = parents[c];
                        parent = parents[c];
                    }
                    if(og_c != c) parents[og_c] = c;
                    return c;
                }

                void merge(ap_uint<10> c0, ap_uint<10> c1)
                {
                	ap_uint<10> repr_c0 = get_repr(c0);
                	ap_uint<10> repr_c1 = get_repr(c1);
                    if(repr_c0 != repr_c1) parents[repr_c0] = repr_c1;
                    update_cont(repr_c1, conts.contours[repr_c0].bb_br_x, conts.contours[repr_c0].bb_br_y);
                    update_cont(repr_c1, conts.contours[repr_c0].bb_tl_x, conts.contours[repr_c0].bb_tl_y);
                }

                void get_merged_conts(hls::stream<Streamed_contour, MOTDET_STREAM_DEPTH> &out)
                {
                	Streamed_contour streamed_cont;

                    for(ap_uint<10> k = 0; k < conts.contour_count; ++k)
                    {
#pragma HLS LOOP_TRIPCOUNT avg=30 max=1023 min=0
#pragma HLS PIPELINE II=2
                        if(parents[k] == 0x3FF)
                        {
                        	ap_uint<21> area = hls::abs((conts.contours[k].bb_tl_x - conts.contours[k].bb_br_x) * (conts.contours[k].bb_tl_y - conts.contours[k].bb_br_y));
                            if(area >= motdet_min_cont_area)
                            {
                            	streamed_cont.contour = conts.contours[k];
                            	streamed_cont.contour.bb_tl_x *= MOTDET_REDUCTION_FACTOR;
								streamed_cont.contour.bb_tl_y *= MOTDET_REDUCTION_FACTOR;
								streamed_cont.contour.bb_br_x *= MOTDET_REDUCTION_FACTOR;
								streamed_cont.contour.bb_br_y *= MOTDET_REDUCTION_FACTOR;
                            	streamed_cont.stream_end = false;
                            	out.write(streamed_cont);
                            }
                        }
                    }
                    streamed_cont.stream_end = true;
                    out.write(streamed_cont);
                }

            private:
                Contour_package conts;
                ap_uint<10> parents[MOTDET_MAX_CONTOURS];
            };

        }

        void connected_components(hls::stream<ap_uint<1>, MOTDET_STREAM_DEPTH> &in, hls::stream<motdet::Streamed_contour, MOTDET_STREAM_DEPTH> &out)
        {

		// Find the contours in the image, we are using a one pass algorithm so in some cases it is not possible to know if 2 pixels belong to the same object.
		// This why we tag them as different contours and then "merge" them when a pixel that connects both is discovered.
		// In essence, we have a graph of blobs with a set of connections (mergers), so we can use well known graph theory algorithms here.
		// Use a disjoint-set data structures, based on a list of trees with representative nodes.

        #ifndef __SYNTHESIS__
        	ap_uint<10> *buffer = new ap_uint<10>[MOTDET_WIDTH];
            Disjoint_contour_connector &dcc = *(new Disjoint_contour_connector());
        #else
            ap_uint<10> buffer[MOTDET_WIDTH];
            Disjoint_contour_connector dcc;
        #endif

            // 0x3FF will be the tag representing a null tag.
            for(ap_uint<9> k = 0; k < MOTDET_WIDTH; ++k) buffer[k] = 0x3FF;

            ap_uint<10> prev = 0x3FF, top_prev, tag;


            for(ap_uint<9> i = 0; i < MOTDET_HEIGHT; ++i)
            {
                for(ap_uint<9> j = 0; j < MOTDET_WIDTH; ++j)
                {
#pragma HLS PIPELINE
                    tag = 0x3FF;

                    ap_uint<1> read_val = in.read();
                    if(read_val)
                    {
						if(prev != 0x3FF){
							// Previous pix to the left exists
							tag = prev;

							top_prev = buffer[j];
							if(top_prev != 0x3FF)
							{
								// And top pix also exists, check for merge.
								if(top_prev != tag) dcc.merge(top_prev, tag);
							}
						}
						else
						{
							top_prev = buffer[j];
							if(top_prev != 0x3FF)
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
                    if(tag != 0x3FF) dcc.update_cont(tag, j, i);
                }
            }

            dcc.get_merged_conts(out);

        #ifndef __SYNTHESIS__
            delete[] buffer;
            delete &dcc;
        #endif
        }

    } // namespace imgutil
} // namespace motdet
