#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "facecode.h"


#define INPUT1_IMAGE_WIDTH   (480)
#define INPUT1_IMAGE_HEIGHT  (640)
#define INPUT1_IMAGE_FORMAT  ASVL_PAF_I420

#define INPUT2_IMAGE_FORMAT  ASVL_PAF_I420
#define INPUT2_IMAGE_WIDTH   (480)
#define INPUT2_IMAGE_HEIGHT  (640)

//get filename
int getFileName(char *path,char *filename)
{
    if (NULL == path) {
        return -1;
    }

    char *name_start = NULL;
    char *name_end = NULL;
    /*int flag = 0;

    name_start = strrchr(path, '/');
    if(name_start == NULL)
    {
       name_start = path;
    }
    else
       flag = 1;*/
    name_end = strrchr(path, '.');
    if((name_end == NULL) || (name_end == path))
        return -1;
    memcpy(filename, path, name_end - path);
    //memcpy(filename, name_start+flag, name_end - name_start-flag);
    return 0;
}

//jpg to yuv
int jpgToyuv(char *path,int width,int height,char *convertName)
{
    if(path == NULL)
        return -1;
   
    //if (image format )
   
    char cmdstring[256] = {0};
    char imageName[256] = {0};
    int ret = getFileName(path,imageName);
    if(0 != ret)
    {
        printf("getFileName errno\n");
        return -1;
    }
    sprintf(convertName,"%s.yuv",imageName);
    sprintf(cmdstring,"ffmpeg -i %s -s %dx%d -pix_fmt yuv420p -y %s",path,width,height,convertName);
    
    ret = -1;
    ret = system(cmdstring);
    if(ret < 0)
    {
        printf("cmd: %s\t error: %s", cmdstring, strerror(errno));
        return -1;
    }
    return 0;
}


int facecomparison(char *path1,char *path2,float *fSimilScore)
{
    AFD_FSDK_IMAGE fd_image1 = {0};
    AFD_FSDK_IMAGE fd_image2 = {0};
    AFR_FSDK_IMAGE fr_image1 = {0};
    AFR_FSDK_IMAGE fr_image2 = {0};
    AFR_FSDK_FACEINPUT fr_faceResult1;
    AFR_FSDK_FACEINPUT fr_faceResult2;
    
    PMRECT rect = (PMRECT)malloc(sizeof(MRECT));
    int ret = -1;
    char imageName1[256] = {0};
    char imageName2[256] = {0};
    
 //image 1
    //1.jpg convert to yuv
    ret = jpgToyuv(path1,INPUT1_IMAGE_WIDTH,INPUT1_IMAGE_HEIGHT,imageName1);
    if(ret != 0)
    {
        printf("image formate convert error\n");
        free(rect);
        return -1;
    }
    //2.detect face rect
    fd_image1.lPath = imageName1;
    fd_image1.format = INPUT1_IMAGE_FORMAT;
    fd_image1.width = INPUT1_IMAGE_WIDTH;
    fd_image1.height = INPUT1_IMAGE_HEIGHT;
    
    ret = -1;
    ret = facedetect(fd_image1, rect);
    if(ret != 0)
    {
        printf("face detect error\n");
        free(rect);
        return -1;
    }
    
    fr_faceResult1.lOrient = AFR_FSDK_FOC_0;
    fr_faceResult1.rcFace.left = rect->left;
    fr_faceResult1.rcFace.top = rect->top;
    fr_faceResult1.rcFace.right = rect->right;
    fr_faceResult1.rcFace.bottom = rect->bottom;
    
    
  //image 2
    //1.jpg convert to yuv
    ret = -1;
    ret = jpgToyuv(path2,INPUT2_IMAGE_WIDTH,INPUT2_IMAGE_HEIGHT,imageName2);
    if(ret != 0)
    {
        printf("image formate convert error\n");
        free(rect);
        return -1;
    }
    //2.detect face rect
    fd_image2.lPath = imageName2;
    fd_image2.format = INPUT2_IMAGE_FORMAT;
    fd_image2.width = INPUT2_IMAGE_WIDTH;
    fd_image2.height = INPUT2_IMAGE_HEIGHT;
    
    ret = -1;
    memset(rect, 0, sizeof(MRECT));
    ret = facedetect(fd_image2, rect);
    if(ret != 0)
    {
        printf("face detect error\n");
        free(rect);
        return -1;
    }
    
    fr_faceResult2.lOrient = AFR_FSDK_FOC_0;
    fr_faceResult2.rcFace.left = rect->left;
    fr_faceResult2.rcFace.top = rect->top;
    fr_faceResult2.rcFace.right = rect->right;
    fr_faceResult2.rcFace.bottom = rect->bottom;

  //face comparison
    fr_image1.lPath = imageName1;
    fr_image1.format = INPUT1_IMAGE_FORMAT;
    fr_image1.width = INPUT1_IMAGE_WIDTH;
    fr_image1.height = INPUT1_IMAGE_HEIGHT;
    
    fr_image2.lPath = imageName2;
    fr_image2.format = INPUT2_IMAGE_FORMAT;
    fr_image2.width = INPUT2_IMAGE_WIDTH;
    fr_image2.height = INPUT2_IMAGE_HEIGHT;
    
    ret = -1;
    ret = facerecognition(fr_image1,fr_faceResult1,fr_image2,fr_faceResult2,fSimilScore);
    if(ret != 0)
    {
        printf("face recognition error\n");
        free(rect);
        return -1;
    }
    
    printf("face recognition successed\n");
    
    free(rect);
    return 0;
}

int main(int argc,char*argv[])
{
    if(argc != 2)
    {
        printf("facecomparison imagepath1 imagepath2 \n");
        exit(0);
    }
    float fSimilScore = 0.0;
    facecomparison(argv[0],argv[1],&fSimilScore);
    printf("fSimilScore === %f",fSimilScore);
    return 0;
}