
/*!
 *****************************************************************************
 *
 * \file fmo.c
 *
 * \brief
 *    Support for Flexible Macroblock Ordering (FMO)
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Stephan Wenger      stewe@cs.tu-berlin.de
 *    - Karsten Suehring
 ******************************************************************************
 */
#include "ifunctions.h"
#include "common.h"
#include "h264_stream.h"
#include "define.h"
#include "fmo.h"

//#define PRINT_FMO_MAPS

static void FmoGenerateType0MapUnitMap (h264_stream_t *h, unsigned PicSizeInMapUnits );
static void FmoGenerateType1MapUnitMap (h264_stream_t *h, unsigned PicSizeInMapUnits );
static void FmoGenerateType2MapUnitMap (h264_stream_t *h, unsigned PicSizeInMapUnits );
static void FmoGenerateType3MapUnitMap (h264_stream_t *h, unsigned PicSizeInMapUnits );
static void FmoGenerateType4MapUnitMap (h264_stream_t *h, unsigned PicSizeInMapUnits );
static void FmoGenerateType5MapUnitMap (h264_stream_t *h, unsigned PicSizeInMapUnits );
static void FmoGenerateType6MapUnitMap (h264_stream_t *h, unsigned PicSizeInMapUnits );


/*!
 ************************************************************************
 * \brief
 *    Generates h->MapUnitToSliceGroupMap
 *    Has to be called every time a new Picture Parameter Set is used
 *
 * \param h
 *      video encoding parameters for current picture
 *
 ************************************************************************
 */
//8.2.2 宏块到条带组的映射的解码过程
static int FmoGenerateMapUnitToSliceGroupMap (h264_stream_t* h)
{
  sps_t* sps = h->sps;
  pps_t* pps = h->pps;
  slice_header_t* sh = h->sh;

  unsigned int NumSliceGroupMapUnits;

  NumSliceGroupMapUnits = (sps->pic_height_in_map_units_minus1+1)* (sps->pic_width_in_mbs_minus1+1);

  if (pps->slice_group_map_type == 6)
  {
    if ((pps->pic_size_in_map_units_minus1 + 1) != NumSliceGroupMapUnits)
    {
      //error("wrong pps->pic_size_in_map_units_minus1 for used SPS and FMO type 6", 500);
      error("fun: FmoGenerateMapUnitToSliceGroupMap",500);
    }
  }

  // allocate memory for h->MapUnitToSliceGroupMap
  if (h->MapUnitToSliceGroupMap)
    free(h->MapUnitToSliceGroupMap);
  if ((h->MapUnitToSliceGroupMap = (int *)malloc ((NumSliceGroupMapUnits) * sizeof (int))) == NULL)
  {
    printf ("cannot allocated %d bytes for h->MapUnitToSliceGroupMap, exit\n", (int) ( (pps->pic_size_in_map_units_minus1+1) * sizeof (int)));
    exit (-1);
  }

  if (pps->num_slice_groups_minus1 == 0)    // only one slice group
  {
    memset(h->MapUnitToSliceGroupMap, 0, NumSliceGroupMapUnits * sizeof (int));
    return 0;
  }

  switch (pps->slice_group_map_type)
  {
  case 0:
    FmoGenerateType0MapUnitMap (h, NumSliceGroupMapUnits);
    break;
  case 1:
    FmoGenerateType1MapUnitMap (h, NumSliceGroupMapUnits);
    break;
  case 2:
    FmoGenerateType2MapUnitMap (h, NumSliceGroupMapUnits);
    break;
  case 3:
    FmoGenerateType3MapUnitMap (h, NumSliceGroupMapUnits);
    break;
  case 4:
    FmoGenerateType4MapUnitMap (h, NumSliceGroupMapUnits);
    break;
  case 5:
    FmoGenerateType5MapUnitMap (h, NumSliceGroupMapUnits);
    break;
  case 6:
    FmoGenerateType6MapUnitMap (h, NumSliceGroupMapUnits);
    break;
  default:
    printf ("Illegal slice_group_map_type %d , exit \n", (int) pps->slice_group_map_type);
    exit (-1);
  }
  return 0;
}


