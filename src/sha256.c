#include "sha256.h"
#include <stdio.h>
#include <stdlib.h>
#include <openssl/evp.h>


char* calculateSHA256(const char *path){
    // open file
    FILE *file = fopen(path, "rb");
    if (!file){
        perror("Ошибка");
        return NULL;
    }

    // create OpenSSL context
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (mdctx == NULL){
        perror("Ошибка");
        fclose(file);
        return NULL;
    }

    // set sha256 as hashing algorithm
    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1){
        perror("Ошибка");
        EVP_MD_CTX_free(mdctx);
        fclose(file);
        return NULL;
    }

    // allocate buffer to read file by parts
    const int bufSize = 32768;
    unsigned char *buffer = malloc(bufSize);
    if (buffer == NULL){
        perror("Ошибка");
        EVP_MD_CTX_free(mdctx);
        fclose(file);
        return NULL;
    }

    // pass each part to OpenSSL hashing tool
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, bufSize, file)) > 0){
        if (EVP_DigestUpdate(mdctx, buffer, bytesRead) != 1){
            perror("Ошибка");
            free(buffer);
            EVP_MD_CTX_free(mdctx);
            fclose(file);
            return NULL;
        }
    }

    // check that there were no file-reading errors
    if (ferror(file)){
        perror("Ошибка");
        free(buffer);
        EVP_MD_CTX_free(mdctx);
        fclose(file);
        return NULL;
    }

    // allocate memory for the hash string
    char *hash_string = malloc(SHA256_HASH_LENGTH * 2 + 1);
    if (hash_string == NULL){
        perror("Ошибка");
        free(buffer);
        EVP_MD_CTX_free(mdctx);
        fclose(file);
        return NULL;
    }

    // put final file-hash to output variable
    unsigned char hash[SHA256_HASH_LENGTH];
    unsigned int output_length = SHA256_HASH_LENGTH;
    if (EVP_DigestFinal_ex(mdctx, hash, &output_length) != 1){
        perror("Ошибка");
        free(buffer);
        EVP_MD_CTX_free(mdctx);
        fclose(file);
        free(hash_string);
        return NULL;
    }

    // convert hash to a hex string
    for (unsigned int i = 0; i < SHA256_HASH_LENGTH; i++){
        sprintf(&hash_string[i * 2], "%02x", hash[i]);
    }
    hash_string[SHA256_HASH_LENGTH * 2] = '\0';

    EVP_MD_CTX_free(mdctx);
    fclose(file);
    free(buffer);

    return hash_string;
}