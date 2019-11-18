#include <stdio.h>
int strCompare(char* str1, char* str2);

void main()
{
	int j = strCompare("Hello","No");
	printf("%d",j);
}

int strCompare(char* str1,char* str2)
{
	int i;
	for(i=0;i<6;i++)
	{
		if(str1[i] != str2[i])
			return 0;
		if(str1[i] == '\0' && str2[i] == '\0')
			break;
		if(str1[i] == '\r' && str2[i]=='\0')
			break;
	}
	return 1;
}