/*!
 ************************************************************************
 * \brief
 *    Generates h->MbToSliceGroupMap from h->MapUnitToSliceGroupMap
 *
 * \param h
 *      video encoding parameters for current picture
 *
 ************************************************************************
 */
static int FmoGenerateMbToSliceGroupMap(h264_stream_t* h)
{
  sps_t* sps = h->sps;
  slice_header_t* sh = h->sh;

  unsigned i;

  // allocate memory for h->MbToSliceGroupMap
  if (h->MbToSliceGroupMap)
    free(h->MbToSliceGroupMap);

  if ((h->MbToSliceGroupMap = (int*)malloc ((h->PicSizeInMbs) * sizeof (int))) == NULL)
  {
    printf ("cannot allocate %d bytes for h->MbToSliceGroupMap, exit\n", (int)((h->PicSizeInMbs) * sizeof (int)));
    exit (-1);
  }

  //FmoGenerateMapUnitToSliceGroupMap(h);

  if ((sps->frame_mbs_only_flag)|| sh->field_pic_flag)
  {
    int *MbToSliceGroupMap = h->MbToSliceGroupMap;
    int *MapUnitToSliceGroupMap = h->MapUnitToSliceGroupMap;
    for (i=0; i<h->PicSizeInMbs; i++)
    {
      *MbToSliceGroupMap++ = *MapUnitToSliceGroupMap++;
    }
  }
  else
  {
    if (sps->mb_adaptive_frame_field_flag  &&  (!sh->field_pic_flag))
    {
      for (i=0; i<h->PicSizeInMbs; i++)
      {
        h->MbToSliceGroupMap[i] = h->MapUnitToSliceGroupMap[i/2];
      }
    }
    else
    {
      for (i=0; i<h->PicSizeInMbs; i++)
      {
        h->MbToSliceGroupMap[i] = h->MapUnitToSliceGroupMap[(i/(2*h->PicWidthInMbs))*h->PicWidthInMbs+(i%h->PicWidthInMbs)];
      }
    }
  }
  
  return 0;
}


/*!
 ************************************************************************
 * \brief
 *    FMO initialization: Generates h->MapUnitToSliceGroupMap and h->MbToSliceGroupMap.
 *
 * \param h
 *      video encoding parameters for current picture
 ************************************************************************
 */
int Fmo_init(h264_stream_t* h)
{
  pps_t* pps = h->pps;
  slice_header_t* sh = h->sh;

  FmoGenerateMapUnitToSliceGroupMap(h);
  FmoGenerateMbToSliceGroupMap(h);

  h->NumberOfSliceGroups = pps->num_slice_groups_minus1 + 1;

#ifdef PRINT_FMO_MAPS
  printf("\n");
  printf("FMO Map MapUnitToSliceGroupMap (Units):\n");

  unsigned i,j;

  for (j=0; j<h->PicHeightInMapUnits; j++)
  {
    for (i=0; i<h->PicWidthInMbs; i++)
    {
      printf("%c",48+h->MapUnitToSliceGroupMap[i+j*h->PicWidthInMbs]);
    }
    printf("\n");
  }
  printf("\n");
  printf("FMO Map MbToSliceGroupMap (Mb):\n");

  for (j=0; j<h->PicHeightInMbs; j++)
  {
    for (i=0; i<h->PicWidthInMbs; i++)
    {
      printf("%c",48 + h->MbToSliceGroupMap[i + j * h->PicWidthInMbs]);
    }
    printf("\n");
  }
  printf("\n");

#endif

  return 0;
}


/*!
 ************************************************************************
 * \brief
 *    Free memory allocated by FMO functions
 ************************************************************************
 */
