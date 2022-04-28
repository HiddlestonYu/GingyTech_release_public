#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <asm/ioctl.h>
#include <android/log.h>
#include <math.h>
#include <iostream>
#include <vector>

#include <jni.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libISP.h"
#include "gingyAlgor.h"
#include "libgingyimg.h"

using namespace std;

#define LOG_TAG "GingyIA_Jni"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG , LOG_TAG, __VA_ARGS__)
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG,__VA_ARGS__)

int mnImgWidth    = 0;
int mnImgHeight   = 0;
int mnIspResultW  = 0;
int mnIspResultH  = 0;

void CalcRangeLevel(unsigned short *input, unsigned short *max, unsigned short *min, int srcWidth, int srcHeight, int roiX, int roiY, int roiWidth, int roiHeight) {
	int i, j;
	unsigned short MaxLevel, MinLevel;
	MaxLevel = 0;
	MinLevel = 65535;

	for (i = roiY; i < (roiY + roiHeight); i = i + 1) {
		for (j = roiX; j < (roiX + roiWidth); j = j + 1) {
			if (input[i*srcWidth + j] > MaxLevel) {
				MaxLevel = input[i*srcWidth + j];
			}
			else if (input[i*srcWidth + j] < MinLevel) {
				MinLevel = input[i*srcWidth + j];
			}
		}
	}

	*max = MaxLevel;
	*min = MinLevel;
}

#define BitMode_Full_12Bit 0
#define BitMode_High_8Bit  1
#define BitMode_Low_8Bit   2
void AutoLevelOffset(unsigned short *input, unsigned char *output, int srcWidth, int srcHeight, int bitMode) {
	int i;
	double tmp;
	unsigned short MaxLevel, MinLevel;
	unsigned short *ShortIntBuffer = (unsigned short *)malloc(sizeof(unsigned short) * srcWidth * srcHeight);
	memcpy(ShortIntBuffer, input, srcWidth*srcHeight*sizeof(unsigned short));

	//Calculate from full image
	CalcRangeLevel(ShortIntBuffer, &MaxLevel, &MinLevel, srcWidth, srcHeight, 0, 0, srcWidth, srcHeight);

	switch (bitMode) {
		case BitMode_Full_12Bit:
			for (i = 0; i < (srcHeight*srcWidth); i = i + 1) {
				tmp = (double)(ShortIntBuffer[i] - MinLevel) / (double)(MaxLevel - MinLevel) * 255;

				if (tmp > 255) {
					output[i] = 255;
				} else if (tmp < 0) {
					output[i] = 0;
				} else {
					output[i] = (unsigned char)tmp;
				}
			}
			break;

		case BitMode_High_8Bit:
			for (i = 0; i < (srcHeight*srcWidth); i = i + 1) {
				tmp = (ShortIntBuffer[i] >> 4) & 0xFF;
				output[i] = tmp;
			}
			break;

		case BitMode_Low_8Bit:
			for (i = 0; i < (srcHeight*srcWidth); i = i + 1) {
				tmp = ShortIntBuffer[i] & 0xFF;

				if (ShortIntBuffer[i] > 0xFF) {
					output[i] = 255;
				} else {
					output[i] = (unsigned char)tmp;
				}
			}

			break;
	}

	free(ShortIntBuffer);
}

bool Convert2ByteTo1Byte(unsigned char* pData, int* pnDataSize, int nDataBits, int srcWidth, int srcHeight){
	if(nDataBits == 8)
		return true;

	unsigned short *pusBuf = (unsigned short *)malloc(sizeof(unsigned short)*((*pnDataSize) / 2));
	for(int nIdx=0; nIdx<((*pnDataSize) / 2); nIdx++){
		pusBuf[nIdx] = (unsigned short) ( ((pData[nIdx * 2]&0xff) | (((pData[nIdx * 2 + 1]&0xff) << 8) & 0xFFFF) ));
	}
	AutoLevelOffset(pusBuf, pData, srcWidth, srcHeight, BitMode_Full_12Bit);
	return true;
}

