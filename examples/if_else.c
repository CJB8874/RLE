#include <stdio.h>

int main(){
	int i, j = 0;
	/* the if statement takes an argument that is conditional
	 * If the statement evaluated to true then what is inside
	 * the curly braces is done, if it is false then it evaluates
	 * the else statement
	 */
	for(i = 0; i < 10; i++){
		// The % (modulo) operator takes the remainder if the
		// value is divided by the operand on the right.
		if(j % 2 == 0){
			printf("%d is even\n", j);
			j++;
			// This iterates the variable by 1
			// equivalent to the following:
			// j = j + 1;
		}
		else{
			printf("%d is odd\n", j);
			j++;
		}
	}
	return 0;
}