int Fmo_Deinit(h264_stream_t *h)
{
  if (h->MbToSliceGroupMap)
  {
    free (h->MbToSliceGroupMap);
    h->MbToSliceGroupMap = NULL;
  }
  if (h->MapUnitToSliceGroupMap)
  {
    free (h->MapUnitToSliceGroupMap);
    h->MapUnitToSliceGroupMap = NULL;
  }
  return 0;
}


/*!
 ************************************************************************
 * \brief
 *    FmoGetNumberOfSliceGroup(h)
 *
 * \par h:
 *    h264_stream_t
 ************************************************************************
 */
int FmoGetNumberOfSliceGroup(h264_stream_t *h)
{
  return h->NumberOfSliceGroups;
}


/*!
 ************************************************************************
 * \brief
 *    FmoGetLastMBOfPicture(h)
 *    returns the macroblock number of the last MB in a picture.  This
 *    mb happens to be the last macroblock of the picture if there is only
 *    one slice group
 *
 * \par Input:
 *    None
 ************************************************************************
 */
int FmoGetLastMBOfPicture(h264_stream_t *h)
{
  return FmoGetLastMBInSliceGroup (h, FmoGetNumberOfSliceGroup(h)-1);
}


/*!
 ************************************************************************
 * \brief
 *    FmoGetLastMBInSliceGroup: Returns MB number of last MB in SG
 *
 * \par Input:
 *    SliceGroupID (0 to 7)
 ************************************************************************
 */

int FmoGetLastMBInSliceGroup (h264_stream_t *h, int SliceGroup)
{
  int i;

  for (i=h->PicSizeInMbs-1; i>=0; i--)
    if (FmoGetSliceGroupId (h, i) == SliceGroup)
      return i;
  return -1;

}


/*!
 ************************************************************************
 * \brief
 *    Returns SliceGroupID for a given MB
 *
 * \param h
 *      video encoding parameters for current picture
 * \param mb
 *    Macroblock number (in scan order)
 ************************************************************************
 */
int FmoGetSliceGroupId (h264_stream_t* h, int mb)
{
  assert (mb < (int) h->PicSizeInMbs);
  assert (h->MbToSliceGroupMap != NULL);
  
  return h->MbToSliceGroupMap[mb];
}


/*!
 ************************************************************************
 * \brief
 *    FmoGetNextMBBr: Returns the MB-Nr (in scan order) of the next
 *    MB in the (scattered) Slice, -1 if the slice is finished
 * \param h
 *      video encoding parameters for current picture
 *
 * \param CurrentMbNr
 *    number of the current macroblock
 ************************************************************************
 */
int FmoGetNextMBNr (h264_stream_t* h, int CurrentMbNr)
{			
  	int SliceGroup = FmoGetSliceGroupId (h, CurrentMbNr);
	
  	while (++CurrentMbNr < h->PicSizeInMbs && h->MbToSliceGroupMap[CurrentMbNr] != SliceGroup)
    	;

	if (CurrentMbNr >= h->PicSizeInMbs)
		return -1;    // No further MB in this slice (could be end of picture)
	else
		return CurrentMbNr;
}


/*!
 ************************************************************************
 * \brief
 *    Generate interleaved slice group map type MapUnit map (type 0)
 *
 ************************************************************************
 */
static void FmoGenerateType0MapUnitMap (h264_stream_t* h, unsigned PicSizeInMapUnits )
{
  pps_t* pps = h->pps;
  unsigned iGroup, j;
  unsigned i = 0;
  do
  {
    for( iGroup = 0;
         (iGroup <= pps->num_slice_groups_minus1) && (i < PicSizeInMapUnits);
         i += pps->run_length_minus1[iGroup++] + 1 )
    {
      for( j = 0; j <= pps->run_length_minus1[ iGroup ] && i + j < PicSizeInMapUnits; j++ )
        h->MapUnitToSliceGroupMap[i+j] = iGroup;
    }
  }
  while( i < PicSizeInMapUnits );
}


