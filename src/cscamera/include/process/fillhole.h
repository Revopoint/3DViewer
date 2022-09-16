/*******************************************************************************
* This file is part of the 3DViewer
*
* Copyright 2022-2026 (C) Revopoint3D AS
* All rights reserved.
*
* Revopoint3D Software License, v1.0
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistribution of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
*
* 2. Redistribution in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. Neither the name of Revopoint3D AS nor the names of its contributors may be used
* to endorse or promote products derived from this software without specific
* prior written permission.
*
* 4. This software, with or without modification, must not be used with any
* other 3D camera than from Revopoint3D AS.
*
* 5. Any software provided in binary form under this license must not be
* reverse engineered, decompiled, modified and/or disassembled.
*
* THIS SOFTWARE IS PROVIDED BY REVOPOINT3D AS "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL REVOPOINT3D AS OR CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* Info:  https://www.revopoint3d.com
******************************************************************************/

#ifndef _FILLHOLE_H_
#define _FILLHOLE_H_
#include <vector>

struct  _block
{
    int  start;
    int  end;
    int  num;
    int     flag;
};

template<class T>
void fillRow(T*  mask, int width, std::vector<struct  _block> *  row_cluster)
{
    bool  isStart = false;
    struct  _block    res;
    res.flag = 0;
    for (int i = 0; i < width; i++)
    {
        if (mask[i] != 0)
        {
            if (!isStart)
            {
                isStart = true;
                res.start = i;
            }
        }
        else
        {
            if (isStart)
            {
                res.end = i - 1;
                res.num = res.end - res.start + 1;
                row_cluster->push_back(res);
                isStart = false;
            }
        }
    }

    if (isStart)
    {
        res.end = width - 1;
        res.num = res.end - res.start + 1;
        row_cluster->push_back(res);
    }
}

bool  isConnect(struct  _block  block1, struct  _block  block2)
{
    int max = (block1.end > block2.end) ? block1.end : block2.end;//std::max(block1.end, block2.end);
    int min = (block1.start < block2.start) ? block1.start : block2.start;//std::min(block1.start, block2.start);
    if ((max - min + 1) < (block1.num + block2.num))
        return  true;
    else
        return  false;
}


