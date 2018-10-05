#include <iostream>
//#include <std_msgs/String.h>
#include <curl/curl.h>
#include <vector>
#include <malloc.h>
#include <string.h>
#include <jsoncpp/json/json.h>
#include <string>
//#include "access_token.h"
using namespace std;
#define PATH_LEN    50
// libcurl库下载链接：https://curl.haxx.se/download.html
// jsoncpp库下载链接：https://github.com/open-source-parsers/jsoncpp/
// 获取access_token所需要的url
const std::string access_token_url = "https://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials";
static const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static std::string face_match_result;
string search_result;
// 回调函数获取到的access_token存放变量
// static std::string access_token_result;
/**
 * curl发送http请求调用的回调函数，回调函数中对返回的json格式的body进行了解析，解析结果储存在result中
 * @param 参数定义见libcurl库文档
 * @return 返回值定义见libcurl库文档
 */
const static std::string request_url = "https://aip.baidubce.com/rest/2.0/face/v3/search";//ren lian jian ce di zhi
//const static std::string face_match_url = "https://aip.baidubce.com/rest/2.0/face/v3/match";//ren lian shi bie di zhi

static std::string detect_result;

static size_t callback(void *ptr, size_t size, size_t nmemb, void *stream) {
    // 获取到的body存放在ptr中，先将其转换为string格式
    std::string s((char *) ptr, size * nmemb);
    // 开始获取json中的access token项目
    Json::Reader reader;
    Json::Value root;
    // 使用boost库解析json
    reader.parse(s,root);
    std::string* access_token_result = static_cast<std::string*>(stream);
    *access_token_result = root["access_token"].asString();
    return size * nmemb;
}
//返回人脸对比
int writer(char *data,size_t size,size_t nmemb,string *writerData)
{
	if (writerData ==NULL)
	{
		return -1;
	}
	int len =size*nmemb;
	writerData->append(data,len);
	return len;
}
/**
 * 用以获取access_token的函数，使用时需要先在百度云控制台申请相应功能的应用，获得对应的API Key和Secret Key
 * @param access_token 获取得到的access token，调用函数时需传入该参数
 * @param AK 应用的API key
 * @param SK 应用的Secret key
 * @return 返回0代表获取access token成功，其他返回值代表获取失败
 */
int get_access_token(std::string &access_token, const std::string &AK, const std::string &SK) {
    CURL *curl;
    CURLcode result_code;
    int error_code = 0;
    curl = curl_easy_init();
    if (curl) {
        std::string url = access_token_url + "&client_id=" + AK + "&client_secret=" + SK;
        curl_easy_setopt(curl, CURLOPT_URL, url.data());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        std::string access_token_result;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &access_token_result);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
        result_code = curl_easy_perform(curl);
        if (result_code != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(result_code));
            return 1;
        }
        access_token = access_token_result;
        //cout<<access_token_result<<'2'<<endl;
        curl_easy_cleanup(curl);
        error_code = 0;
    } else {
        fprintf(stderr, "curl_easy_init() failed.");
        error_code = 1;
    }
    return error_code;
}
int search(std::string &json_result, const std::string &access_token,std::string &json) {
    std::string url = request_url + "?access_token=" + access_token;
    CURL *curl = NULL;
    CURLcode result_code;
    int is_success;
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.data());
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type:application/json;charset=UTF-8");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,writer);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &search_result);
        result_code = curl_easy_perform(curl);
        if (result_code != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(result_code));
            is_success = 1;
            return is_success;
        }
        json_result = search_result;
        curl_easy_cleanup(curl);
        is_success = 0;
    } else {
        fprintf(stderr, "curl_easy_init() failed.");
        is_success = 1;
    }
    return is_success;
}

int Read_Image(char *path, char* out)
{
	FILE *fd;
	int length;
	int ret;
	
	fd = fopen(path,"r");
	if(fd==NULL)
	{
		cout<<"Read_Image open file err"<<endl;
	}
	
	fseek(fd, 0, SEEK_END);
	length = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	ret = fread(out, 1, length, fd);
	if(ret == 0)
	{
		cout<<"Read_Image fread err" <<endl;
	}
	fclose(fd);
	
	return ret;
}
//chsrc为源数据，chdes为Base64编码后的数据，len为数据长度
void Base64_Code(char* chsrc, char* chdes, int len)
{
      unsigned char char_array_3[3], char_array_4[4];
      int i = 0, j = 0;
  
      while(len--)
      {
            char_array_3[i++] = *(chsrc++);
            if(3 == i)
            {
                  char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                  char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                  char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;
                  for(i = 0; i < 4; i++)
                        *(chdes+i) = base64_chars[char_array_4[i]];
     
                  i = 0;
                 chdes += 4;
            }
      }
      if(i)
      {
             for(j = i; j < 3; j++)
             char_array_3[j] = '\0';
   
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
   
            for(j = 0; j < (i+1); j++)
                  *(chdes++) = base64_chars[char_array_4[j]];
    
            while((3 > i++))
                  *(chdes++) = '=';
      }
  
      *chdes = '\0';
      return;
}   

int Calculat_FlieLength(char *path)
{
	FILE *fd;
	int length;

	fd = fopen(path,"r");
	fseek(fd, 0, SEEK_END);
	length = ftell(fd);
	fseek(fd, 0, SEEK_SET);
	fclose(fd);
	return length;
	
}



int main()
{
    int ret=-1;
	int file_len = 0;
    int fread_len = 0;
	std::string  result ;
	std::string ImageA;
	std::string ImageB;
	char pathA[PATH_LEN] = "./1.jpg";
		
	char* PpathA = pathA;
	char* PdataA = NULL;
	char* Pbase64A = NULL;
	file_len = Calculat_FlieLength(pathA);
	PdataA = (char *)malloc(file_len);
	Pbase64A = (char *)malloc(file_len*3/2); /*base63 = 1.33 jpg*/
	fread_len = Read_Image(pathA, PdataA);

	if(file_len==fread_len)
	{
		Base64_Code(PdataA, Pbase64A, fread_len);
		ImageA = Pbase64A;
		cout << "fileA len = " << file_len<<"   "<< "Pbase64A len = " << strlen(Pbase64A)<< endl;
	}
	else
	{
		cout << "Image A read err" << endl;
	}
    std::string Secretkey="rLfEINAf8ecvUp89ORjeRY68Epk9fRSG";
    std::string APIkey="9o09t8GXTqAcUuveQaG4Erin";
    std::string AccessKey="0";
    ret= get_access_token(AccessKey,APIkey,Secretkey);
    cout<<"ret="<<ret<<endl;
    cout<<"Secretkey="<<Secretkey<<endl;;
    cout<<"AccessKey="<<AccessKey<<endl;
//编入json字符串
  Json::Value value;
    //std::string ssrc1 = "https://ss2.bdstatic.com/70cFvnSh_Q1YnxGkpoWK1HF6hhy/it/u=1715859303,169882199&fm=200&gp=0.jpg";
   std::string ssrc1 =ImageA;
	value["image"] = ssrc1;
 
    std::string ssrc2 = "BASE64";
    value["image_type"] = ssrc2;
	
    std::string ssrc3 = "faceshape,facetype";
    value["face_field"] = ssrc3;

    std::string ssrc4 = "group1";
	value["group_id_list"] = ssrc4;
    std::string body = value.toStyledString();
//cout<<body<<endl;
int res=search(search_result,AccessKey,body);
//int res=detect(detect_result,AccessKey);
cout<<"search_result="<<search_result<<endl;
	free(PdataA);
	free(Pbase64A);

	
    return 0;
}
