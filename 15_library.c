#include "main_func.h"
#include "15_last.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Максимальная длинна кода
const int max_length = 400;

void Build_Haffman_tree(Node *node, unsigned char *codeMap, char *codes, int *lastIndex, char *string_now){
    if (node->left == NULL && node->right == NULL) {
        codeMap[*lastIndex] = node->elem;
        char *stringPointer = codes + *lastIndex * max_length;
        char *currentStringPointer = string_now;

        int stringLength = strlen(string_now);
        for (int i = 0; i < stringLength; i++) {
            (*stringPointer) = (*currentStringPointer);
            stringPointer++;
            currentStringPointer++;
        }

        (*stringPointer) = '\0';

        (*lastIndex)++;
        return;
    }

    char currentStringLeft[max_length];
    char currentStringRight[max_length];
    strcpy(currentStringLeft, string_now);
    strcpy(currentStringRight, string_now);
    strcat(currentStringLeft, "0");
    strcat(currentStringRight, "1");

    Build_Haffman_tree(node->left, codeMap, codes, lastIndex, currentStringLeft);
    Build_Haffman_tree(node->right, codeMap, codes, lastIndex, currentStringRight);



}

void Compress_immerce_name(unsigned char *buffer, int *bufferPivot, char *string) {
    char *stringPivot = string;
    int stringLength = strlen(string);

    for (int i = 0; i < stringLength; i++) {
        if (string[i] == '0') {
            buffer[*bufferPivot / 8] &= ~(1 << (7 - (*bufferPivot % 8)));
        } else {
            buffer[*bufferPivot / 8] |= (1 << (7 - (*bufferPivot % 8)));
        }
        (*bufferPivot)++;
        stringPivot++;
    }
}

