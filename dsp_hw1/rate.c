#include "hmm.h"
#include <string.h>
#define Test_num 2500

int main()
{
	FILE *fp1, *fp2, *acc;
	fp1 = open_or_die("testing_answer.txt", "r");
	fp2 = open_or_die("result1.txt", "r");
	double accurate = 0;
	char ans1[20], ans2[20], rate[20];

	for(int i=0;i<Test_num;i++) {
		fscanf(fp1, "%s", ans1);
		fscanf(fp2, "%s%s", ans2, rate);
		if(strcmp(ans1, ans2) == 0)
			accurate++;
	}

	acc = fopen("acc.txt", "w");
	fprintf(acc, "%lf", accurate/Test_num);
}