template<class T>
void depthFillHole(T* depth, int width, int height)    //????
{
    //cv::Mat  mask = cv::Mat::zeros(height, width, CV_16SC1);
    T* ptr_depth = depth;
    T* ptr_mask = (T*)malloc(height * width * sizeof(T)); //(short*)mask.data;
    memset(ptr_mask, 0, height * width * sizeof(T));

#pragma omp parallel for
    for (int i = 0; i < height * width; i++)
    {
        if (fabs(ptr_depth[i]) < 1e-6)
            ptr_mask[i] = -1;
    }

    int   label = 1;
    std::vector<std::vector<struct  _block> *>    row_clusters;

    for (int i = 0; i < height; i++)
    {
        std::vector<struct  _block> *tmp = new  std::vector<struct  _block>();
        row_clusters.push_back(tmp);
    }

    //??????§Ö???????
#pragma omp parallel for
    for (int i = 0; i < height; i++)
        fillRow<T>(ptr_mask + i * width, width, row_clusters[i]);

    //?????????
    for (int i = 0; i < row_clusters[0]->size(); i++)
    {
        for (int j = row_clusters[0]->at(i).start; j <= row_clusters[0]->at(i).end; j++)
        {
            row_clusters[0]->at(i).flag = label;
        }
        label++;
    }
    for (int i = 1; i < height; i++)
    {
        T* tmp_mask_pre = ptr_mask + (i - 1) * width;
        T* tmp_mask_next = ptr_mask + i * width;
        for (int t = 0; t < row_clusters[i]->size(); t++)
        {
            std::vector<struct  _block> *cur = row_clusters[i];
            std::vector<struct  _block> *pre = row_clusters[i - 1];

            bool  flag1 = true;
            for (int j = 0; j < pre->size(); j++)
            {
                if (isConnect(cur->at(t), pre->at(j)))
                {
                    flag1 = false;
                    if (cur->at(t).flag == 0)
                        cur->at(t).flag = pre->at(j).flag;
                    else
                    {
                        int  tmp_flag = pre->at(j).flag;

                        for (int n = i - 1; n >= 0; n--)
                        {
                            std::vector<struct  _block> *cur1 = row_clusters[n];

                            bool  flag = true;
                            for (int m = 0; m < cur1->size(); m++)
                            {
                                if (cur1->at(m).flag == tmp_flag)
                                {
                                    cur1->at(m).flag = cur->at(t).flag;
                                    flag = false;
                                }
                            }
                            if (flag)
                                break;
                        }
                    }
                }
            }

            if (flag1)
            {
                cur->at(t).flag = label++;
            }
        }
    }


#pragma omp parallel for    
    for (int i = 1; i < label; i++)
    {
        std::vector<std::pair<int, int>>   clusters;
        bool  isStart = false;
        for (int y = 0; y < row_clusters.size(); y++)
        {
            bool  isHave = false;
            for (int x = 0; x < row_clusters[y]->size(); x++)
            {
                if (row_clusters[y]->at(x).flag == i)
                {
                    isStart = true;

                    if (((row_clusters[y]->at(x).start == 0) || (row_clusters[y]->at(x).end == (width - 1))) ||
                        ((y == 0) || (y == (height - 1))))
                    {
                        isHave = false;
                        clusters.clear();
                        break;
                    }
                    else
                    {
                        clusters.push_back(std::pair<int, int>(y, x));
                        isHave = true;
                    }

                }
            }

            if (isStart && (!isHave))
            {
                if (clusters.size() > 0)
                {
                    int  num = 0;
                    for (int k = 0; k < clusters.size(); k++)
                        num += row_clusters[clusters[k].first]->at(clusters[k].second).num;

                    if (num < 200)
                    {

                        //printf("................................%d\n", i);

                        int  max_v = -1;
                        int  min_v = 100000;
                        for (int k = 0; k < clusters.size(); k++)
                        {
                            int  row = clusters[k].first;
                            int  start_c = row_clusters[row]->at(clusters[k].second).start;
                            int  end_c = row_clusters[row]->at(clusters[k].second).end;

                            int  tmp = row * width;
                            T z1 = depth[tmp + start_c - 1];
                            T z2 = depth[tmp + end_c + 1];
                            //printf("row:%d, start:%d,end:%d,z1:%d,z2:%d\n", row, start_c, end_c,z1,z2);

                            if (z1 > max_v)  max_v = z1;
                            if (z1 < min_v)   min_v = z1;
                            if (z2 > max_v)  max_v = z2;
                            if (z2 < min_v)   min_v = z2;
                            for (int p = 0; p < 2; p++)
                            {
                                int tmp1 = tmp;
                                if (p == 0) tmp1 -= width;
                                else    tmp1 += width;
                                for (int k = start_c; k <= end_c; k++)
                                {
                                    int depth1 = depth[tmp1 + k];
                                    if (depth1 != 0)
                                    {
                                        if (depth1 > max_v)  max_v = depth1;
                                        if (depth1 < min_v)   min_v = depth1;
                                    }
                                }

                            }
                        }

                        if ((max_v - min_v) > 500)
                            continue;


                        for (int k = 0; k < clusters.size(); k++)
                        {
                            int  row = clusters[k].first;
                            int  start_c = row_clusters[row]->at(clusters[k].second).start;
                            int  end_c = row_clusters[row]->at(clusters[k].second).end;

                            int  tmp = row * width;
                            T z1 = depth[tmp + start_c - 1];
                            T z2 = depth[tmp + end_c + 1];
                            //printf("row:%d, start:%d,end:%d,z1:%d,z2:%d\n", row, start_c, end_c,z1,z2);

                            if ((z1 != 0) || (z2 != 0))
                            {
                                for (int k = start_c; k <= end_c; k++)
                                {
                                    if (depth[tmp + k] == 0)
                                    {
                                        depth[tmp + k] = (float)(z2 - z1) * (float)(k - start_c + 1) / (float)(end_c - start_c + 2) + z1;
                                    }
                                }
                            }
                        }
                    }
                }


                break;
            }
        }
    }


    for (int i = 0; i < height; i++)
    {
        row_clusters[i]->clear();
        delete  row_clusters[i];
    }
    row_clusters.clear();
    free(ptr_mask);

}

#endif // !_FILLHOLE_H_
