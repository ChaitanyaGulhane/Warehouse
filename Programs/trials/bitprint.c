#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>

void printbit(uint32_t temp)
{
	int i =0;
	unsigned int bit = (uintmax_t)UINT32_MAX;
	
	/*bit = temp << 1;*/
	printf("\n\n bit = %d", bit);
	for(; i < (sizeof(temp)*8); i++)
	{
		if( (bit & (temp <<=  1) ))
			printf("1");
		else
			printf("0");
//	printf("\n\n Temp = %d    ", temp);
		//temp = temp >> 1;
	}
	printf("\n\ntemp = %d and size = %d", temp, sizeof(temp));

}

int main()
{
	uint32_t x = 32;
	printf("\n\n");
	printbit(x);
	printf("\n\n");
	return 0;
}
