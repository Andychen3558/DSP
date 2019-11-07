#include <cstdio>
#include <cstring>
#include <fstream>
#include "Ngram.h"

int main(int argc, char* argv[])
{
 	fstream ftext, fmap;
	int ngram_order;
	char buffer[5000], phone[40][2000][3];
	int isPhonetic = 0, cur = -1, count[40]={0};

	if(strcmp(argv[1], "-text")==0) ftext.open(argv[2], ios::in);
	if(strcmp(argv[3], "-map")==0) fmap.open(argv[4], ios::in);
	if(strcmp(argv[7], "-order")==0) ngram_order = atoi(argv[8]);

    Vocab voc;
    Ngram lm( voc, ngram_order );
    
	{
		if(strcmp(argv[5], "-lm")==0) 
		{
			File lmFile( argv[6], "r" );
       	 	lm.read(lmFile);
        	lmFile.close();
		}
    }

	int maxCount = 0, tmpCount = 0;
	while(fmap.getline(buffer, sizeof(buffer))) {
		if(buffer[0]==-93)
		{
			if(tmpCount > maxCount)
			{
				maxCount = tmpCount;
			}
			isPhonetic = 1;
			cur++;
			tmpCount = 0;
		}
		else
		{
			isPhonetic = 0;
			tmpCount++;
			char word[3];
			memcpy(word, &buffer[0], 2);
			word[2] = 0;
			strcpy(phone[cur][count[cur]++], word);
		}
	}
	fmap.close();
	
	while(ftext.getline(buffer, sizeof(buffer))) {
		double delta[200][maxCount]={0};
		int backtrack[200][maxCount];
		char words[200][maxCount][5];
		char word[3]; //word which is not <s> or </s>
		int n = strlen(buffer), word_index = 0, state_num[200] = {0};
		
		VocabIndex wid;
		VocabIndex context[2];
		context[0] = voc.getIndex("<s>");
		context[1] = Vocab_None;

		VocabIndex last_wid[200][maxCount];

		for(int i=0;i<n;i++) {
			if(buffer[i]!=' ' && buffer[i]!='\n')
			{
				if(buffer[i]!=-93) // 國字
				{
					memcpy(word, &buffer[i], 2);
					word[2] = 0;
					wid = voc.getIndex(word);
					if(wid==Vocab_None)
					{
						wid = voc.getIndex(Vocab_Unknown);
					}

					if(word_index==0)
					{
						delta[word_index][0] = lm.wordProb(wid, context);
						strcpy(words[word_index][0], "<s>");
						words[word_index][0][3] = 0;
						state_num[word_index] = 1;
						last_wid[word_index+1][0] = voc.getIndex("<s>");
						word_index++;
					}

					memcpy(words[word_index][0], &buffer[i], 2);
					words[word_index][0][2] = 0;
					state_num[word_index] = 1;
					
					double max = -10000;
					int index;
					//printf("%s\n", word);
					for(int j=0;j<state_num[word_index-1];j++) {
						context[0] = last_wid[word_index][j];
						context[1] = Vocab_None;
						if(delta[word_index-1][j]+lm.wordProb(wid, context) > max)
						{
							max = delta[word_index-1][j]+lm.wordProb(wid, context);
							index = j;
						}
					}
					delta[word_index][0] = max;
					backtrack[word_index][0] = index;
					last_wid[word_index+1][0] = wid;
				}
				else // 注音
				{
					int phonetic = buffer[i+1]>0 ? buffer[i+1]-116:buffer[i+1]+106;
				
					for(int j=0;j<count[phonetic];j++) {
						memcpy(word, &phone[phonetic][j], 2);
						word[2] = 0;
						wid = voc.getIndex(word);
						if(wid==Vocab_None)
						{
							wid = voc.getIndex(Vocab_Unknown);
						}

						if(word_index==0)
						{
							delta[word_index][j] = lm.wordProb(wid, context);
							//printf("delta:%lf\n", delta[word_index][j]);
							strcpy(words[word_index][j], "<s>");
							words[word_index][j][3] = 0;
							state_num[word_index] = 1;
							last_wid[word_index+1][j] = voc.getIndex("<s>");
						}
						
						int tmp = word_index==0 ? word_index+1:word_index;
						memcpy(words[tmp][j], &phone[phonetic][j], 2);
						words[tmp][j][2] = 0;
					}
					if(word_index==0)
						word_index++;
					state_num[word_index] = count[phonetic];
					
					for(int j=0;j<state_num[word_index];j++) {
						memcpy(word, &phone[phonetic][j], 2);
						word[2] = 0;
						wid = voc.getIndex(word);
						if(wid==Vocab_None)
						{
							wid = voc.getIndex(Vocab_Unknown);
						}

						double max = -10000;
						int index;
						
						for(int k=0;k<state_num[word_index-1];k++) {
							context[0] = last_wid[word_index][k];
							context[1] = Vocab_None;
							if(delta[word_index-1][k]+lm.wordProb(wid, context) > max)
							{
								max = delta[word_index-1][k]+lm.wordProb(wid, context);
								index = k;
							}
						}
						delta[word_index][j] = max;
						backtrack[word_index][j] = index;
						last_wid[word_index+1][j] = wid;
					}
				}
				i++;
				word_index++;
			}
		}

		// deal with </s>
		double max = -10000;
		int index;
		wid = voc.getIndex("</s>");
		for(int j=0;j<state_num[word_index-1];j++) {
			context[0] = last_wid[word_index][j];
			context[1] = Vocab_None;
			if(delta[word_index-1][j]+lm.wordProb(wid, context) > max)
			{
				max = delta[word_index-1][j]+lm.wordProb(wid, context);
				index = j;
			}
		}
		backtrack[word_index][0] = index;
		strcpy(words[word_index][0], "</s>");
		words[word_index][0][4] = 0;
		state_num[word_index] = 1;

		// backtracking
		char answer[200][5];
		int ansCount = 0, cur = 0;
		while(word_index>=0) {
			strcpy(answer[ansCount], words[word_index][cur]);
			int end;
			if(strcmp(answer[ansCount], "<s>")==0)
				end = 3;
			else if(strcmp(answer[ansCount], "</s>")==0)
				end = 4;
			else
				end = 2;
			answer[ansCount][end] = 0;

			cur = backtrack[word_index][cur];
			ansCount++;
			word_index--;
		}

		// printing
		for(int i=ansCount-1;i>0;i--)
			printf("%s ", answer[i]);
		printf("%s\n", answer[0]);
	}

	ftext.close();
	return 0;
}
