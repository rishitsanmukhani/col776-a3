#include "word.h"

Model model;
vector<Word> words;
int parse_input(string ocr_filename, string word_filename){
	FILE* file1 = fopen(ocr_filename.c_str(), "r");
	FILE* file2 = fopen(word_filename.c_str(), "r");
	if(!file1 || !file2){
		return EXIT_FAILURE;
	}
	Word w;
	while(fscanf(file2, " %s ", w.w1)!=EOF){
		fscanf(file2, " %s ", w.w2);
		w.init();
		for(int i=0;i<w.n1;i++)
			fscanf(file1,"%d", &w.img1[i]);
		for(int i=0;i<w.n2;i++)
			fscanf(file1,"%d", &w.img2[i]);
		w.convert_to_int(model.char_to_id);
		words.push_back(w);
	}
	fclose(file2);
	fclose(file1);
	return EXIT_SUCCESS;
}

int main(int argc, char** argv){
	srand(time(NULL));
	// Input
	if(argc!=5){
		cout<<"./a.out <trans.dat> <ocr.dat> <data.dat> <truth.dat>"<<endl;
		return EXIT_FAILURE;
	}
	// Model
	if(model.init(string(argv[1]), string(argv[2]))){
		return EXIT_FAILURE;
	}
	// Words
	if(parse_input(string(argv[3]), string(argv[4]))){
		return EXIT_FAILURE;
	}
	// Generating graph w.r.t model-type (1, 2, 3, 4)
	int type=4;
	double dll=0, T=0;
	int nc=0, tc=0, nw=0, tw=0;
	for(auto& word:words){
		word.init_graph(model, type);
		dll += word.ll;
		nc += word.nc, tc += word.n;
		nw += (word.nc1==word.n1), nw += (word.nc2==word.n2), tw += 2;
		T += word.T;
	}
	printf("Ch-Acc: %f (%d/%d)\n", double(nc)/tc, nc, tc);
	printf("Wd-Acc: %f (%d/%d)\n", double(nw)/tw, nw, tw);
	printf("LL    : %f\n", dll/tw);
	printf("Time  : %f s\n", T/1000);
	printf("\n");
	return 0;
}