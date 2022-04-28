#ifndef _LIB_GINGY_IMG_H
#define _LIB_GINGY_IMG_H

#ifdef _cplusplus
extern "C"{
#endif
void CalUniformity(unsigned char* imgBuf, int imgW, int imgH, int Cutting_Number_x, int Cutting_Number_y, float* uniformity_mean, float* uniformity_std);
void GetVersion(char *version_buf);

#ifdef _cplusplus
}
#endif
#endif
