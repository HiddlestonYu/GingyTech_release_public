#ifndef __LIB_GINGY_ALGOR__
#define __LIB_GINGY_ALGOR__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief: Initialize Gingy algorithm. Should be called at the beginning.
 *
 * @param storage_root: Input: Storage path of template files.
 * @param nImgWidth: Input: Image width.
 * @param nImgHeight: Input: Image height.
 *
 * @return int
 *  0: Initialize success.
 * <0: Storage path setting fail. Return errno.
 */
int AlgorInit(char* storage_root, int nImgWidth, int nImgHeight);

/**
 * @brief: Free algorithm resource. Call before terminating process.
 */
void AlgorUninit();

/**
 * @brief: Get Exist Template FID list.
 *
 * @param fidList: Output: Fid list of saved templates.
 *
 * @return int: Saved template numbers.
 */
int LoadSavedTemplatesFID(int* fidList);

/**
 * @brief: Initialize enroll algorithm. Should be called before enrollment and enroll screen.
 *
 * @param enrollNum: Input: Number of enroll samples.
 */
void EnrollInit(int enrollNum);

/**
 * @brief: Check image quality and similarity before Enrollment. It can help caller decide enrolling the image or not.
 *
 * @param pData: Input: Input image data.
 * @param pnQualityScore: Output: Image Quality. Recommended value: over 20.
 * @param pnOverlapAll: Output: Overlap ratio with all enrolled image. Recommended value: less than 95.
 * @param pnOverlapLast: Output: Overlap ratio with last enrolled image. Recommended value: less than 80.
 *
 * @return int
 *  0: enroll screen success.
 * -1: enroll screen fail.
 */
int EnrollScreen(unsigned char* pData, int *pnQualityScore, int *pnOverlapAll, int *pnOverlapLast);

/**
 * @brief: Enrollment. The caller should keep calling this function until return value is 0.
 *
 * @param pData: Input: Input image data.
 *
 * @return int
 * >=0: remaining call times of the function
 *  -1: algorithm fail
 *  -2: input image enroll fail, retry another one.
 *  -3: input null
 *  -4: enrolled template number reach limit. Store 100 templates at most.
 */
int Enroll(unsigned char* pData);

/**
 * @brief: Finalize Enrollment and storage template.
 *
 * @param fid: Output: When enroll finish success, it will return finger id of stored template, start from 1. Otherwise, it returns -1.
 *
 * @return int
 *  0: success
 * -1: algorithm fail
 * -2: enrolled template number reach limit.
 */
int EnrollFinish(int* fid);

/**
 * @brief: Initialize authenticate algorithm. Should be called before authentication.
 */
void AuthInit();

/**
 * @brief: Authentication.
 *
 * @param pData: Input: Input image data.
 *
 * @return int
 * >0: image match. Return finger id of match template
 *  0: image mismatch
 * -1: algorithm fail
 * -2: input null
 * -3: no enrolled template
 */
int Auth(unsigned char *pData);

/**
 * @brief: Remove template.
 *
 * @param fid: Input: finger id of the template which caller want to delete.
 *
 * @return int
 *  0: success
 * -1: no template file with input fid
 * -2: remove fail
 */
int Remove(int fid);

/**
 * @brief: If caller uses AlgorInit() to change input image w/h, the caller needs to call this function to delete mobile file.
 *
 * @return int
 *  0: success
 * -1: mobile file is not found.
 * -2: remove fail
 */
int RemoveMobileFile();

/**
 * @brief: Get Image Quality Score.
 *
 * @param pData: Input: Input image data.
 *
 * @return int: Quality Score.
 */
int GetImageQuality(unsigned char *pData);

/**
 * @brief: Get library version info.
 *
 * @param version: Version info, char array size: 50.
 */
void GetVersion(char* version);

#ifdef __cplusplus
}
#endif
#endif