#include "hmm.h"
#include <math.h>
#define Model_num 5
#define Test_num 2500
#define T 50

int main(int argc, char *argv[])
{
	HMM hmm[Model_num];
	load_models(argv[1], hmm, Model_num);

	FILE *fp = open_or_die(argv[2], "r");
	int testing[Test_num][T];

	for(int i=0;i<Test_num;i++) {
		for(int j=0;j<T;j++) {
			char tmp;
			fscanf(fp, "%c", &tmp);
			testing[i][j] = tmp - 'A';
			if(j == T-1) fseek(fp, 1, SEEK_CUR);
		}
	}

	fclose(fp);

	FILE *result = open_or_die(argv[3], "w");
	int state_num = hmm[0].state_num;
	double delta[T][state_num];

	for(int index=0;index<Test_num;index++) {
		double P_opt = 0;
		int model_opt = -1;
		for(int m=0;m<Model_num;m++) {
			double P_tmp = 0;
			for(int i=0;i<state_num;i++)
				delta[0][i] = hmm[m].initial[i] * hmm[m].observation[testing[index][0]][i];
			for(int t=1;t<T;t++) {
				for(int j=0;j<state_num;j++) {
					double max = 0;
					for(int i=0;i<state_num;i++) {
						if(delta[t-1][i] * hmm[m].transition[i][j] > max)
							max = delta[t-1][i] * hmm[m].transition[i][j];
					}
					delta[t][j] = max * hmm[m].observation[testing[index][t]][j];

					if(t==T-1 && delta[t][j]>P_tmp) {
						P_tmp = delta[t][j];
					}
				}
			}
			if(P_tmp>P_opt) {
				P_opt = P_tmp;
				model_opt = m;
			}
		}

		/* write result */
		char* ans[5] = {"model_01.txt", "model_02.txt", "model_03.txt", "model_04.txt", "model_05.txt"};
		fprintf(result, "%s %e\n", ans[model_opt], P_opt);
	}

	fclose(result);

	return 0;
}