/*!
 ************************************************************************
 * \brief
 *    Generate dispersed slice group map type MapUnit map (type 1)
 *
 ************************************************************************
 */
static void FmoGenerateType1MapUnitMap (h264_stream_t* h, unsigned PicSizeInMapUnits )
{
  pps_t* pps = h->pps;
  unsigned i;
  for( i = 0; i < PicSizeInMapUnits; i++ )
  {
    h->MapUnitToSliceGroupMap[i] = ((i%h->PicWidthInMbs)+(((i/h->PicWidthInMbs)*(pps->num_slice_groups_minus1+1))/2))
                                %(pps->num_slice_groups_minus1+1);
  }
}

/*!
 ************************************************************************
 * \brief
 *    Generate foreground with left-over slice group map type MapUnit map (type 2)
 *
 ************************************************************************
 */
static void FmoGenerateType2MapUnitMap (h264_stream_t* h, unsigned PicSizeInMapUnits )
{
  pps_t* pps = h->pps;
  int iGroup;
  unsigned i, x, y;
  unsigned yTopLeft, xTopLeft, yBottomRight, xBottomRight;

  for( i = 0; i < PicSizeInMapUnits; i++ )
    h->MapUnitToSliceGroupMap[ i ] = pps->num_slice_groups_minus1;

  for( iGroup = pps->num_slice_groups_minus1 - 1 ; iGroup >= 0; iGroup-- )
  {
    yTopLeft = pps->top_left[ iGroup ] / h->PicWidthInMbs;
    xTopLeft = pps->top_left[ iGroup ] % h->PicWidthInMbs;
    yBottomRight = pps->bottom_right[ iGroup ] / h->PicWidthInMbs;
    xBottomRight = pps->bottom_right[ iGroup ] % h->PicWidthInMbs;
    for( y = yTopLeft; y <= yBottomRight; y++ )
      for( x = xTopLeft; x <= xBottomRight; x++ )
        h->MapUnitToSliceGroupMap[ y * h->PicWidthInMbs + x ] = iGroup;
 }
}


/*!
 ************************************************************************
 * \brief
 *    Generate box-out slice group map type MapUnit map (type 3)
 *
 ************************************************************************
 */
static void FmoGenerateType3MapUnitMap (h264_stream_t* h, unsigned PicSizeInMapUnits)
{
  pps_t* pps = h->pps;
  slice_header_t* sh = h->sh;
  
  unsigned i, k;
  int leftBound, topBound, rightBound, bottomBound;
  int x, y, xDir, yDir;
  int mapUnitVacant;

  unsigned mapUnitsInSliceGroup0 = imin((pps->slice_group_change_rate_minus1 + 1) * sh->slice_group_change_cycle, PicSizeInMapUnits);

  for( i = 0; i < PicSizeInMapUnits; i++ )
    h->MapUnitToSliceGroupMap[ i ] = 2;

  x = ( h->PicWidthInMbs - pps->slice_group_change_direction_flag ) / 2;
  y = ( h->PicHeightInMapUnits - pps->slice_group_change_direction_flag ) / 2;

  leftBound   = x;
  topBound    = y;
  rightBound  = x;
  bottomBound = y;

  xDir =  pps->slice_group_change_direction_flag - 1;
  yDir =  pps->slice_group_change_direction_flag;

  for( k = 0; k < PicSizeInMapUnits; k += mapUnitVacant )
  {
    mapUnitVacant = ( h->MapUnitToSliceGroupMap[ y * h->PicWidthInMbs + x ]  ==  2 );
    if( mapUnitVacant )
       h->MapUnitToSliceGroupMap[ y * h->PicWidthInMbs + x ] = ( k >= mapUnitsInSliceGroup0 );

    if( xDir  ==  -1  &&  x  ==  leftBound )
    {
      leftBound = imax( leftBound - 1, 0 );
      x = leftBound;
      xDir = 0;
      yDir = 2 * pps->slice_group_change_direction_flag - 1;
    }
    else
      if( xDir  ==  1  &&  x  ==  rightBound )
      {
        rightBound = imin( rightBound + 1, (int)h->PicWidthInMbs - 1 );
        x = rightBound;
        xDir = 0;
        yDir = 1 - 2 * pps->slice_group_change_direction_flag;
      }
      else
        if( yDir  ==  -1  &&  y  ==  topBound )
        {
          topBound = imax( topBound - 1, 0 );
          y = topBound;
          xDir = 1 - 2 * pps->slice_group_change_direction_flag;
          yDir = 0;
         }
        else
          if( yDir  ==  1  &&  y  ==  bottomBound )
          {
            bottomBound = imin( bottomBound + 1, (int)h->PicHeightInMapUnits - 1 );
            y = bottomBound;
            xDir = 2 * pps->slice_group_change_direction_flag - 1;
            yDir = 0;
          }
          else
          {
            x = x + xDir;
            y = y + yDir;
          }
  }

}

