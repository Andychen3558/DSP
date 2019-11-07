#include "hmm.h"
#include <math.h>
#define Train_num 10000
#define T 50

int main(int argc, char *argv[])
{
	int ite = atoi(argv[1]);
	HMM hmm;
	loadHMM(&hmm, argv[2]);

	FILE *fp = open_or_die(argv[3], "r");
	int training[Train_num][T];
	for(int i=0;i<Train_num;i++) {
		for(int j=0;j<T;j++) {
			char tmp;
			fscanf(fp, "%c", &tmp);
			training[i][j] = tmp - 'A';
			if(j == T-1) fseek(fp, 1, SEEK_CUR);
		}
	}

	fclose(fp);

	double alpha[T][hmm.state_num];
	double beta[T][hmm.state_num];
	double gamma[T][hmm.state_num], gamma_sum[T][hmm.state_num];
	double epsilon[T-1][hmm.state_num][hmm.state_num], epsilon_sum[T-1][hmm.state_num][hmm.state_num];
	double update_observation[hmm.observ_num][hmm.state_num];

	while(ite--) {
		memset(gamma_sum, 0, T * hmm.state_num * sizeof(double));
		memset(epsilon_sum, 0, T * hmm.state_num * hmm.state_num * sizeof(double));
		memset(update_observation, 0, hmm.observ_num * hmm.state_num * sizeof(double));

		for(int index=0;index<Train_num;index++) {
			/* forward-algorithm */
			for(int i=0;i<hmm.state_num;i++)
				alpha[0][i] = hmm.initial[i] * hmm.observation[training[index][0]][i];
			for(int i=0;i<T-1;i++) {
				for(int j=0;j<hmm.state_num;j++) {
					alpha[i+1][j] = 0;
					for(int k=0;k<hmm.state_num;k++) {
						alpha[i+1][j] += alpha[i][k] * hmm.transition[k][j];
					}
					alpha[i+1][j] *= hmm.observation[training[index][i+1]][j];
				}
			}

			/* backward-algorithm */
			for(int i=0;i<hmm.state_num;i++)
				beta[T-1][i] = 1.0;
			for(int i=T-2;i>=0;i--) {
				for(int j=0;j<hmm.state_num;j++) {
					beta[i][j] = 0;
					for(int k=0;k<hmm.state_num;k++) {
						beta[i][j] += beta[i+1][k] * hmm.transition[j][k] * hmm.observation[training[index][i+1]][k];
					}
				}
			}

			/* calculate gamma */
			double P1[T];
			for(int i=0;i<T;i++) {
				P1[i] = 0;
				for(int j=0;j<hmm.state_num;j++) {
					P1[i] += alpha[i][j] * beta[i][j];
				}
			}
			for(int i=0;i<T;i++) {
				for(int j=0;j<hmm.state_num;j++) {
					gamma[i][j] = alpha[i][j] * beta[i][j] / P1[i];
					update_observation[training[index][i]][j] += gamma[i][j];
					gamma_sum[i][j] += gamma[i][j];
				}
			}
			
			/* calculate epsilon */
			double P2[T];
			for(int i=0;i<T-1;i++) {
				P2[i] = 0;
				for(int j=0;j<hmm.state_num;j++) {
					for(int k=0;k<hmm.state_num;k++) {
						P2[i] += alpha[i][j] * hmm.transition[j][k] * hmm.observation[training[index][i+1]][k] * beta[i+1][k];
					}
				}
			}
			for(int i=0;i<T-1;i++) {
				for(int j=0;j<hmm.state_num;j++) {
					for(int k=0;k<hmm.state_num;k++) {
						epsilon[i][j][k] = alpha[i][j] * hmm.transition[j][k] * hmm.observation[training[index][i+1]][k] * beta[i+1][k] / P2[i];
						epsilon_sum[i][j][k] += epsilon[i][j][k];
					}
				}
			}
		}

		/* update params */
		// pi
		for(int i=0;i<hmm.state_num;i++) {
			hmm.initial[i] = gamma_sum[0][i] / Train_num;
		}

		// A
		for(int i=0;i<hmm.state_num;i++) {
			for(int j=0;j<hmm.state_num;j++) {
				double divisor = 0, dividend = 0;
				for(int t=0;t<T-1;t++) {
					divisor += epsilon_sum[t][i][j];
					dividend += gamma_sum[t][i];
				}
				hmm.transition[i][j] = divisor / dividend;
			}
		}

		// B
		for(int i=0;i<hmm.state_num;i++) {
			double dividend = 0;
			for(int t=0;t<T;t++)
				dividend += gamma_sum[t][i];
			for(int j=0;j<hmm.observ_num;j++)
				hmm.observation[j][i] = update_observation[j][i] /= dividend;
		}
	}

	FILE *model = open_or_die(argv[4], "w");
	dumpHMM(model, &hmm);
	fclose(model);

	return 0;
}
