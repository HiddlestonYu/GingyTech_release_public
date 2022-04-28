package com.gingytech.imageanalysis;
import java.nio.ByteBuffer;

import com.gingytech.gingyusb.Device;
import com.gingytech.gingyusb.GingyDev;
import com.gingytech.gingyusb.GingyDev.IUsbEventObserver;
import com.gingytech.gingyusb.Utils;

import android.content.Context;

public class FingerprintSensor {

	private final static String TAG = "GingyIA_FPSensor";
	private GingyDev mGDev = null;
	static {
		System.loadLibrary("gfp");
	}

	public FingerprintSensor(boolean bIsUseUsb, Context cx, IUsbEventObserver listenerUSB) {
		if(bIsUseUsb) {
			mGDev = new GingyDev(cx, listenerUSB);
		}
	}

	public boolean IsReady() {
		return mGDev.IsDeviceReady();
	}

	public boolean getImage(ByteBuffer buffer, int imgW, int imgH) {
		boolean bRet = mGDev.SetImgInfo(imgW, imgH);
		mGDev.EnableLamp(true);
		byte[] imgbuf = mGDev.GetRawImage();
		mGDev.EnableLamp(false);
		if(imgbuf != null) {
			buffer.put(imgbuf);
			return true;
		} else return false;
	}

	public int getImgWidth(){
		return mGDev.GetImgWidth();
	}

	public int getImgHeight(){
		return mGDev.GetImgHeight();
	}

	public void SetReg(int nAddr, int nVal) {
		mGDev.SetReg(nAddr, nVal);
	}

	public int GetReg(int nAddr) {
		return mGDev.GetReg(nAddr);
	}

	public boolean IsGingyNewDev() {
		if(mGDev.getDeviceType() == Device.DEV_TYPE_GINGY)
			return true;
		else return false;
	}

	public int GetDataBits() {
		return mGDev.GetDataBits();
	}

	public boolean SetModuleInfo(int nSensorType, int nSensorBits) {
		if(mGDev != null)
			return mGDev.SetModuleInfo(nSensorType, nSensorBits);
		return false;
	}

	public String GetGingyUsbInfo() {
		return GingyDev.ReleaseVersion;
	}

	public String GetFwVer() {
		return mGDev.GetSensorFWVersion();
	}

	public byte[] ConvertTo8Bits(byte[]data, int nWidth, int nHeight, int nDataBits) {
		return Utils.Convert2ByteTo1Byte2(data, nWidth, nHeight, nDataBits);
	}

	public void Isp2(byte[] input, byte[] output, byte[] updateBuffer, boolean bIsUpdate, boolean bDoIsp2) {
		if(bDoIsp2 == true) {
			native_ISP2(input, output, updateBuffer, bIsUpdate==true?1:0);
		} else {
			System.arraycopy(input, 0, output, 0, input.length);
		}
	}

	// Gingy ISP
	public  native void native_ispInit(int basicUpdateNum, byte[] updateBuffer, int nImgWidth, int nImgHeight);
	public  native void native_ISP1(byte[] input, byte[] background, int nDataBits, int nResultWidth[], int nResultHeight[], int updateResult[], boolean bDoIsp1);
	private native void native_ISP2(byte[] input, byte[] output, byte[] updateBuffer, int updateFlag);
	public  native int  native_ImgBgMake(byte[] input, int nWidth, int nHeight, boolean bIsTwoByte);
	public  native int  native_ImgBgGet(byte[] outdata, int nWidth, int nHeight, boolean bIsTwoByte);
	public  native void native_ImgBgFinish();
	public  native String native_getIspVer();

	// PB algorithm
	public native int native_algorInit(String storage_path, int nIspResultW, int nIspResultH);
	public native void native_algorUninit();
	public native int native_LoadSavedTemplatesFID(int fidList[]);
	public native int native_bioEnrollInit(int nEnrollNum);
	public native int native_bioEnrollScreen(byte ImgBuf[], int nQuality[], int nOverlapAll[], int nOverlapLast[]);
	public native int native_bioEnroll(byte ImgBuf[]);
	public native int native_bioEnrollFinish(int fid[]);
	public native int native_bioAuthInit();
	public native int native_bioAuthenticate(byte ImgBuf[]);
	public native int native_Remove(int fid);
	public native int native_RemoveMobile();
	public native int native_bioImageQuality(byte ImgBuf[]);

	public native float native_ImageUniformity(byte ImgBuf[], int nBorderSize,int imgWidth,int imgHeight);
}
