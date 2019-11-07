#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
using namespace std;

int main(int argc, char* argv[])
{
	fstream fin, fout;
	fin.open(argv[1], ios::in);
	if(!fin)
		return 0;
	fout.open(argv[2], ios::out);
	if(!fout)
		return 0;
	char buffer[50];
	char phonetic[40][13010][3];
	int count[40];

	for(int i=0;i<37;i++) {	
		phonetic[i][0][0] = -93;
		phonetic[i][0][1] = i<=10 ? i+116:i-106;
		phonetic[i][0][2] = 0;
		count[i] = 1;
	}

	while(fin.getline(buffer, sizeof(buffer))) {
		int n = strlen(buffer);
		char word[3], lastPhone[5], lCount = 0;
		memset(word, '\0', sizeof(word));
		memset(lastPhone, '\0', sizeof(lastPhone));
		for(int i=0;i<n;i++) {
			if(i==0) {
				strncpy(word, buffer, 2);
				word[2] = 0;
				i++;
			}
			else if(buffer[i]<0) {
				if((buffer[i-1]==32 || buffer[i-1]==47) && buffer[i]==-93) {
					char tmp[3];
					memcpy(tmp, &buffer[i], 2);
					tmp[2] = 0;
					int flag = 0;
					for(int i=0;i<lCount;i++) {
						if(tmp[1]==lastPhone[i])
							flag = 1;
					}
					if(!flag) {
						int cur = tmp[1]>0 ? tmp[1]-116:tmp[1]+106;
						strcpy(phonetic[cur][count[cur]], word);
						phonetic[cur][count[cur]][2] = 0;
						count[cur]++;
						lastPhone[lCount++] = tmp[1];
					}
				}
				i++;
			}
		}
	}

	for(int i=0;i<37;i++) {
		fout<<phonetic[i][0]<<"	";
		for(int j=1;j<count[i]-1;j++) {
			fout<<phonetic[i][j]<<" ";
		}
		fout<<phonetic[i][count[i]-1]<<endl;
		for(int j=1;j<count[i];j++) {
			fout<<phonetic[i][j]<<"	"<<phonetic[i][j]<<endl;
		}
	}

	fin.close();
	fout.close();
	return 0;
}
