#pragma once
#include "model.h"
#include "factor.h"

struct Graph{
	// G[i][j] = bitmask(3) -> [b2 b1 b0]
	// b0 = transition edge
	// b1 = skip edge
	// b2 = pair skip edge
	vector<vector<int> > G;
	int n;
	// key = bitmask(n) -> [bn-1 bn-2 ... b1 b0]
	// (bi==1) => ith node is present
	map<int, Factor> factors;

	void init(const vector<int>& img1, const vector<int>& img2, const Model& model, int type){
		init_graph(img1, img2, type);
		vector<int> img;
		img.insert(img.end(), img1.begin(), img1.end());
		img.insert(img.end(), img2.begin(), img2.end());
		init_factors(img, model, type);
	}
	void init_graph(const vector<int>& img1, const vector<int>& img2, int type){
		int n1 = img1.size(), n2 = img2.size();
		n = img1.size() + img2.size();
		// Generating graph
		G.assign(n, vector<int>(n,0));
		if(type>=2){
			// Transition Factors
			for(int i=0;i<n1-1;i++)SET(G[i][i+1],0),SET(G[i+1][i],0);
			for(int i=0;i<n2-1;i++)SET(G[n1+i][n1+i+1],0),SET(G[n1+i+1][n1+i],0);
		}
		if(type>=3){
			// Skip Factors
			for(int i=0;i<n1;i++)
				for(int j=i+1;j<n1;j++)
					if(img1[i]==img1[j])SET(G[i][j],1), SET(G[j][i],1);
			for(int i=0;i<n2;i++)
				for(int j=i+1;j<n2;j++)
					if(img2[i]==img2[j])SET(G[n1+i][n1+j],1), SET(G[n1+j][n1+i],1);
		}
		if(type>=4){
			for(int i=0;i<n1;i++)
				for(int j=0;j<n2;j++){
					if(img1[i]==img2[j])SET(G[i][n1+j],2), SET(G[n1+j][i],2);
				}
		}
	}
	void init_factors(const vector<int>& img, const Model& model, int type){
		// phi(i) -> corresponding to OCR factors
		Factor phi;
		for(int i=0;i<n;i++){
			phi.init((1<<i));
			for(int c=0;c<10;c++){
				phi.v[c] *= model.ocr[img[i]][c];
			}
			phi.normalize();
			factors[1<<i] = phi;
		}
		// phi(i,j)
		for(int i=0;i<n;i++){
			for(int j=i+1;j<n;j++){
				if(G[i][j]==0)continue;
				phi.init((1<<i)+(1<<j));
				if(BIT(G[i][j], 0)){														// Transition Factor (note importance of ordering)
					for(int c1=0;c1<10;c1++)											// c1 -> assigned to i
						for(int c2=0;c2<10;c2++)										// c2 -> assigned to j = i+1
							phi.v[10*c2+c1] *= model.trans[c1][c2];
				}
				if(BIT(G[i][j], 1)){													// Skip Factor
					for(int c=0;c<10;c++)
						phi.v[10*c + c] *= model.skip;
				}
				if(BIT(G[i][j], 2)){													// Pair-Skip Factor
					for(int c=0;c<10;c++)
						phi.v[10*c + c] *= model.pair_skip;
				}
				phi.normalize();
				factors[(1<<i) + (1<<j)] = phi;
			}
		}
	}
	int key(const vector<int>& x, int mask){
		int i=0, ret=0, pv=1;
		while(mask){
			if(mask&1){
				ret = x[i]*pv + ret;
				pv *= 10;
			}
			i++;
			mask>>=1;
		}
		return ret;
	}
	double factor_product(const vector<int>& x){
		double ret=0;
		for(auto &p:factors){
			ret += log(p.second.v[key(x, p.first)]);
		}
		return exp(ret);
	}
	int sample(int i, vector<int> x){
		vector<double> p(10, 0);
		double sum=0, toss_value = double(rand())/double(INT_MAX);
		for(int j=0;j<10;j++){
			x[i] = j;
			p[j] = factor_product(x);
			sum += p[j];
		}
		for(int j=0;j<10;j++) p[j]/=sum;
		for(int j=1;j<10;j++) p[j]+=p[j-1];
		for(int j=8;j>=0;j--) if(toss_value>=p[j]) return j+1;
		return 0;
	}
	void gibbs_sampler(vector<int>& x,vector<vector<double> >& p){
		vector<vector<int> > cnt(n,vector<int>(10, 0));
		for(int i=0;i<n;i++) x[i]=rand()%10;

		int i=0, itr=1;
		while(itr<=5500){
			x[i] = sample(i, x);
			if(itr>500)					// Burn out
				cnt[i][x[i]]++;
			itr++;
			i=(i+1)%n;
		}
		for(int i=0;i<n;i++){
			x[i]=0;
			for(int j=0;j<10;j++)
				x[i] += cnt[i][j];
			for(int j=0;j<10;j++)
				p[i][j] = double(cnt[i][j])/double(x[i]);
			x[i]=0;
			for(int j=0;j<10;j++)
				if(cnt[i][j]>cnt[i][x[i]])
					x[i]=j;
		}
	}
	void print(){
		printf("Graph:\n");
		for(int i=0;i<n;i++){
			for(int j=0;j<n;j++){
				cout<<G[i][j]<<" ";
			}
			cout<<endl;
		}
	}
};