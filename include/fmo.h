#ifndef _FMO_H_
#define _FMO_H_

#ifdef __cplusplus
extern "C" {
#endif

extern int Fmo_init (h264_stream_t *h);
extern int Fmo_Deinit (h264_stream_t *h);

extern int FmoGetNumberOfSliceGroup(h264_stream_t *h);
extern int FmoGetLastMBOfPicture   (h264_stream_t *h);
extern int FmoGetLastMBInSliceGroup(h264_stream_t *h, int SliceGroup);
extern int FmoGetSliceGroupId      (h264_stream_t *h, int mb);
extern int FmoGetNextMBNr          (h264_stream_t *h, int CurrentMbNr);


#ifdef __cplusplus
}
#endif

#endif