extern "C" JNIEXPORT void JNICALL Java_com_gingytech_imageanalysis_FingerprintSensor_native_1ispInit
		(JNIEnv * env, jobject thiz, jint jnBasicUpdateNum, jbyteArray jabUpdateBuffer, jint jnImgWidth, jint jnImgHeight){
	mnImgWidth   = jnImgWidth;
	mnImgHeight  = jnImgHeight;
	mnIspResultW = jnImgWidth;
	mnIspResultH = jnImgHeight;

	jbyte* pjbyDB = NULL;
	int nDbSize = 0;
	if(jabUpdateBuffer != NULL){
		nDbSize = (int)env->GetArrayLength(jabUpdateBuffer);
		LOGD("IspInit DbSize:%d", nDbSize);
		pjbyDB = env->GetByteArrayElements(jabUpdateBuffer, 0);
	}

	IspInitInfo strIspInitInfo;
	strIspInitInfo.basicUpdateNum = jnBasicUpdateNum;
	strIspInitInfo.updateBuffer = (unsigned char*)pjbyDB;
	strIspInitInfo.nImageWidth = mnImgWidth;
	strIspInitInfo.nImageHeight = mnImgHeight;

	Gingytech_InitISP(&strIspInitInfo);

	if(jabUpdateBuffer != NULL){
		env->SetByteArrayRegion(jabUpdateBuffer, 0, nDbSize, (signed char*)pjbyDB);
		env->ReleaseByteArrayElements(jabUpdateBuffer, pjbyDB, 0);
	}
}

extern "C" JNIEXPORT void JNICALL Java_com_gingytech_imageanalysis_FingerprintSensor_native_1ISP1
		(JNIEnv * env, jobject thiz, jbyteArray jabInput, jbyteArray jabBackground, jint jnDataBits, jintArray janResultWidth, jintArray janResultHeight, jintArray janUpdateResult, jboolean jbDoIsp1){
	int nSize = env->GetArrayLength(jabInput);
	jbyte* pjbyInput = env->GetByteArrayElements(jabInput, 0);

	if(jbDoIsp1 == false){
		if(jnDataBits > 8){
			Convert2ByteTo1Byte((unsigned char*)pjbyInput, &nSize, jnDataBits, mnImgWidth, mnImgHeight);
		}
		env->SetIntArrayRegion(janResultWidth, 0, 1, &mnImgWidth);
		env->SetIntArrayRegion(janResultHeight, 0, 1, &mnImgHeight);
		env->SetByteArrayRegion(jabInput, 0, mnImgWidth * mnImgHeight, (signed char*)pjbyInput);
		env->ReleaseByteArrayElements(jabInput, pjbyInput, 0);
		return;
	}

	jbyte* pjbyBg = env->GetByteArrayElements(jabBackground, 0);
	int nUpdateResult = 0;
	int	nOutSize = mnIspResultW * mnIspResultH;
	unsigned char* pOutBuf = new unsigned char[nOutSize];

	Isp1Info strIsp1Info;
	strIsp1Info.input = (unsigned char*)pjbyInput;
	strIsp1Info.background = (unsigned char*)pjbyBg;
	strIsp1Info.output = pOutBuf;
	strIsp1Info.updateResult = &nUpdateResult;

	Gingytech_ISP1(&strIsp1Info);

	env->ReleaseByteArrayElements(jabInput, pjbyInput, 0);
	env->ReleaseByteArrayElements(jabBackground, pjbyBg, 0);

	env->SetByteArrayRegion(jabInput, 0, nOutSize, (signed char*)pOutBuf);
	env->SetIntArrayRegion(janResultWidth, 0, 1, &mnIspResultW);
	env->SetIntArrayRegion(janResultHeight, 0, 1, &mnIspResultH);
	delete []pOutBuf;

	env->SetIntArrayRegion(janUpdateResult, 0, 1, &nUpdateResult);
}

extern "C" JNIEXPORT void JNICALL Java_com_gingytech_imageanalysis_FingerprintSensor_native_1ISP2
		(JNIEnv * env, jobject thiz, jbyteArray jabInput, jbyteArray jabOutput, jbyteArray jabUpdateBuffer, jint jnUpdateFlag){
	int nSrcSize = (int)env->GetArrayLength(jabInput);
	int nDstSize = (int)env->GetArrayLength(jabOutput);
	int nDbSize  = (int)env->GetArrayLength(jabUpdateBuffer);
	// LOGD("ISP2 BufSize Src:%d Dst:%d DB:%d", nSrcSize, nDstSize, nDbSize);
	jbyte* pjbySrc = env->GetByteArrayElements(jabInput, 0);
	jbyte* pjbyDB = env->GetByteArrayElements(jabUpdateBuffer, 0);
	unsigned char* pResultImg = new unsigned char[nDstSize];

	Isp2Info strIsp2Info;
	strIsp2Info.input = (unsigned char*)pjbySrc;
	strIsp2Info.output = (unsigned char*)pResultImg;
	strIsp2Info.updateBuffer = (unsigned char*)pjbyDB;
	strIsp2Info.updateFlag = (int)jnUpdateFlag;

	Gingytech_ISP2(&strIsp2Info);

	env->SetByteArrayRegion(jabOutput, 0, nDstSize, (signed char*)pResultImg);
	env->SetByteArrayRegion(jabUpdateBuffer, 0, nDbSize, (signed char*)pjbyDB);
	env->ReleaseByteArrayElements(jabInput, pjbySrc, 0);
	env->ReleaseByteArrayElements(jabUpdateBuffer, pjbyDB, 0);
	delete[] pResultImg;
}

