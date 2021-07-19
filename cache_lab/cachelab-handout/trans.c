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

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */

char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) //final result
{ 
   int i,j,tmp_0,tmp_1,tmp_2,tmp_3,tmp_4,tmp_5,tmp_6,tmp_7,ib,jb;
    for(ib=0;ib<N-15;ib+=8){
        for(jb=0;jb<M-7;jb+=8){
            for (i = ib; i < ib+8; i++) {
                for (j = jb; j < jb+8; j+=8) {
                tmp_0=A[i][j];
                tmp_1=A[i][j+1];
                tmp_2=A[i][j+2];
                tmp_3=A[i][j+3];
                tmp_4=A[i][j+4];
                tmp_5=A[i][j+5];
                tmp_6=A[i][j+6];
                tmp_7=A[i][j+7];
                B[j][i]=tmp_0;
                B[j+1][i]=tmp_1;
                B[j+2][i]=tmp_2;
                B[j+3][i]=tmp_3;
                B[j][i+8]=tmp_4;  //store the late 4 ints of a line of A into the "wrong" space in B temporarily to avoid too much cache conflict misses.
                B[j+1][i+8]=tmp_5;
                B[j+2][i+8]=tmp_6;
                B[j+3][i+8]=tmp_7;
                }
            }
        for(j=jb;j<jb+4;j++){  //move the elements in wrong space in B to the right space in B
            for(i=ib+8;i<ib+16;i+=8){
                tmp_0=B[j][i];
                tmp_1=B[j][i+1];
                tmp_2=B[j][i+2]; 
                tmp_3=B[j][i+3];
                tmp_4=B[j][i+4];
                tmp_5=B[j][i+5];
                tmp_6=B[j][i+6]; 
                tmp_7=B[j][i+7];
                B[j+4][i-8]=tmp_0;
                B[j+4][i-7]=tmp_1;
                B[j+4][i-6]=tmp_2;
                B[j+4][i-5]=tmp_3;
                B[j+4][i-4]=tmp_4;
                B[j+4][i-3]=tmp_5;
                B[j+4][i-2]=tmp_6;
                B[j+4][i-1]=tmp_7;
            }    
        } 
    }   
        
        for(;jb<M;jb++) //finish any remaining columns
        {
          tmp_0=A[ib][jb];
          tmp_1=A[ib+1][jb];
          tmp_2=A[ib+2][jb];
          tmp_3=A[ib+3][jb];
          tmp_4=A[ib+4][jb];
          tmp_5=A[ib+5][jb];
          tmp_6=A[ib+6][jb];
          tmp_7=A[ib+7][jb];
          B[jb][ib]=tmp_0;
          B[jb][ib+1]=tmp_1;
          B[jb][ib+2]=tmp_2;
          B[jb][ib+3]=tmp_3;
          B[jb][ib+4]=tmp_4;
          B[jb][ib+5]=tmp_5;
          B[jb][ib+6]=tmp_6;
          B[jb][ib+7]=tmp_7;
        }
    }   
    //since we use the right 8 columns of B as "wrong" space , we must check the boundary of B.
    //So we treat the last 8 rows of A specially.
    for(;ib<N;ib+=8){  //using 8*4 block strategy
        for(jb=0;jb<M-3;jb+=4){
            for (i = ib; i<N&&i<ib+8;i++) {
                for(j=jb;j<M&&j<jb+4;j+=4){
                    tmp_0=A[i][j];
                    tmp_1=A[i][j+1];
                    tmp_2=A[i][j+2];
                    tmp_3=A[i][j+3];
                    B[j][i]=tmp_0;
                    B[j+1][i]=tmp_1;
                    B[j+2][i]=tmp_2;
                    B[j+3][i]=tmp_3;
                }
            }
        }
        for(;jb<M;jb++)  //finish any remaining
        {
            for(i=ib;i<N&&i<ib+8;i++)
               B[jb][i]=A[i][jb];
        }    
    }  
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc1[] = "8*8 block A-column wise scan transpose";
void trans1(int M, int N, int A[N][M], int B[M][N])
{
    int i,j,tmp_0,tmp_1,tmp_2,tmp_3,tmp_4,tmp_5,tmp_6,tmp_7,ib,jb;
    for(jb=0;jb<M-7;jb+=8){
        for(ib=0;ib<N-7;ib+=8){
            for (i = ib; i < ib+8; i++) {  //deal with a 8*8 block
                for (j = jb; j < jb+8; j+=8) {
                tmp_0=A[i][j];
                tmp_1=A[i][j+1];
                tmp_2=A[i][j+2];
                tmp_3=A[i][j+3];
                tmp_4=A[i][j+4];
                tmp_5=A[i][j+5];
                tmp_6=A[i][j+6];
                tmp_7=A[i][j+7];
                B[j][i]=tmp_0;
                B[j+1][i]=tmp_1;
                B[j+2][i]=tmp_2;
                B[j+3][i]=tmp_3;
                B[j+4][i]=tmp_4;
                B[j+5][i]=tmp_5;
                B[j+6][i]=tmp_6;
                B[j+7][i]=tmp_7;
                }
            }
        }    
    }    

}
char trans_desc2[] = "8*8 (contains 4 4*4 blocks)block A row-wise scan transpose";
void trans2(int M, int N, int A[N][M], int B[M][N])
{
    int i,j,tmp_0,tmp_1,tmp_2,tmp_3,ib,jb,ibb,jbb;
    for(ibb=0;ibb<N-7;ibb+=8){
        for(jbb=0;jbb<M-7;jbb+=8){
            for(ib=ibb;ib<ibb+8;ib+=4){
                for(jb=jbb;jb<jbb+8;jb+=4){
                    for (i = ib; i < ib+4; i++) {
                        for (j = jb; j < jb+4; j+=4) {
                        tmp_0=A[i][j];
                        tmp_1=A[i][j+1];
                        tmp_2=A[i][j+2];
                        tmp_3=A[i][j+3];
                        B[j][i]=tmp_0;
                        B[j+1][i]=tmp_1;
                        B[j+2][i]=tmp_2;
                        B[j+3][i]=tmp_3;
                        }
                    }
                }    
            } 
        }    
    }                
}
char trans_desc3[] = "8*4 block A row-wise scan transpose";
void trans3(int M, int N, int A[N][M], int B[M][N]) //8*4 block A row-wise transpose
{ 
    int i,j,tmp_0,tmp_1,tmp_2,tmp_3,ib,jb;
    for(ib=0;ib<N-7;ib+=8){
        for(jb=0;jb<M-3;jb+=4){
            for (i = ib; i < ib+8; i++) {
                for (j = jb; j < jb+4; j+=4) {
                tmp_0=A[i][j];
                tmp_1=A[i][j+1];
                tmp_2=A[i][j+2];
                tmp_3=A[i][j+3];
                B[j][i]=tmp_0;
                B[j+1][i]=tmp_1;
                B[j+2][i]=tmp_2;
                B[j+3][i]=tmp_3;
                }
            }
        }    
    }    
}
char trans_desc4[] = "8*4 block A row-wise scan transpose(store the later 4 ints in B span 4)";
void trans4(int M, int N, int A[N][M], int B[M][N]) //8*4 block A row-wise transpose
{ 
    int i,j,tmp_0,tmp_1,tmp_2,tmp_3,tmp_4,tmp_5,tmp_6,tmp_7,ib,jb;
    for(ib=0;ib<N-7;ib+=4){
        for(jb=0;jb<M-7;jb+=8){
            for (i = ib; i < ib+4; i++) {
                for (j = jb; j < jb+8; j+=8) {
                tmp_0=A[i][j];
                tmp_1=A[i][j+1];
                tmp_2=A[i][j+2];
                tmp_3=A[i][j+3];
                tmp_4=A[i][j+4];
                tmp_5=A[i][j+5];
                tmp_6=A[i][j+6];
                tmp_7=A[i][j+7];
                B[j][i]=tmp_0;
                B[j+1][i]=tmp_1;
                B[j+2][i]=tmp_2;
                B[j+3][i]=tmp_3;
                B[j][i+4]=tmp_4;  //store the late 4 ints of a line of A into the "wrong" space in B temporarily to avoid too much cache conflict misses.
                B[j+1][i+4]=tmp_5;
                B[j+2][i+4]=tmp_6;
                B[j+3][i+4]=tmp_7;
                }
            }
        for(j=jb;j<jb+4;j++){  //move the elements in wrong space in B to the right space in B
            for(i=ib+4;i<ib+8;i+=4){
                tmp_0=B[j][i];
                tmp_1=B[j][i];
                tmp_2=B[j][i]; 
                tmp_3=B[j][i];
                B[j+4][i-4]=tmp_0;
                B[j+4][i-3]=tmp_1;
                B[j+4][i-2]=tmp_2;
                B[j+4][i-1]=tmp_3;
            }    
        }  
        }    
    }   
    //since we use the right 4 columns of B as "wrong" space , we must check the boundary of B.
    //So we treat the last 4 rows of A specially.
        for(jb=0;jb<M-7;jb+=8){
            for (i = ib; i < ib+4; i++) {
                for (j = jb; j < jb+8; j+=8) {
                    tmp_0=A[i][j];
                    tmp_1=A[i][j+1];
                    tmp_2=A[i][j+2];
                    tmp_3=A[i][j+3];
                    tmp_4=A[i][j+4];
                    tmp_5=A[i][j+5];
                    tmp_6=A[i][j+6];
                    tmp_7=A[i][j+7];
                    B[j][i]=tmp_0;
                    B[j+1][i]=tmp_1;
                    B[j+2][i]=tmp_2;
                    B[j+3][i]=tmp_3;
                    B[j+4][i]=tmp_4;
                    B[j+5][i]=tmp_5;
                    B[j+6][i]=tmp_6;
                    B[j+7][i]=tmp_7;
                }
            }
        }           
}
char trans_desc5[] = "23*23 block";
void trans5(int M, int N, int A[N][M], int B[M][N]) //only work for 61*67
{ 
    int ib,jb,i,j;
    for(ib=0;ib<N;ib+=23)
    {
        for(jb=0;jb<M;jb+=23)
        {
            for(i=ib;i<ib+23&&i<N;i++)
            {
                for(j=jb;j<jb+23&&j<M;j++)
                  B[j][i]=A[i][j];
            }
        }
    }   
}
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
   // registerTransFunction(trans1, trans_desc1); 

   // registerTransFunction(trans2, trans_desc2); 

    //registerTransFunction(trans3, trans_desc3); 

    //registerTransFunction(trans4, trans_desc4); 
    //registerTransFunction(trans5, trans_desc5); 
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

