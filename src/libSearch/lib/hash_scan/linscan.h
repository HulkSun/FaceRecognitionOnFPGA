#ifndef __LINSCAN_H
#define __LINSCAN_H

#include <vector>
#include "types.h"

class Linscan{

public:
	Linscan(int b, int k, int d, int dayn);
	~Linscan();

	void query(UINT8 *query, UINT8** result, int day);

	void add_codes(UINT8 *codes, int day);
	void clear_codes(int day);
	void move_codes(int rec, int s, int e);
	void delete_all();

	int codes_size();

private:
	int B;			
    int B_over_8;
    int D;
    int K;
    int m;

	std::vector<std::vector<UINT8 *> > data_codes;

	void query(const std::vector<UINT8 *>& codes, UINT8 *query, UINT8** result, int day);
	void assign_codes(int n);
	
};

#endif
