#pragma once
#include <iostream>

#define IN 
#define OUT

/******************************************************************************
*! @brief  : Read bytes data file
*! @author : thuong.nv - [Date] : 11/11/2022
*! @return : int : nsize / data : buff
******************************************************************************/
int read_data_file(IN const wchar_t* path, OUT void** data)
{
    if (data) *data = NULL;
    int nbytes = 0;
    // open and read file share data
    FILE* file = _wfsopen(path, L"rb", _SH_DENYRD);
    if (!file) return nbytes;

    // read number of bytes file size 
    fseek(file, 0L, SEEK_END);
    nbytes = static_cast<int>(ftell(file));
    fseek(file, 0L, SEEK_SET);

    // read data form file to memory
    auto tbuff = new unsigned char[nbytes + 2];
    memset(tbuff, 0, (nbytes + 2));
    nbytes = (int)fread_s(tbuff, nbytes, sizeof(char), nbytes, file);
    fclose(file);

    // Read content bytes file + nbyte read
    if (data) *data = tbuff;
    else delete[] tbuff;
    return nbytes;
}