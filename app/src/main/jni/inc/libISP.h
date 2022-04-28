#ifndef __LIB_GINGYTECH_ISP_RELEASE__
#define __LIB_GINGYTECH_ISP_RELEASE__
#ifdef __cplusplus
extern "C" {
#endif
#define ORG_IMG_SIZE 80000 // 200 * 200 * 2(2 bytes)
#define OUT_IMG_SIZE 40000 // 200 * 200
#define ISP_BUFFER_SIZE 160028

////For Gingytech ISP1////
typedef enum {
	CF_OK                      = 0,
	CF_FINGERPRINT_TOO_SIMILAR = 104
} GINGYTECH_CHECK_FINGER_RETURN_CODE;

typedef struct tagIspInitInfo {
	//// Input
	int basicUpdateNum;
	//// Input/Output
	unsigned char *updateBuffer;
	//// Input
	int nImageWidth;
	//// Input
	int nImageHeight;
} IspInitInfo;

typedef struct tagIsp1Info {
	//// Input
	unsigned char *input;
	unsigned char *background;
	//// Output
	unsigned char *output;
	int *reserve;
	int *updateResult;
} Isp1Info;

typedef struct tagIsp2Info {
	//// Input
	unsigned char *input;
	//// Output
	unsigned char *output;
	//// Input/Output
	unsigned char *updateBuffer;
	//// reserve
	unsigned char *reserve1;
	//// reserve
	unsigned char *reserve2;
	//// Input
	int updateFlag;
} Isp2Info;

/**
 * @brief: Get libISP Version info.
 *
 * @param ispVersion: Isp version info, char array size: 50.
 */
void Gingytech_GetIspVersion(char *ispVersion);

/**
 * @brief: Initialize ISP info and ISP update buffer.
 *         MUST CALL THIS FUNCTION BEFORE YOU CALL ISP1/ISP2 !
 *
 * @param pIspInitInfo:
 * [
 * basicUpdateNum: Basic update number before using ISP. Can be 5-30. Recommend matching the enroll amount.
 * updateBuffer: updateBuffer used by ISP2. Buffer size should be ISP_BUFFER_SIZE.
 *               If NULL, only initialize ISP info.
 * nImageWidth: User-specified width. MUST be same as input image width.
 * nImageHeight: User-specified height. MUST be same as input image height.
 * ]
 */
void Gingytech_InitISP(IspInitInfo *pIspInitInfo);

/**
 * @brief: ISP1.
 *
 * @param pIsp1Info:
 * [
 * input: Input image data. Data size should be ORG_IMG_SIZE.
 * background: Background image data. Data size should be ORG_IMG_SIZE.
 * output: Output image data, is also ISP2 input data. Data size should be OUT_IMG_SIZE!!!
 * reserve: Unused parameter.
 * updateResult: Decide ISP2 updateFlag.
 *               0 for CF_OK, ISP2 updateFlag set to 1.
 *               Negative value follow -(GINGYTECH_CHECK_FINGER_RETURN_CODE), ISP2 updateFlag set to 0.
 * ]
 */
void Gingytech_ISP1(Isp1Info *pIsp1Info);

/**
 * @brief: ISP2.
 *
 * @param pIsp2Info:
 * [
 * input: Input image data, is also ISP1 output data. Data size should be OUT_IMG_SIZE.
 * output: Output image data. Data size should be OUT_IMG_SIZE.
 * updateBuffer: Update buffer. Buffer size should be ISP_BUFFER_SIZE.
 * reserve1: Unused parameter.
 * reserve2: Unused parameter.
 * updateFlag: Input 1 for update, 0 for not update. Decide from ISP1 updateResult.
 * ]
 */
void Gingytech_ISP2(Isp2Info *pIsp2Info);

/**
 * @brief: Make background image. Keep calling this function until return 0.
 *
 * @param input: Original image data. Data size should be ORG_IMG_SIZE.
 * @param nWidth: Original image width. Should be 200.
 * @param nHeight: Original image height. Should be 200.
 * @param bIsTwoByte: True for 2-bytes image, false for 1-bytes image. Should be true.
 *
 * @return int
 * remaining count of BG make.
 * 0 for finish.
 */
int ImgBgMake(unsigned char* input, int nWidth, int nHeight, bool bIsTwoByte);

/**
 * @brief: Get background image. Call after ImgBgMake() return 0.
 *
 * @param outdata: Output background image data. Data size should be ORG_IMG_SIZE.
 * @param nWidth: Background image width. Should be 200.
 * @param nHeight: Background image height. Should be 200.
 * @param bIsTwoByte: True for 2-bytes image, false for 1-bytes image. Should be true.
 *
 * @return int
 * 0 for success.
 * -1 for fail.
 */
int ImgBgGet(unsigned char* outdata, int nWidth, int nHeight, bool bIsTwoByte);

/**
 * @brief: Free MakeBG resource. Call when MakeBG cancel or finish.
 */
void ImgBgFinish();
#ifdef __cplusplus
}
#endif
#endif