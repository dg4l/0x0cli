#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

struct memory{
    char* response;
    size_t size;
};
 
size_t cb(char* data, size_t size, size_t nmemb, void* clientp){
    size_t realsize = size * nmemb;
    struct memory* mem = (struct memory *)clientp;
    char* ptr = realloc(mem->response, mem->size + realsize + 1);
    if (!ptr){
        return 0;
    }
    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;
    return realsize;
}

// some stuff could be factored out of main,
// but right now i don't really see a reason to.
int main(int argc, char** argv){
    char* filename = NULL;
    char* expires = NULL;
    char* url = NULL;
    char* secret = NULL;
    if (argc < 2){
        fprintf(stderr, "no filename provided!\n");
        return 0;
    }
    int c;
    while ((c = getopt(argc, argv, "f:e:u:s:")) != -1){
        switch(c){
            case 'f':
                filename = optarg;
                break;
            case 'e':
                expires = optarg;
                break;
            case 'u':
                url = optarg;
                break;
            case 's':
                secret = optarg;
                break;
            case '?':
                return 1;
        }
    }
    CURL* curl = NULL;
    struct memory response_data = {0};
    struct curl_httppost* post = NULL;
    struct curl_httppost* last = NULL;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    // todo: replace deprecated stuff with mime stuff.
    if (!curl){
        return 1;
    }
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response_data);
    curl_easy_setopt(curl, CURLOPT_URL, "https://0x0.st");
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "0x0cli/0.1");
    if (filename){
        curl_formadd(&post, &last, CURLFORM_COPYNAME, "file", CURLFORM_FILE, filename, CURLFORM_END);
    }
    if (secret){
        curl_formadd(&post, &last, CURLFORM_COPYNAME, "secret", CURLFORM_COPYCONTENTS, secret, CURLFORM_END);
    }
    if (url){
        curl_formadd(&post, &last, CURLFORM_COPYNAME, "url", CURLFORM_COPYCONTENTS, url, CURLFORM_END);
    }
    if (expires){
        curl_formadd(&post, &last, CURLFORM_COPYNAME, "expires", CURLFORM_COPYCONTENTS, expires, CURLFORM_END);
    }
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    printf("%s\n", response_data.response);
    curl_formfree(post);
    free(response_data.response);
    curl_global_cleanup();
    return 0;
}