extern "C" JNIEXPORT jint JNICALL Java_com_gingytech_imageanalysis_FingerprintSensor_native_1ImgBgMake
		(JNIEnv * env, jobject thiz, jbyteArray jabInput, jint jnWidth, jint jnHeight, jboolean jbIsTwoByte){
	jbyte* pjbyInput = env->GetByteArrayElements(jabInput, 0);

	int nRet = ImgBgMake((unsigned char*)pjbyInput, jnWidth, jnHeight, jbIsTwoByte);
	env->ReleaseByteArrayElements(jabInput, pjbyInput, 0);

	return nRet;
}

extern "C" JNIEXPORT jint JNICALL Java_com_gingytech_imageanalysis_FingerprintSensor_native_1ImgBgGet
		(JNIEnv * env, jobject thiz, jbyteArray jabOutdata, jint jnWidth, jint jnHeight, jboolean jbIsTwoByte){
	int nSize = 0;

	if(jbIsTwoByte == true){
		nSize = jnWidth * jnHeight * 2;
	} else {
		nSize = jnWidth * jnHeight;
	}
	unsigned char* pOutBuf = new unsigned char[nSize];

	int nRet = ImgBgGet(pOutBuf, jnWidth, jnHeight, jbIsTwoByte);
	if(nRet != 0){
		LOGE("Isp ImgBgGet Fail !");
	}

	env->SetByteArrayRegion(jabOutdata, 0, nSize, (signed char*)pOutBuf);
	delete []pOutBuf;

	return nRet;
}

extern "C" JNIEXPORT void JNICALL Java_com_gingytech_imageanalysis_FingerprintSensor_native_1ImgBgFinish
		(JNIEnv * env, jobject thiz){
	ImgBgFinish();
}

extern "C" JNIEXPORT jstring JNICALL Java_com_gingytech_imageanalysis_FingerprintSensor_native_1getIspVer
		(JNIEnv *env, jobject thiz){
	char buf[50];
	Gingytech_GetIspVersion(buf);
	return env->NewStringUTF(buf);
}

extern "C" JNIEXPORT jint JNICALL Java_com_gingytech_imageanalysis_FingerprintSensor_native_1algorInit
(JNIEnv * env, jobject thiz, jstring jstorage_path, jint jnIspResultW, jint jnIspResultH){
    const char *storage_path = env->GetStringUTFChars(jstorage_path, 0);

	int nRet = AlgorInit((char*)storage_path, (int)jnIspResultW, (int)jnIspResultH);
    env->ReleaseStringUTFChars(jstorage_path, storage_path);

    return nRet;
}

extern "C" JNIEXPORT void JNICALL Java_com_gingytech_imageanalysis_FingerprintSensor_native_1algorUninit
		(JNIEnv * env, jobject thiz){
	AlgorUninit();
}


extern "C" JNIEXPORT jint JNICALL Java_com_gingytech_imageanalysis_FingerprintSensor_native_1LoadSavedTemplatesFID
(JNIEnv * env, jobject thiz, jintArray janFidList){
    int* fidList = new int[100]; //目前開放可以註冊100個template

    int nRet = LoadSavedTemplatesFID(fidList);
    env->SetIntArrayRegion(janFidList, 0, nRet, fidList);

	delete[] fidList;
    return nRet;
}

extern "C" JNIEXPORT jint JNICALL Java_com_gingytech_imageanalysis_FingerprintSensor_native_1bioEnrollInit
		(JNIEnv * env, jobject thiz, jint jiEnrollNum){
	EnrollInit(jiEnrollNum);
	return 0;
}