void create(char *archiveName, int argc, char **argv) {
    FILE *currentFile;

    for (int fileIndex = 0; fileIndex < argc - 4; fileIndex++) {
        currentFile = NULL;
        currentFile = fopen(argv[fileIndex + 4], "rb");
        if (currentFile == NULL) {
            printf("File  %s  not found!\n",argv[fileIndex + 4]);
        }
        fclose(currentFile);
    }


    int fileSizes[argc - 4];

    unsigned char currentByte;
    int c;
    int nodeFound;

    short nodesNumber = 0;
    Node * nodes[512];
    int active[512];
    for (int i = 0; i < 512; i++) active[i] = 0;

    for (int currentFileIndex = 0; currentFileIndex < argc - 4; currentFileIndex++) {

        currentFile = NULL;
        currentFile = fopen(argv[currentFileIndex + 4], "rb");

        fileSizes[currentFileIndex] = 0;

        while ((c = getc(currentFile)) != EOF) {

            fileSizes[currentFileIndex]++;
            currentByte = c;
            nodeFound = 0;

            for (int i = 0; i < nodesNumber; i++) {
                if (nodes[i]->elem == currentByte) {
                    nodes[i]->frequency++;
                    nodeFound = 1;
                    break;
                }
            }

            if (!nodeFound) {
                nodes[nodesNumber] = malloc(sizeof(Node));
                active[nodesNumber] = 1;
                nodes[nodesNumber]->elem = currentByte;
                nodes[nodesNumber]->frequency = 1;
                nodes[nodesNumber]->left = NULL;
                nodes[nodesNumber]->right = NULL;
                nodesNumber++;
            }
        }

        fclose(currentFile);
    }

    // Строим само дерево
    int lastIndex = nodesNumber;
    Node *firstMin, *secondMin;
    int firstMinValue, secondMinValue;
    int firstMinIndex, secondMinIndex;

    for (int iter = 0; iter < nodesNumber - 1; iter++) {
        firstMinValue = INT_MAX;
        secondMinValue = INT_MAX;

        for (int node = 0; node < lastIndex; node++) {
            if (active[node] && nodes[node] != NULL && nodes[node]->frequency < firstMinValue) {
                firstMinValue = nodes[node]->frequency;
                firstMin = nodes[node];
                firstMinIndex = node;
            }
        }

        for (int node = 0; node < lastIndex; node++) {
            if (active[node] && nodes[node] != NULL && nodes[node]->frequency < secondMinValue && firstMinIndex != node) {
                secondMinValue = nodes[node]->frequency;
                secondMin = nodes[node];
                secondMinIndex = node;
            }
        }

        active[firstMinIndex] = 0;
        active[secondMinIndex] = 0;
        nodes[lastIndex] = malloc(sizeof(Node));
        nodes[lastIndex]->left = firstMin;
        nodes[lastIndex]->right = secondMin;
        nodes[lastIndex]->frequency = nodes[firstMinIndex]->frequency + nodes[secondMinIndex]->frequency;
        active[lastIndex] = 1;
        lastIndex++;

    }

    Node *root;
    for (int i = 0; i < lastIndex; i++) {
        if (active[i]) {
            root = nodes[i];
            break;
        }
    }

    int initialSize = 0;
    for (int i = 0; i < argc - 4; i++) initialSize += fileSizes[i];

    unsigned char codeMap[nodesNumber];
    int codeMapLastIndex = 0;
    char codeMapCodes[nodesNumber][400];

    Build_Haffman_tree(root, codeMap, codeMapCodes, &codeMapLastIndex, "");
    char encodingTable[256][400] = {0};
    for (int i = 0; i < codeMapLastIndex; i++) strcpy(encodingTable[codeMap[i]], codeMapCodes[i]);

    // Создаём архив

    FILE *archiveFile = fopen(archiveName, "wb");

    short archiveFileNameSize = strlen(archiveName) + 1;
    fwrite(&archiveFileNameSize, 1, 2, archiveFile);

    fwrite(archiveName, 1, archiveFileNameSize, archiveFile);

    short numberOfFiles = argc - 4;
    fwrite(&numberOfFiles, sizeof(short), 1, archiveFile);

    short currentFileNameSize;
    for (int i = 0; i < numberOfFiles; i++) {
        currentFileNameSize = strlen(argv[i + 4]) + 1;
        fwrite(&currentFileNameSize, sizeof(short), 1, archiveFile);
        fwrite(argv[i + 4], 1, currentFileNameSize, archiveFile);
        fwrite(&fileSizes[i], 1, sizeof(int), archiveFile);
    }

    fwrite(&nodesNumber, 1, 2, archiveFile);

    short currentCodeSize;
    for (int i = 0; i < codeMapLastIndex; i++) {
        fwrite(&codeMap[i], 1, 1, archiveFile);
        currentCodeSize = strlen(codeMapCodes[i]) + 1;
        fwrite(&currentCodeSize, 1, 2, archiveFile);
        fwrite(codeMapCodes[i], 1, currentCodeSize, archiveFile);
    }

    unsigned char byteBuffer[20];
    int byteBufferPivot = 0;


    for (int currentFileIndex = 0; currentFileIndex < argc - 4; currentFileIndex++) {
        FILE *currentFile = fopen(argv[currentFileIndex + 4], "rb");
        while ((c = getc(currentFile)) != EOF) {
            currentByte = c;

            Compress_immerce_name(byteBuffer, &byteBufferPivot, encodingTable[currentByte]); //отвечает за сжатие название архива

            if (byteBufferPivot > 7) {
                for (int i = 0; i < byteBufferPivot / 8; i++) {
                    putc(byteBuffer[i], archiveFile);
                }
                byteBuffer[0] = byteBuffer[byteBufferPivot / 8];
                byteBufferPivot %= 8;
            }
        }

        fclose(currentFile);

    }

    putc(byteBuffer[0], archiveFile);
    char usefulBitsNumber = byteBufferPivot;
    putc(usefulBitsNumber, archiveFile);


    printf("Initial size:\t%d bytes \n", initialSize);
    printf("Final size:\t%ld bytes \n", ftell(archiveFile));
    printf("Compression:\t%.2f%% \n", ftell(archiveFile) * 100 / (double) initialSize);


    for (int i = 0; i < lastIndex; i++) free(nodes[i]);
    fclose(archiveFile);
}

