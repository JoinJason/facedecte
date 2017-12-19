#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "arcsoft_fsdk_face_recognition.h"
#include "merror.h"

#define APPID     "6oFpQrmZG5e9BGD595YhiZWRv5rmNeS8MXwPNNPhpAwh"
#define SDKKEY    "24qHQiaSGNPLvVEgM1MYhZfoYZUJsEZEp3vs6CvYB9Em"

#define nullptr NULL
#define WORKBUF_SIZE        (40*1024*1024)

int fu_ReadFile(const char* path, uint8_t **raw_data, size_t* pSize) {
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

int facerecognition(AFR_FSDK_IMAGE image1,AFR_FSDK_FACEINPUT faceResult1,AFR_FSDK_IMAGE image2,AFR_FSDK_FACEINPUT faceResult2,MFloat *fSimilScore)
{
    MByte *pWorkMem = (MByte *)malloc(WORKBUF_SIZE);
    if(pWorkMem == nullptr){
        fprintf(stderr, "fail to malloc engine work buffer\r\n");
        //exit(0);
        return -1;
    }

    MHandle hEngine = nullptr;

    //初始化人脸比对引擎
    int ret = AFR_FSDK_InitialEngine(APPID, SDKKEY, pWorkMem, WORKBUF_SIZE, &hEngine);
    if (ret != 0) {
        fprintf(stderr, "fail to AFR_FSDK_InitialEngine(): 0x%x\r\n", ret);
        free(pWorkMem);
        //exit(0);
        return -1;
    }

    //获取人脸比对引擎版本信息
    const AFR_FSDK_Version*pVersionInfo = AFR_FSDK_GetVersion(hEngine);
    printf("%d %d %d %d\r\n", pVersionInfo->lCodebase, pVersionInfo->lMajor,
                                 pVersionInfo->lMinor, pVersionInfo->lBuild);
    printf("%s\r\n", pVersionInfo->Version);
    printf("%s\r\n", pVersionInfo->BuildDate);
    printf("%s\r\n", pVersionInfo->CopyRight);

    //待比对的图片1
    ASVLOFFSCREEN inputImg1 = { 0 };
    inputImg1.u32PixelArrayFormat = image1.format;
    inputImg1.i32Width = image1.width;
    inputImg1.i32Height = image1.height;
    inputImg1.ppu8Plane[0] = nullptr;
    fu_ReadFile(image1.lPath, (uint8_t**)&inputImg1.ppu8Plane[0], nullptr);
     if (!inputImg1.ppu8Plane[0]) {
        fprintf(stderr, "fail to fu_ReadFile(%s): %s\r\n", image1.lPath, strerror(errno));
        AFR_FSDK_UninitialEngine(hEngine);
        free(pWorkMem);
        //exit(0);
        return -1;
    }
    inputImg1.pi32Pitch[0] = inputImg1.i32Width;
    inputImg1.pi32Pitch[1] = inputImg1.i32Width/2;
    inputImg1.pi32Pitch[2] = inputImg1.i32Width/2;
    inputImg1.ppu8Plane[1] = inputImg1.ppu8Plane[0] + inputImg1.pi32Pitch[0] * inputImg1.i32Height;
    inputImg1.ppu8Plane[2] = inputImg1.ppu8Plane[1] + inputImg1.pi32Pitch[1] * inputImg1.i32Height/2;

    //待比对的图片2
    ASVLOFFSCREEN inputImg2 = { 0 };
    inputImg2.u32PixelArrayFormat = image2.format;
    inputImg2.i32Width = image2.width;
    inputImg2.i32Height = image2.height;
    inputImg2.ppu8Plane[0] = nullptr;
    fu_ReadFile(image2.lPath, (uint8_t**)&inputImg2.ppu8Plane[0], nullptr);
     if (!inputImg2.ppu8Plane[0]) {
        fprintf(stderr, "fail to fu_ReadFile(%s): %s\r\n", image2.lPath, strerror(errno));
        free(inputImg1.ppu8Plane[0]);
        AFR_FSDK_UninitialEngine(hEngine);
        free(pWorkMem);
        //exit(0);
        return -1;
    }
    inputImg2.pi32Pitch[0] = inputImg2.i32Width;
    inputImg2.pi32Pitch[1] = inputImg2.i32Width/2;
    inputImg2.pi32Pitch[2] = inputImg2.i32Width/2;
    inputImg2.ppu8Plane[1] = inputImg2.ppu8Plane[0] + inputImg2.pi32Pitch[0] * inputImg2.i32Height;
    inputImg2.ppu8Plane[2] = inputImg2.ppu8Plane[1] + inputImg2.pi32Pitch[1] * inputImg2.i32Height/2;
    
    //获取图片1的人脸特征信息
    AFR_FSDK_FACEMODEL faceModels1 = { 0 };
    {
        /*AFR_FSDK_FACEINPUT faceResult;
        faceResult.lOrient = AFR_FSDK_FOC_0;
        faceResult.rcFace.left = 241;
        faceResult.rcFace.top = 127;
        faceResult.rcFace.right = 428;
        faceResult.rcFace.bottom = 315;*/
        AFR_FSDK_FACEMODEL LocalFaceModels = { 0 };
        ret = AFR_FSDK_ExtractFRFeature(hEngine, &inputImg1, &faceResult1, &LocalFaceModels);
        if(ret != 0){
            fprintf(stderr, "fail to AFR_FSDK_ExtractFRFeature in Image A\r\n");
            free(inputImg2.ppu8Plane[0]);
            free(inputImg1.ppu8Plane[0]);
            AFR_FSDK_UninitialEngine(hEngine);
            free(pWorkMem);
            //exit(0);
            return -1;
        }
        faceModels1.lFeatureSize = LocalFaceModels.lFeatureSize;
        faceModels1.pbFeature = (MByte*)malloc(faceModels1.lFeatureSize);
        memcpy(faceModels1.pbFeature, LocalFaceModels.pbFeature, faceModels1.lFeatureSize);
        free(inputImg1.ppu8Plane[0]);
    }
    
    //获取图片2的人脸特征信息
    AFR_FSDK_FACEMODEL faceModels2 = { 0 };
    {
        /*AFR_FSDK_FACEINPUT faceResult;
        faceResult.lOrient = AFR_FSDK_FOC_0;
        faceResult.rcFace.left = 241;
        faceResult.rcFace.top = 127;
        faceResult.rcFace.right = 428;
        faceResult.rcFace.bottom = 315;*/
        AFR_FSDK_FACEMODEL LocalFaceModels = { 0 };
        ret = AFR_FSDK_ExtractFRFeature(hEngine, &inputImg2, &faceResult2, &LocalFaceModels);
        if(ret != 0){
            fprintf(stderr, "fail to AFR_FSDK_ExtractFRFeature in Image B\r\n");
            free(inputImg2.ppu8Plane[0]);
            AFR_FSDK_UninitialEngine(hEngine);
            free(pWorkMem);
            //exit(0);
            return -1;
        }
        faceModels2.lFeatureSize = LocalFaceModels.lFeatureSize;
        faceModels2.pbFeature = (MByte*)malloc(faceModels2.lFeatureSize);
        memcpy(faceModels2.pbFeature, LocalFaceModels.pbFeature, faceModels2.lFeatureSize);
        free(inputImg2.ppu8Plane[0]);
    }

    //人脸特征信息比对
    *fSimilScore = 0.0f;
    ret = AFR_FSDK_FacePairMatching(hEngine, &faceModels1, &faceModels2, fSimilScore);
    printf("fSimilScore ==  %f\r\n", *fSimilScore);

    free(faceModels1.pbFeature);
    free(faceModels2.pbFeature);
    AFR_FSDK_UninitialEngine(hEngine);
    free(pWorkMem);

    return 0;
}
