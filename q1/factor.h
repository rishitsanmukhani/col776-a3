#pragma once
#include "common.h"
const int p[9] = {1, int(1e1), int(1e2), int(1e3), int(1e4), int(1e5), int(1e6), int(1e7), int(1e8)};

struct Factor{
	int mask;
	vector<double> v;
	void init(int _mask=0, double _val = 1.0){
		mask = _mask;
		v.assign(p[CNT(mask)], _val);
	}
	// (key, inp_mask = 008109, 1100110011)
	// (ret, out_mask = 		09, 1000000001)
	int get_key(int key, int out_mask)const{
		int ret=0, place_val = 1;
		int inp_mask = mask;
		while(out_mask){
			if(out_mask&1){
				ret = (key%10)*place_val + ret;
				place_val *= 10;
			}
			if(inp_mask&1){
				key /= 10;
			}
			inp_mask >>= 1;
			out_mask >>= 1;
		}
		return ret;
	}
	Factor get_factor(int out_mask)const{
		assert(out_mask == (out_mask & mask));	// Checking out_mask is subset of mask
		Factor ret;
		ret.init(out_mask, 0);
		for(int i=0;i<v.size();i++){
			ret.v[get_key(i, out_mask)] += v[i];
		}
		return ret;
	}
	Factor operator*(const Factor& rhs)const{
		int m = mask | rhs.mask;
		Factor ret;
		ret.init(m);
		for(int i=0;i<ret.v.size();i++){
			ret.v[i] *= v[ret.get_key(i, mask)];
			ret.v[i] *= rhs.v[ret.get_key(i, rhs.mask)];
		}
		ret.normalize();
		return ret;
	}
	void normalize(){
		double m=0;
		for(auto val:v)m = max(m, val);
		if(m>0)for(auto& val:v)val/=m;
	}
	void print(){
		printf("Mask: %d\n", mask);
		for(int i=0;i<v.size();i++){
			printf("%d %f\n", i, v[i]);
		}
	}
};