/*!
 ************************************************************************
 * \brief
 *    Generate raster scan slice group map type MapUnit map (type 4)
 *
 ************************************************************************
 */
static void FmoGenerateType4MapUnitMap (h264_stream_t* h, unsigned PicSizeInMapUnits)
{
  pps_t* pps = h->pps;
  slice_header_t* sh = h->sh;

  unsigned mapUnitsInSliceGroup0 = imin((pps->slice_group_change_rate_minus1 + 1) * sh->slice_group_change_cycle, PicSizeInMapUnits);
  unsigned sizeOfUpperLeftGroup = pps->slice_group_change_direction_flag ? ( PicSizeInMapUnits - mapUnitsInSliceGroup0 ) : mapUnitsInSliceGroup0;

  unsigned i;

  for( i = 0; i < PicSizeInMapUnits; i++ )
    if( i < sizeOfUpperLeftGroup )
        h->MapUnitToSliceGroupMap[ i ] = pps->slice_group_change_direction_flag;
    else
        h->MapUnitToSliceGroupMap[ i ] = 1 - pps->slice_group_change_direction_flag;

}

/*!
 ************************************************************************
 * \brief
 *    Generate wipe slice group map type MapUnit map (type 5)
 *
 ************************************************************************
 */
static void FmoGenerateType5MapUnitMap (h264_stream_t* h, unsigned PicSizeInMapUnits)
{
  pps_t* pps = h->pps;
  slice_header_t* sh = h->sh;

  unsigned mapUnitsInSliceGroup0 = imin((pps->slice_group_change_rate_minus1 + 1) * sh->slice_group_change_cycle, PicSizeInMapUnits);
  unsigned sizeOfUpperLeftGroup = pps->slice_group_change_direction_flag ? ( PicSizeInMapUnits - mapUnitsInSliceGroup0 ) : mapUnitsInSliceGroup0;

  unsigned i,j, k = 0;

  for( j = 0; j < h->PicWidthInMbs; j++ )
    for( i = 0; i < h->PicHeightInMapUnits; i++ )
        if( k++ < sizeOfUpperLeftGroup )
            h->MapUnitToSliceGroupMap[ i * h->PicWidthInMbs + j ] = pps->slice_group_change_direction_flag;
        else
            h->MapUnitToSliceGroupMap[ i * h->PicWidthInMbs + j ] = 1 - pps->slice_group_change_direction_flag;

}

/*!
 ************************************************************************
 * \brief
 *    Generate explicit slice group map type MapUnit map (type 6)
 *
 ************************************************************************
 */
static void FmoGenerateType6MapUnitMap (h264_stream_t* h, unsigned PicSizeInMapUnits )
{
  pps_t* pps = h->pps;
  unsigned i;
  for (i=0; i<PicSizeInMapUnits; i++)
  {
    h->MapUnitToSliceGroupMap[i] = pps->slice_group_id[i];
  }
}


