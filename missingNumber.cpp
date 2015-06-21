/*
	정수 1억개가 주어졌을 때 1부터 시작하여 비어있는 수를 구하라. 
	단, 메모리는 1MB 만 사용 가능하다.

*/

#include<iostream>
#include<time.h>
#include<algorithm>
#include<vector>
#include<map>

using namespace std;

enum{
	/* 정수 하나를 4byte로 계산하면 적어도 382개 파일 필요 
	   1억 : 1MB = 150000 : X 에서 X = 1572byte 
	   1572byte면 393개씩 담을 수 있음             */
	NUMBER_RANGE = 150000, // 1~(N-1)까지 숫자, 101이면 1~100까지의 숫자
	BUFFER_SIZE =  393 // 버퍼크기 = 메모리크기(1572byte) / 정수하나 크기(4byte)
};

int usedMem = 0;

void createTestFile(const char* fileName);
void makeRandomAray(vector<int>& aray);
void writeNumAray(const vector<int>& aray, int size, FILE* fp);
void readNumAray(vector<int>& buff, FILE* fp);
int getNextNumOfFile(FILE* fp);
int externalSort(const char* fileName);
void mergeFromFile(vector<int>& mergeAray, const vector<FILE*>& fp);
int findMinIndexOfAray(const vector<int>& numAray, const vector<bool>& isEOF);
int findMissingNumber(const char* fileName);
void printAray(int* aray, int size);
void fileOpenFail();
int calcMaxNumLength();
void memoryCheck();

int main()
{
	char* fileName = "TestFile.txt";
	
	// createTestFile(fileName);

	cout << "Missing Number is " << findMissingNumber(fileName) << endl;

	return 0;
}

void createTestFile(const char* fileName)
{
	vector<int> randomAray = vector<int>(NUMBER_RANGE, 0);

	makeRandomAray(randomAray);

	FILE* fp = fopen(fileName, "w");
	if(fp == NULL)
		fileOpenFail();
	
	writeNumAray(randomAray, NUMBER_RANGE-1, fp);

	fclose(fp);
}
void makeRandomAray(vector<int>& aray)
{
	int size = aray.size();

	srand((unsigned)time(NULL));

	int missingNum = rand() % size + 1;

	//bool dupleFlag[NUMBER_RANGE] = {false,};
	//map<int, bool> dupleCheck;

	//int i=0;
	//for(i=0; i<size; i++){
	//	aray[i] = rand() % size + 1; // 1~NUMBER_RANGE
	//	while(dupleCheck[aray[i]])
	//		aray[i] = rand() % size + 1;
	//	dupleCheck[aray[i]] = true;
	//	cout << i+1 << endl;
	//}
	bool aparance = false;
	for(int i=0; i<size; i++){
		if(i+1 == missingNum) aparance = true;
		if(!aparance) aray[i] = i+1;
		else aray[i] = i+2;
	}
}
void writeNumAray(const vector<int>& aray, int size, FILE* fp)
{
	for(int i=0; i<size; i++)
	{
		if(aray[i] == EOF) continue; // 외부로 파일 쪼갤 때 정렬된 배열이 들어오므로(EOF(-1)이 앞에 위치)
		char* num = new char[calcMaxNumLength()+2](); // 최대 자리수 + \n + \0
		sprintf(num, "%d\n", aray[i]);
		int j=0;
		while(num[j])
		{
			fwrite(num+j, sizeof(char), 1, fp);
			j++;
		}
		delete []num;
	}
}
void readNumAray(vector<int>& buff, FILE* fp)
{
	for(int i=0; i<buff.size(); i++)
		buff[i] = getNextNumOfFile(fp);
}
int getNextNumOfFile(FILE* fp)
{
	char* lineBuf = new char[calcMaxNumLength()+1](); // 최고 자릿수 + \0
	int bufIndex = 0;

	while(1)
	{
		char chBuf[2] = {0,};
		fread(chBuf, sizeof(char), 1, fp);
		if(chBuf[0] == '\0') return EOF;
		if(chBuf[0] == '\n') break;
		lineBuf[bufIndex++] = chBuf[0];
	}
	
	int result = atoi(lineBuf);
	delete[] lineBuf;

	return result;
}
int externalSort(const char* fileName)
{
	FILE* readFp = fopen(fileName, "r");
	if(readFp == NULL)
		fileOpenFail();

	int fileCount = 0;

	int i=0;
	while(1)
	{
		vector<int> buff = vector<int>(BUFFER_SIZE, 0);
		
		readNumAray(buff, readFp);

		if(buff[0] == EOF) break;

		sort(buff.begin(), buff.end());
		
		char writeFileName[20] = {0,};
		sprintf(writeFileName, "divied%d.txt", i++);
		
		FILE *writeFp = fopen(writeFileName, "w");
		if(writeFp == NULL)
			fileOpenFail();
		
		writeNumAray(buff, BUFFER_SIZE, writeFp);
		fileCount++;

		fclose(writeFp);
	}
	fclose(readFp);

	return fileCount;
}
void mergeFromFile(vector<int>& mergeAray, const vector<FILE*>& fp)
{
	size_t i=0;
	for(i=0; i<fp.size(); i++)
		mergeAray[i] = getNextNumOfFile(fp[i]);
}
int findMinIndexOfAray(const vector<int>& numAray, const vector<bool>& isEOF)
{
	int minIndex = 0;
	for(int i=0; i<isEOF.size(); i++)
	{
		if(!isEOF[i])
		{
			minIndex = i;
			break;
		}
	}

	for(int i=0; i<numAray.size(); i++)
	{
		if(!isEOF[i] && numAray[minIndex] > numAray[i])
			minIndex = i;
	}
	return minIndex;
}
int findMissingNumber(const char* fileName)
{
	int fileCount = externalSort(fileName);
	
	vector<FILE*> fp = vector<FILE*>(fileCount, 0);
	
	for(int i=0; i<fileCount; i++)
	{
		char readFileName[20] = {0,};
		sprintf(readFileName, "divied%d.txt", i);
		fp[i] = fopen(readFileName, "r");
		
		if(fp[i] == NULL)
			fileOpenFail();
	}

	// 각각의 파일에서 첫 번째 수를 꺼내서 합침
	vector<int> mergeAray = vector<int>(fileCount, 0);
	mergeFromFile(mergeAray, fp);

	// 배열에서 최소 값의 인덱스를 찾고 그 인덱스로 파일에 접근해 다음 숫자 가져옴
	vector<bool> isEOF = vector<bool>(fileCount, false);
	for(int i=0; i<NUMBER_RANGE-1; i++)
	{
		int minIndex = findMinIndexOfAray(mergeAray, isEOF);

		// 현재 최소 값과 i+1(1부터 시작해서 1씩 증가하는 값) 값을 비교
		// i+1과 같으면 최소 값 인덱스의 파일 포인터 +1
		// i+1이랑 다르면 빠진 숫자 찾음 -> 종료
		if(i + 1 != mergeAray[minIndex])
			return i+1;
		else
		{
			mergeAray[minIndex] = getNextNumOfFile(fp[minIndex]);
			if(mergeAray[minIndex] == EOF) isEOF[minIndex] = true;
		}
	}

	for(int i=0; i<fileCount; i++)
		fclose(fp[i]);
	
	return -1;
}
void fileOpenFail()
{
	cout << "File Open Error!" << endl;
		exit(1);
}
int calcMaxNumLength(){
	int count = 0;
	int num = NUMBER_RANGE;
	while(num){
		count++;
		num /= 10;
	}

	return count;
}
