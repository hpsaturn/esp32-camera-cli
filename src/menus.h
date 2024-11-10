#include <Arduino.h>

#define MFRAME_SIZE_LIST \
   X(MFSIZE_96X96,   "96X96    \t96x96    ", FRAMESIZE_96X96  ) \
   X(MFSIZE_QQVGA,   "QQVGA    \t160x120  ", FRAMESIZE_QQVGA  ) \
   X(MFSIZE_QCIF,    "QCIF     \t176x144  ", FRAMESIZE_QCIF   ) \
   X(MFSIZE_HQVGA,   "HQVGA    \t240x176  ", FRAMESIZE_HQVGA  ) \
   X(MFSIZE_240X240, "240X240  \t240x240  ", FRAMESIZE_240X240) \
   X(MFSIZE_QVGA,    "QVGA     \t320x240  ", FRAMESIZE_QVGA   ) \
   X(MFSIZE_CIF,     "CIF      \t400x296  ", FRAMESIZE_CIF    ) \
   X(MFSIZE_HVGA,    "HVGA     \t480x320  ", FRAMESIZE_HVGA   ) \
   X(MFSIZE_VGA,     "VGA      \t640x480  ", FRAMESIZE_VGA    ) \
   X(MFSIZE_SVGA,    "SVGA     \t800x600  ", FRAMESIZE_SVGA   ) \
   X(MFSIZE_XGA,     "XGA      \t1024x768 ", FRAMESIZE_XGA    ) \
   X(MFSIZE_HD,      "HD       \t1280x720 ", FRAMESIZE_HD     ) \
   X(MFSIZE_SXGA,    "SXGA     \t1280x1024", FRAMESIZE_SXGA   ) \
   X(MFSIZE_UXGA,    "UXGA     \t1600x1200", FRAMESIZE_UXGA   ) \
   X(MFSIZE_FHD,     "FHD      \t1920x1080", FRAMESIZE_FHD    ) \
   X(MFSIZE_P_HD,    "P_HD     \t720x1280 ", FRAMESIZE_P_HD   ) \
   X(MFSIZE_P_3MP,   "P_3MP    \t864x1536 ", FRAMESIZE_P_3MP  ) \
   X(MFSIZE_QXGA,    "QXGA     \t2048x1536", FRAMESIZE_QXGA   ) \
   X(MFSIZE_QHD,     "QHD      \t2560x1440", FRAMESIZE_QHD    ) \
   X(MFSIZE_WQXGA,   "WQXGA    \t2560x1600", FRAMESIZE_WQXGA  ) \
   X(MFSIZE_P_FHD,   "P_FHD    \t1080x1920", FRAMESIZE_P_FHD  ) \
   X(MFSIZE_QSXGA,   "QSXGA    \t2560x1920", FRAMESIZE_QSXGA  ) \
   X(MFSIZE_INVALID, "INVALID"             , FRAMESIZE_INVALID)

#define X(mfsname, mfsdesc, mfs) mfsname,
typedef enum MFSKEYS : size_t { MFRAME_SIZE_LIST } MFRAME_SIZE; 
#undef X

#define X(kfsname, kfsdesc, kfs) kfsdesc, 
char const *fskeys[] = { MFRAME_SIZE_LIST };
#undef X
