int extract(char *fileName) {
    FILE *archiveFile = NULL;
    archiveFile = fopen(fileName, "rb");
    if (archiveFile == NULL){
        printf("ARCHIVE NOT FOUND");
    }

    fseek(archiveFile, 0, SEEK_END);
    int archiveSize = ftell(archiveFile);
    fseek(archiveFile, 0, SEEK_SET);

    short archiveNameSize;
    fread(&archiveNameSize, sizeof(short), 1, archiveFile);
    fseek(archiveFile, archiveNameSize, SEEK_CUR);

    short numberOfFiles;
    fread(&numberOfFiles, sizeof(short), 1, archiveFile);

    short filesNameSize[numberOfFiles];
    char filesName[numberOfFiles][2000];
    int filesSize[numberOfFiles];

    for (int currentFileIndex = 0; currentFileIndex < numberOfFiles; currentFileIndex++) {
        fread(&filesNameSize[currentFileIndex], sizeof(short), 1, archiveFile);
        fread(&filesName[currentFileIndex], 1, filesNameSize[currentFileIndex], archiveFile);
        fread(&filesSize[currentFileIndex], sizeof(int), 1, archiveFile);
    }


    for (int i = 0; i < numberOfFiles; i++) printf("%s   (%d bytes)  \n", filesName[i], filesSize[i]);


    short codeMapSize;
    fread(&codeMapSize, sizeof(short), 1, archiveFile);
    char encodingTable[256][max_length];
    char active[256];
    for (int i = 0; i < 256; i++) active[i] = 0;

    unsigned char currentCodeByte;
    short currentCodeSize;

    for (int i = 0; i < codeMapSize; i++) {
        fread(&currentCodeByte, 1, 1, archiveFile);
        active[currentCodeByte] = 1;
        fread(&currentCodeSize, sizeof(short), 1, archiveFile);
        fread(encodingTable[currentCodeByte], 1, currentCodeSize, archiveFile);
    }

    unsigned char currentByte;
    char buffer[max_length];
    buffer[0] = '\0';
    int bufferPointer = 0;
    int byteCount = 0;

    FILE *currentFile;

    for (int fileIndex = 0; fileIndex < numberOfFiles; fileIndex++) {
        printf("...extracting  %s  \n", filesName[fileIndex]);
        currentFile = fopen(filesName[fileIndex], "wb");
        byteCount = 0;

        while (byteCount < filesSize[fileIndex]) {
            printf("%d\n", byteCount);
            currentByte = getc(archiveFile);
            for (int bit = 0; bit < 8; bit++) {
                buffer[bufferPointer++] = ('0' + ((currentByte >> (7 - bit)) & 1));
                buffer[bufferPointer] = '\0';

                if (fileIndex == numberOfFiles - 1 && byteCount == filesSize[fileIndex]) break;

                for (int code = 0; code < 256; code++) {

                    if (active[code] && strcmp(buffer, encodingTable[code]) == 0) {

                        putc(code, currentFile);
                        bufferPointer = 0;
                        buffer[bufferPointer] = '\0';
                        byteCount++;
                        break;
                    }
                }
            }
        }
        fclose(currentFile);
    }

    fclose(archiveFile);

    printf("DONE!\n");
}


void list(char *fileName) {
    FILE *archiveFile = NULL;
    archiveFile = fopen(fileName, "rb");
    if (archiveFile == NULL){
        printf("ARCHIVE NOT FOUND");
    }

    fseek(archiveFile, 0, SEEK_END);
    int archiveSize = ftell(archiveFile);
    fseek(archiveFile, 0, SEEK_SET);

    short archiveNameSize;
    fread(&archiveNameSize, sizeof(short), 1, archiveFile);
    char archiveName[archiveNameSize];
    fread(archiveName, 1, archiveNameSize, archiveFile);
    printf(". . . %s . . . \n\n", archiveName);

    short numberOfFiles;
    fread(&numberOfFiles, sizeof(short), 1, archiveFile);

    short filesNameSize[numberOfFiles];
    char filesName[numberOfFiles][2000];
    int filesSize[numberOfFiles];

    for (int currentFileIndex = 0; currentFileIndex < numberOfFiles; currentFileIndex++) {
        fread(&filesNameSize[currentFileIndex], sizeof(short), 1, archiveFile);
        fread(&filesName[currentFileIndex], 1, filesNameSize[currentFileIndex], archiveFile);
        fread(&filesSize[currentFileIndex], sizeof(int), 1, archiveFile);

        printf("> %s (%d bytes)\n\n", filesName[currentFileIndex], filesSize[currentFileIndex]);
    }

}

