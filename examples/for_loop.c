#include <stdio.h>

int main(){
	int i, sum = 0;
	// This is a loop, it iterates 10 times over
	// everything contained within the curly braces
	for(i = 0; i < 10; i++){
		sum += i;
		//sum = sum + i; //also valid	
	}
	printf("Sum: %d\n", sum);
	return 0;
}	
