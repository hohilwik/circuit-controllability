#include<stdio.h>
int main()
{
	// inputs
	for(int i=0; i<64; i++)
	{
		printf("%d ", i+1);
	}
	printf("\n");
	// outputs
	for(int i=0; i<64; i++)
	{
		printf("%d ", 64+i+1);
	}
	printf("\n");
	
	// nodes
	for(int i=0; i<194; i++)
	{
		printf("%d ", i+1);
	}
	printf("\n");
	//gates
	int gates=0;
	// Gi
	for(int i=0; i<32; i++)
	{
		printf("AND 2 %d %d %d\n", i+1, 32+i+1, 96+i+1);
		gates++;
	}
	// Pi
	for(int i=0; i<32; i++)
	{
		printf("XOR 2 %d %d %d\n", i+1, 32+i+1, 129+i+1);
		gates++;
	}
	// PiCi
	for(int i=0; i<32; i++)
	{
		printf("AND 2 %d %d %d\n", 129+i+1, 161+i+1, 162+i+1);
		gates++;
	}
	// Ci
	for(int i=0; i<32; i++)
	{
		printf("OR 2 %d %d %d\n", 96+i+1, 162+i+1, 64+i+1);
		gates++;
	}
	printf("GATES %d", gates);
	return 0;
}