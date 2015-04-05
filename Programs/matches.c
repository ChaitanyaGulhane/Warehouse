#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int
main(void)
{
	printf("\nEnter number of teams  => ");
	int teams;
	scanf("%d", &teams);
	printf("\nEnter number of rounds => ");
	int rounds;
	scanf("%d", &rounds);
	int no_of_matches = ceil(((teams-1)*rounds)/2);
	while((teams = ceil(teams/2)) > 0 )
	{
		no_of_matches ++;;
	}
	printf("\n No of matches = %d", no_of_matches);
	return 0;
}
