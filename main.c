#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include "unistd.h"
#include "cJSON.h"
#include <string.h>
#define MAX_STRING_SIZE  20

static void (*cJSON_free)(void* ptr) = free;

cJSON* open_cfgfile(const char* fname)
{
    FILE* fp;
    cJSON* root;
    int nbyte, size;

    fp = fopen(fname, "rb");
    if (fp == NULL) {
        perror("fopen");
        return NULL;
    }
    char* src;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    src = malloc(size + 1);
    nbyte = fread(src, 1, size, fp);
    if (nbyte < 0) {
        free(src);
        return NULL;
    }
    cJSON_Minify(src);
    // 解析数据包
    root = cJSON_Parse(src);
    if (!root)
    {
        printf("Error before: [%s]\n", cJSON_GetErrorPtr());
        return NULL;
    }

    free(src);
    fclose(fp);

    return root;
}

int close_cfgfile(cJSON* json)
{
    // 释放内存空间
    cJSON_Delete(json);

    return 0;
}

cJSON* replace_value_from_cfgfile_with_class(cJSON* json, char* node, char* item, int replaceint)
{
    cJSON* json_value, * root;
    root = cJSON_GetObjectItem(json, node);
    if (!root)
        return NULL;

    json_value = cJSON_GetObjectItem(root, item);
    if (json_value && json_value->type == cJSON_Number)
    {
        json_value->valueint = replaceint;
        json_value->valuedouble = replaceint;
    }
    else
        return NULL;

    return json;
}

cJSON* replace_value_from_cfgfile(cJSON* json, char* item, int replaceint)
{
    cJSON* json_value;
    json_value = cJSON_GetObjectItem(json, item);
    if (!json_value)
        return NULL;

    if (json_value && json_value->type == cJSON_Number)
    {
        json_value->valueint = replaceint;
        json_value->valuedouble = replaceint;
    }
    else
        return NULL;

    return json;
}

cJSON* replace_string_from_cfgfile(cJSON* json, char* item, char* replacestr)
{
    cJSON* json_value;
    json_value = cJSON_GetObjectItem(json, item);
    if (!json_value)
        return NULL;

    if (json_value && json_value->type == cJSON_String) {
        cJSON_free(json_value->valuestring);
        json_value->valuestring = replacestr;
    }
    else
        return NULL;

    return json;
}

cJSON* replace_all_string_from_cfgfile(cJSON* json, char item[][MAX_STRING_SIZE], int itemsize, char replacestr[][MAX_STRING_SIZE], int replacestrsize)
{
    if (itemsize != replacestrsize)
        return NULL;

    for (int i = 0; i < itemsize; i++){
        cJSON* json_value;
        json_value = cJSON_GetObjectItem(json, item[i]);
        if (!json_value)
            return NULL;

        if (json_value && json_value->type == cJSON_String) {
            cJSON_free(json_value->valuestring);
            json_value->valuestring = replacestr[i];
        }
        else
            return NULL;
    }  

    return json;
}

cJSON* replace_string_in_array_from_cfgfile(cJSON* json, char* array_item, char replacestr[][MAX_STRING_SIZE], int size)
{
    cJSON* json_array;

    json_array = cJSON_GetObjectItem(json, array_item);
    if (!json_array)
        return NULL;

    int array_size = cJSON_GetArraySize(json_array);
    if (0 == array_size || size != array_size)
        return NULL;
    
    for (int iCnt = 0; iCnt < array_size; iCnt++) {
        cJSON* pSub = cJSON_GetArrayItem(json_array, iCnt);
        if (pSub && pSub->type == cJSON_String) {
            cJSON_free(pSub->valuestring);
            pSub->valuestring = replacestr[iCnt];
        }
        else
            return NULL;
    }   
    return json;
}

int main(void)
{ 
    unsigned char json_file[500];
    cJSON* json_before = open_cfgfile("Stmsi_json.json");
    char* node = cJSON_Print(json_before);
    printf("modify before:");
    printf(node);
    printf("\n");

    char item[][MAX_STRING_SIZE] = { "Cell_Founded", "RSRP", "EVM", "Probability" };
    char nullstr[][MAX_STRING_SIZE] = { " ", " ", " ", " " };
    char replacestr[4][MAX_STRING_SIZE] = { 0 };
    double array[3] = { 111.01,2223.33,33344.45 };
    char replacestmsi[4][MAX_STRING_SIZE] = { 0 };
    
    for (int i = 0; i < 3; i++)
        sprintf(replacestmsi[i], "%.2f", array[i]);

    /*for (int j = 0; j < 3; j++)
        printf("%s ", replacestmsi[j]);*/

    int numvalue1 = 1234;
    char str1[MAX_STRING_SIZE];
    sprintf(str1, "%d", numvalue1);
    memcpy(&replacestr[0], &str1, strlen(str1));
    int numvalue2 = 14;
    char str2[MAX_STRING_SIZE];
    sprintf(str2, "%d", numvalue2);
    memcpy(&replacestr[1], &str2, strlen(str2));
    int numvalue3 = 234;
    char str3[MAX_STRING_SIZE];
    sprintf(str3, "%d", numvalue3);
    memcpy(&replacestr[2], &str3, strlen(str3));
    int numvalue4 = 34;
    char str4[MAX_STRING_SIZE];
    sprintf(str4, "%d", numvalue4);
    memcpy(&replacestr[3], &str4, strlen(str4));

    cJSON* json_after1 = replace_all_string_from_cfgfile(json_before, item, 4, nullstr, 4);
    char* node1 = cJSON_Print(json_after1);
    printf("modify after1:");
    printf(node1);
    printf("\n");

    json_before = open_cfgfile("Stmsi_json.json");
    cJSON* json_after2 = replace_all_string_from_cfgfile(json_before, item, 4, replacestr, 4);
    cJSON* json_after3 = replace_string_in_array_from_cfgfile(json_after2, "Stmsi", replacestmsi, 3);
    cJSON* json_after4 = replace_string_in_array_from_cfgfile(json_after3, "Page", replacestmsi, 3);
    cJSON* json_after5 = replace_string_in_array_from_cfgfile(json_after4, "Msg3", replacestmsi, 3);
    char* node2 = cJSON_Print(json_after2);
    printf("modify after2:");
    printf(node2);
    printf("\n");

    memcpy(json_file, node2, strlen(node2));
    json_file[strlen(node2) + 1] = '\0';
    printf("copy content:");
    printf(json_file);
    printf("\n");
    FILE* fp = fopen("Stmsi_1.json", "w+");
    //char* buff = cJSON_Print(json_after);
    printf("%d",strlen(json_file));
    fwrite(json_file, strlen(json_file), 1, fp);
    //清理工作
    fclose(fp);
    //close_cfgfile(json_before);
    
    return 0;
}
