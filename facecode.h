#ifndef _FACECODE_H_
#define _FACECODE_H_

#include "arcsoft_fsdk_face_recognition.h"
#include "arcsoft_fsdk_face_detection.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int facedetect(AFD_FSDK_IMAGE image,PMRECT rect);

extern int facerecognition(AFR_FSDK_IMAGE image1,AFR_FSDK_FACEINPUT faceResult1,AFR_FSDK_IMAGE image2,AFR_FSDK_FACEINPUT faceResult2,MFloat *fSimilScore);

//从路径中提取文件名称
int getFileName(const char *path,char *filename);

//将图片从jpg格式转换成yuv格式
int jpgToyuv(char *path,int width,int height,char *convertName);

int facecomparison(char *path1,char *path2,float *fSimilScore);

#ifdef __cplusplus
}
#endif

#endif