extern "C" JNIEXPORT jint JNICALL Java_com_gingytech_imageanalysis_FingerprintSensor_native_1bioEnrollScreen
		(JNIEnv * env, jobject thiz, jbyteArray jabImgBuf, jintArray janQuality, jintArray janOverlapAll, jintArray janOverlapLast){
	if(!jabImgBuf){
		LOGD("img null ptr");
		return -2;
	}

	jbyte* pjb = env->GetByteArrayElements(jabImgBuf, 0);
	int Q,OA,OL;
	int nRet = EnrollScreen((unsigned char*)pjb, &Q, &OA, &OL);
	env->SetIntArrayRegion(janQuality, 0, 1, &Q);
	env->SetIntArrayRegion(janOverlapAll, 0, 1, &OA);
	env->SetIntArrayRegion(janOverlapLast, 0, 1, &OL);
	env->ReleaseByteArrayElements(jabImgBuf, pjb, 0);
	return nRet;
}

extern "C" JNIEXPORT jint JNICALL Java_com_gingytech_imageanalysis_FingerprintSensor_native_1bioEnroll
		(JNIEnv * env, jobject thiz, jbyteArray jabImgBuf) {
	jbyte* pjb = (jabImgBuf==NULL ? NULL : env->GetByteArrayElements(jabImgBuf, 0));

	jint nRet = Enroll((unsigned char*)pjb);
	if(jabImgBuf!=NULL && pjb!=NULL)
		env->ReleaseByteArrayElements(jabImgBuf, pjb, 0);

	return nRet;
}

extern "C" JNIEXPORT jint JNICALL Java_com_gingytech_imageanalysis_FingerprintSensor_native_1bioEnrollFinish
		(JNIEnv * env, jobject thiz, jintArray janFid){
	int fid = -1;
	jint nRet = EnrollFinish(&fid);
	env->SetIntArrayRegion(janFid, 0, 1,&fid);
	return nRet;
}

extern "C" JNIEXPORT jint JNICALL Java_com_gingytech_imageanalysis_FingerprintSensor_native_1bioAuthInit
		(JNIEnv * env, jobject thiz){
	AuthInit();
	return 0;
}

extern "C" JNIEXPORT jint JNICALL Java_com_gingytech_imageanalysis_FingerprintSensor_native_1bioAuthenticate
		(JNIEnv * env, jobject thiz, jbyteArray jabImgBuf){
	jbyte* pjbImg = env->GetByteArrayElements(jabImgBuf, 0);
	int nRet = Auth((unsigned char*)pjbImg);
	env->ReleaseByteArrayElements(jabImgBuf, pjbImg, 0);

	return nRet;
}

extern "C" JNIEXPORT jint JNICALL Java_com_gingytech_imageanalysis_FingerprintSensor_native_1Remove
		(JNIEnv * env, jobject thiz, jint jiFid){
	int nRet = Remove((int)jiFid);
	return nRet;
}

extern "C" JNIEXPORT jint JNICALL Java_com_gingytech_imageanalysis_FingerprintSensor_native_1RemoveMobile
        (JNIEnv * env, jobject thiz){
    int nRet = RemoveMobileFile();
    return nRet;
}

extern "C" JNIEXPORT jint JNICALL Java_com_gingytech_imageanalysis_FingerprintSensor_native_1bioImageQuality
		(JNIEnv * env, jobject thiz, jbyteArray jabImgBuf){
	if(!jabImgBuf){
		LOGD("img null ptr");
		return 0;
	}

	jbyte* pjb = env->GetByteArrayElements(jabImgBuf, 0);
	int nRet = GetImageQuality((unsigned char*)pjb);

	env->ReleaseByteArrayElements(jabImgBuf, pjb, 0);
	return nRet;
}

extern "C" JNIEXPORT jfloat JNICALL Java_com_gingytech_imageanalysis_FingerprintSensor_native_1ImageUniformity
		(JNIEnv * env, jobject thiz, jbyteArray jabImgBuf, jint jiBorderSize,jint jiImgWidth, jint jiImgHeight){
	if(!jabImgBuf){
		LOGD("img null ptr");
		return 0;
	}

	jbyte* pjb = env->GetByteArrayElements(jabImgBuf, 0);

	// 150*380的影像用3*7的切法來計算uniformity
	float uniformity_mean = 0;
	float uniformity_std = 0;
	CalUniformity((unsigned char*)pjb, jiImgWidth, jiImgHeight, 3, 7, &uniformity_mean, &uniformity_std);

    LOGD("uniformity mean: %f", uniformity_mean);
	LOGD("uniformity std: %f", uniformity_std);

	env->ReleaseByteArrayElements(jabImgBuf, pjb, 0);
	return uniformity_mean;
}