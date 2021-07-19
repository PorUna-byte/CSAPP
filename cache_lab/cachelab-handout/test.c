#include<stdio.h>
#include<stdlib.h>
#include<time.h>
void show_array(int A[16][16])
{
   for(int i=0;i<16;i++)
   {
       for(int j=0;j<16;j++)
       {
           printf("%d  ",A[i][j]);
       }
       printf("\n");
   }
}
void trans5(int M, int N, int A[N][M], int B[M][N]) //8*4 block A row-wise transpose
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
                show_array(B);
                printf("\n\n");
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
                show_array(B);
                printf("\n\n");
        }    
    }   
    //since we use the right 8 columns of B as "wrong" space , we must check the boundary of B.
    //So we treat the last 8 rows of A specially.
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
int main(){
    srand(time(NULL));
    int A[16][16];
    int B[16][16];
   for(int i=0;i<16;i++)
   {
       for(int j=0;j<16;j++)
       {
           A[i][j]=rand()%1000;
       }
   }
      for(int i=0;i<16;i++)
   {
       for(int j=0;j<16;j++)
       {
           B[i][j]=-1;
       }
   }
    show_array(A);
    printf("\n\n");
    trans5(16,16,A,B);
    show_array(B);
    printf("\n\n");
    show_array(A);
    printf("\n\n");
    int equal=1;
    for(int i=0;i<16;i++)
    {
        for(int j=0;j<16;j++)
         if(A[i][j]!=B[j][i])
         {
              equal=0;
              break;
         }
    }
    printf("A=B^T? %d\n",equal);
}