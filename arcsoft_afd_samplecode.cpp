#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "arcsoft_fsdk_face_detection.h"
#include "merror.h"

#define APPID     "6oFpQrmZG5e9BGD595YhiZWRv5rmNeS8MXwPNNPhpAwh"
#define SDKKEY    "24qHQiaSGNPLvVEgM1MYhZfgPADCoLnvs38DD3rGvsXE"

#define nullptr NULL
#define WORKBUF_SIZE        (40*1024*1024)
#define MAX_FACE_NUM        (50)

int fd_ReadFile(const char* path, uint8_t **raw_data, size_t* pSize) {
    int res = 0;
    FILE *fp = 0;
    uint8_t *data_file = 0;
    size_t size = 0;

    fp = fopen(path, "rb");
    if (fp == nullptr) {
        res = -1;
        goto exit;
    }

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    data_file = (uint8_t *)malloc(sizeof(uint8_t)* size);
    if (data_file == nullptr) {
        res = -2;
        goto exit;
    }

    if (size != fread(data_file, sizeof(uint8_t), size, fp)) {
        res = -3;
        goto exit;
    }

    *raw_data = data_file;
    data_file = nullptr;
exit:
    if (fp != nullptr) {
        fclose(fp);
    }

    if (data_file != nullptr) {
        free(data_file);
    }

    if (nullptr != pSize) {
        *pSize = size;
    }
    
    return res;
}


