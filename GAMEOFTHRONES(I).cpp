#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
int main() {
	char s[100010];
	gets(s);
	int i=0,j;
	int arr[26]={0};
	while(s[i]!='\0')
	{
		arr[s[i]-'a']++;
		i++;
	}
	int c=0;
	for(j=0;j<26;j++)
	{
		if(arr[j]%2)
			c++;
	}
	if(c==1||c==0)
		printf("YES");
	else
		printf("NO");
	return 0;
}
