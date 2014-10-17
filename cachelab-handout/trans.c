/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"
#include "contracts.h"

//This is the block size for 32*32 and 64*64
#define block 8
//This is the block for the 61*67
#define spcialBlock 17

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. The REQUIRES and ENSURES from 15-122 are included
 *     for your convenience. They can be removed if you like.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    REQUIRES(M > 0);
    REQUIRES(N > 0);
    //this is the index we need when calculating
    int i=0,j=0,k=0,l=0;
    //this is the temporal number we use to store the swap number
    int x0,x1,x2,x3;
    if(M==32&&N==32){
  	//use the block as size of 8
  	//generally follows the idea of the block;
  	//But since we don't need to swap the diagnal numbers
    	while(i<32){
	    j=0;
	    while(j<32){
		/*
		if(i==j){
		    k=i;
		    while(k<i+block){
		    	x0=A[k][j];
		   	x1=A[k][j+1];
		    	x2=A[k][j+2];
		    	x3=A[k][j+3];
		    	x4=A[k][j+4];
		    	x5=A[k][j+5];
		    	x6=A[k][j+6];
		    	x7=A[k][j+7];
		    	B[j][k]=x0;
		    	B[j+1][k]=x1;
		    	B[j+2][k]=x2;
		    	B[j+3][k]=x3;
		    	B[j+4][k]=x4;
		    	B[j+5][k]=x5;
		    	B[j+6][k]=x6;
		    	B[j+7][k]=x7;
		    	k++;
		    }
		}else{
		*/
		    //where blocking begins
		    k=j;
		    while(k<j+block){
			l=i;
		    	while(l<i+block){
			    B[l][k] = A[k][l];
			    l++;
			}
			k++;
		    
		}
		j+=block;
	    }
	    i+=block;
        }
    }
    //This will handle the case when M=64 and N=64
    //This time I also use the blocking
    //But since the cache size could only store 8
    //So we swap it one more time
    if(M==64&&N==64){
	i=0;
   	while(i<64){
	    j=0;	
	    while(j<64){
		k=0;
		while(k<block){
		    l=0;
		    x0 = A[k+j][i];
		    x1 = A[k+j][i+1];
		    x2 = A[k+j][i+2]; 
		    x3 = A[k+j][i+3]; 
		    B[i][k+j] = x0;
		    B[i+1][k+j] = x1; 
		    B[i+2][k+j] = x2; 
		    B[i+3][k+j] = x3;
		    k++;
		}
		l=7;
		while(l>=0){
		    x0 = A[l+j][i+4];
		    x1 = A[l+j][i+5];
		    x2 = A[l+j][i+6]; 
		    x3 = A[l+j][i+7]; 
		    B[i+4][l+j] = x0;
		    B[i+5][l+j] = x1; 
		    B[i+6][l+j] = x2; 
		    B[i+7][l+j] = x3;
		    l--;
		}
		j+=block;
	    }
	    i+=block;
        }
    }
    //This will handle the situation when M=61 and N=67
    //This time we don't need to handle the diagnal problem
    //We just need to care about the block size
    //So it is even easier than the 32*32
    //I just tested several times and found out that 17 is the best number
    //Although I felt this is a magic number
    if(M==61){
	i=0;
   	while(i<61){
	    j=0;	
	    while(j<67){
		k=j;	
		while(k<(j+17)&&k<67){
		    l=i;
		    while(l<(i+17)&&l<61){
			B[l][k]=A[k][l];
			l++;
		    }
		    k++;
		}
		j+=17;
	    }
	    i+=17;
        }
    }
    ENSURES(is_transpose(M, N, A, B));
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
/*
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    REQUIRES(M > 0);
    REQUIRES(N > 0);

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

    ENSURES(is_transpose(M, N, A, B));
}
*/
/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    //registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