int facedetect(AFD_FSDK_IMAGE image,PMRECT rect)
{
    MByte *pWorkMem = (MByte *)malloc(WORKBUF_SIZE);
    if(pWorkMem == nullptr){
        fprintf(stderr, "fail to malloc workbuf\r\n");
        //exit(0);
        return -1;
    }

    MHandle hEngine = nullptr;

    //初始化人脸检测引擎
    int ret = AFD_FSDK_InitialFaceEngine(APPID, SDKKEY, pWorkMem, WORKBUF_SIZE, 
                                         &hEngine, AFD_FSDK_OPF_0_HIGHER_EXT, 16, MAX_FACE_NUM);
    if (ret != 0) {
        fprintf(stderr, "fail to AFD_FSDK_InitialFaceEngine(): 0x%x\r\n", ret);
        free(pWorkMem);
        //exit(0);
        return -1;
    }

    //获取人脸检测引擎版本信息
    const AFD_FSDK_Version*pVersionInfo = AFD_FSDK_GetVersion(hEngine);
    printf("%d %d %d %d\r\n", pVersionInfo->lCodebase, pVersionInfo->lMajor,
                                 pVersionInfo->lMinor, pVersionInfo->lBuild);
    printf("%s\r\n", pVersionInfo->Version);
    printf("%s\r\n", pVersionInfo->BuildDate);
    printf("%s\r\n", pVersionInfo->CopyRight);
    
    //待检测图片信息
    ASVLOFFSCREEN inputImg = { 0 };
    inputImg.u32PixelArrayFormat = image.format;
    inputImg.i32Width = image.width;
    inputImg.i32Height = image.height;
    inputImg.ppu8Plane[0] = nullptr;
    fd_ReadFile(image.lPath, (uint8_t**)&inputImg.ppu8Plane[0], nullptr);
    if (!inputImg.ppu8Plane[0]) {
        fprintf(stderr, "fail to fu_ReadFile(%s): %s\r\n", image.lPath, strerror(errno));
		AFD_FSDK_UninitialFaceEngine(hEngine);
        free(pWorkMem);
        //exit(0);
        return -1;
    }

    //根据不同yuv采样格式进行设置
    if (ASVL_PAF_I420 == inputImg.u32PixelArrayFormat) {
        inputImg.pi32Pitch[0] = inputImg.i32Width;
        inputImg.pi32Pitch[1] = inputImg.i32Width/2;
        inputImg.pi32Pitch[2] = inputImg.i32Width/2;
        inputImg.ppu8Plane[1] = inputImg.ppu8Plane[0] + inputImg.pi32Pitch[0] * inputImg.i32Height;
        inputImg.ppu8Plane[2] = inputImg.ppu8Plane[1] + inputImg.pi32Pitch[1] * inputImg.i32Height/2;
    } else if (ASVL_PAF_NV12 == inputImg.u32PixelArrayFormat) {
        inputImg.pi32Pitch[0] = inputImg.i32Width;
        inputImg.pi32Pitch[1] = inputImg.i32Width;
        inputImg.ppu8Plane[1] = inputImg.ppu8Plane[0] + (inputImg.pi32Pitch[0] * inputImg.i32Height);
    } else if (ASVL_PAF_NV21 == inputImg.u32PixelArrayFormat) {
        inputImg.pi32Pitch[0] = inputImg.i32Width;
        inputImg.pi32Pitch[1] = inputImg.i32Width;
        inputImg.ppu8Plane[1] = inputImg.ppu8Plane[0] + (inputImg.pi32Pitch[0] * inputImg.i32Height);
    } else if (ASVL_PAF_YUYV == inputImg.u32PixelArrayFormat) {
        inputImg.pi32Pitch[0] = inputImg.i32Width*2;
    } else if (ASVL_PAF_I422H == inputImg.u32PixelArrayFormat) {
        inputImg.pi32Pitch[0] = inputImg.i32Width;
        inputImg.pi32Pitch[1] = inputImg.i32Width / 2;
        inputImg.pi32Pitch[2] = inputImg.i32Width / 2;
        inputImg.ppu8Plane[1] = inputImg.ppu8Plane[0] + inputImg.pi32Pitch[0] * inputImg.i32Height;
        inputImg.ppu8Plane[2] = inputImg.ppu8Plane[1] + inputImg.pi32Pitch[1] * inputImg.i32Height;
    } else if (ASVL_PAF_LPI422H == inputImg.u32PixelArrayFormat) {
        inputImg.pi32Pitch[0] = inputImg.i32Width;
        inputImg.pi32Pitch[1] = inputImg.i32Width;
        inputImg.ppu8Plane[1] = inputImg.ppu8Plane[0] + (inputImg.pi32Pitch[0] * inputImg.i32Height);
    } else if (ASVL_PAF_RGB24_B8G8R8 == inputImg.u32PixelArrayFormat) {
        inputImg.pi32Pitch[0] = inputImg.i32Width*3;
    } else {
        fprintf(stderr, "unsupported Image format: 0x%x\r\n",inputImg.u32PixelArrayFormat);
        free(inputImg.ppu8Plane[0]);
        AFD_FSDK_UninitialFaceEngine(hEngine);
        free(pWorkMem);
        //exit(0);
        return -1;
    }
    
    //检测人脸位置
    LPAFD_FSDK_FACERES faceResult;
    ret = AFD_FSDK_StillImageFaceDetection(hEngine, &inputImg, &faceResult);
    if (ret != 0) {
        fprintf(stderr, "fail to AFD_FSDK_StillImageFaceDetection(): 0x%x\r\n", ret);
        free(inputImg.ppu8Plane[0]);
		AFD_FSDK_UninitialFaceEngine(hEngine);
        free(pWorkMem);
        //exit(0);
        return -1;
    }
    
    for (int i = 0; i < faceResult->nFace; i++) {
        printf("face %d:(%d,%d,%d,%d)\r\n", i, 
               faceResult->rcFace[i].left, faceResult->rcFace[i].top,
               faceResult->rcFace[i].right, faceResult->rcFace[i].bottom);
        rect->left = faceResult->rcFace[i].left;
        rect->top = faceResult->rcFace[i].top;
        rect->right = faceResult->rcFace[i].right;
        rect->bottom = faceResult->rcFace[i].bottom;
    }

    free(inputImg.ppu8Plane[0]);
	AFD_FSDK_UninitialFaceEngine(hEngine);
    free(pWorkMem);
    return 0;
}
