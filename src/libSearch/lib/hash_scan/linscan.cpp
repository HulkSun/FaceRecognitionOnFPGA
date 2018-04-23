#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <time.h>
#include <fstream>

#include "bitops.h"
#include "types.h"
#include "linscan.h"

using namespace std;

Linscan::Linscan(int b, int k, int d, int dayn)
{
	B = b;
	B_over_8 = b / 8;
	K = k;
	D = d;

	assign_codes(dayn);
}

Linscan::~Linscan()
{
	for (std::size_t i = 0; i < data_codes.size(); i++)
	{
		for (std::size_t j = 0; j < data_codes[i].size(); j++)
		{
			free(data_codes[i][j]);
		}
		data_codes[i].clear();
	}
	data_codes.clear();
}

void Linscan::assign_codes(int n)
{
	std::vector<UINT8 *> v;
	data_codes.assign(n + 1, v);
}

void Linscan::query(const std::vector<UINT8 *> &codes, UINT8 *query, UINT8 **results, int day)
{
	//std::ofstream out("linscan_query.log");
	//out << "linscan codes_size = " << codes.size() << std::endl;
	// clock_t t1 = clock();
	UINT32 *numres = (UINT32 *)malloc(sizeof(UINT32) * (B + 1));
	UINT32 *res = (UINT32 *)malloc(sizeof(UINT32) * (K * (B + 1)));
	// clock_t t2 = clock();
	//out<<" linscan malloc_time " << (t2- t1)*1.0/1000<<std::endl;
	results[0] = NULL;

	if (numres == NULL)
		return;

	if (res == NULL)
	{
		free(numres);
		return;
	}

	memset(numres, 0, (B + 1) * sizeof(UINT32));

	// t1 = clock();
	for (size_t j = 0; j < codes.size(); j++)
	{
		int h = match(codes[j], query, B_over_8);
		if (h > D || h > B || h < 0)
		{
			continue;
		}
		if (numres[h] < K)
		{
			res[K * h + numres[h]] = j;
			numres[h]++;
		}
	}
	// t2 = clock();
	//out<<" linscan match_time " << (t2- t1)*1.0/1000<<std::endl;

	int index = 0;
	int n = 0;
	// t1 = clock();
	for (int s = 0; s <= D && n < K; s++)
	{
		for (int c = 0; c < numres[s] && n < K; c++)
		{
			index = res[s * K + c];
			results[n++] = codes[index];
		}
	}

	if (n != K)
	{
		results[n] = NULL;
	}

	// t2 = clock();
	//out<<" linscan set_result_time " << (t2- t1)*1.0/1000<<std::endl;
	free(numres);
	free(res);
	//out.close();
}

void Linscan::query(UINT8 *que, UINT8 **result, int day)
{
	query(data_codes[day], que, result, day);
}

void Linscan::add_codes(UINT8 *codes, int day)
{
	data_codes[day].push_back(codes);
}

void Linscan::clear_codes(int day)
{
	data_codes[day].clear();
}

void Linscan::move_codes(int rec, int s, int e)
{
	int n = data_codes.size();
	if (rec < 0)
	{
		e = (e < n ? e : n);
		for (int i = s; i < e; i++)
		{
			if (i + rec > 0)
			{
				data_codes[i + rec].clear();
				data_codes[i + rec] = data_codes[i];
			}
		}
	}
	else if (rec > 0)
	{
		s = (s > 0 ? s : 0);
		for (int i = e - 1; i >= s; i--)
		{
			if (i + rec < n)
			{
				data_codes[i + rec].clear();
				data_codes[i + rec] = data_codes[i];
			}
		}
	}
}

int Linscan::codes_size()
{
	return data_codes.size();
}

void Linscan::delete_all()
{
	for (size_t i = 0; i < data_codes.size(); i++)
	{
		for (size_t j = 0; j < data_codes[i].size(); j++)
		{
			free(data_codes[i][j]);
		}
		data_codes[i].clear();
	}
	data_codes.clear();
}
