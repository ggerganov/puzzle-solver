/*! \file CommonPoints.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "Types.h"
#include "Functions.h"

#include <fstream>

template <>
bool LoadFromFile::operator()<CommonPoints>(CommonPoints & obj, const char * fname) {
    std::ifstream fin(fname);
    if (fin.good() == false) {
        return false;
    }

    obj.clear();

    {
        int n = 0;
        fin >> n;
        obj.resize(n);
    }
    for (auto & p : obj) {
        int n = 0;
        fin >> n;
        for (int i = 0; i < n; ++i) {
            int id;
            fin >> id;
            Point2D c;
            fin >> c.x >> c.y;
            p.posInImage[id] = c;
        }
    }

    return true;
}

template <>
bool SaveToFile::operator()<CommonPoints>(const CommonPoints & obj, const char * fname) {
    std::ofstream fout(fname);
    fout << obj.size() << std::endl;
    for (auto & p : obj) {
        fout << p.posInImage.size() << std::endl;
        for (auto & c : p.posInImage) {
            fout << c.first << " " << c.second.x << " " << c.second.y << std::endl;
        }
    }

    return true;
}

template <>
int32_t Count::operator()<CommonPoints>(const CommonPoints & obj) {
    return obj.size();
}
