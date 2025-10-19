#include<stdio.h>
#include<stdlib.h>
#include<time.h>

#define maxSonarSize 10
#define minSonarSize 2

void getRandomisedSonarImage(int sonarSize,int sonarImage[sonarSize][sonarSize]);
int getRandomValue();
void printSonarImage(int sonarSize,int sonarImage[sonarSize][sonarSize]);
void getRotatedBy90(int sonarSize,int sonarImage[sonarSize][sonarSize]);
void getTranspose(int sonarSize,int sonarImage[sonarSize][sonarSize]);
void getRowsReversed(int sonarSize,int sonarImage[sonarSize][sonarSize]);
void applySmoothingFilter(int sonarSize,int sonarImage[sonarSize][sonarSize]);

int main(){
    
    int sonarSize; 
    printf("Enter the size of Sonar Image (2 to 10) : ");
    scanf("%d",&sonarSize);
    if(sonarSize > maxSonarSize || sonarSize < minSonarSize){
        printf("You have entered a wrong Value !!! ");
        return 0;
    }

    int sonarImage[sonarSize][sonarSize];

    srand(time(NULL));

    getRandomisedSonarImage(sonarSize,sonarImage);
    printf("Original Randomly Generated Matrix: \n");
    printSonarImage(sonarSize,sonarImage);

    getRotatedBy90(sonarSize,sonarImage);
    printf("Matrix after 90 degree Clockwise Rotation: \n");
    printSonarImage(sonarSize,sonarImage);

    applySmoothingFilter(sonarSize,sonarImage);
    printf("Matrix after Applying 3X3 Smoothing Filter:  \n");
    printSonarImage(sonarSize,sonarImage);

    return 0;
}

void applySmoothingFilter(int sonarSize,int sonarImage[sonarSize][sonarSize]){
    int originalPreviousRow[sonarSize];
    int originalCurrentRow[sonarSize];

    int *prevRowPtr = originalPreviousRow;
    int *currRowPtr = originalCurrentRow;

    for (int row = 0; row < sonarSize; row++) {
        for (int col = 0; col < sonarSize; col++) {
            *(currRowPtr + col) = *(*(sonarImage + row) + col);
        }

        for (int col = 0; col < sonarSize; col++) {
            int sum = 0, count = 0;

            if (row > 0) {
                if (col > 0) { 
                    sum += *(prevRowPtr + col - 1); 
                    count++; 
                }
                sum += *(prevRowPtr + col); 
                count++;
                if (col < sonarSize - 1) { 
                    sum += *(prevRowPtr + col + 1); 
                    count++; 
                }
            }

            if (col > 0) { 
                sum += *(currRowPtr + col - 1); 
                count++; 
            }
            sum += *(currRowPtr + col); 
            count++;
            if (col < sonarSize - 1) { 
                sum += *(currRowPtr + col + 1); 
                count++; 
            }

            if (row < sonarSize - 1) {
                int *nextRowPtr = *(sonarImage + row + 1);
                if (col > 0) { 
                    sum += *(nextRowPtr + col - 1); 
                    count++; 
                }
                sum += *(nextRowPtr + col); 
                count++;
                if (col < sonarSize - 1) { 
                    sum += *(nextRowPtr + col + 1); 
                    count++; 
                }
            }

            *(*(sonarImage + row) + col) = sum / count;
        }

        for (int col = 0; col < sonarSize; col++) {
            *(prevRowPtr + col) = *(currRowPtr + col);
        }
    }
}


void getRotatedBy90(int sonarSize,int sonarImage[sonarSize][sonarSize]){
    getTranspose(sonarSize,sonarImage);
    getRowsReversed(sonarSize,sonarImage);
}

void getTranspose(int sonarSize,int sonarImage[sonarSize][sonarSize]){
    for(int row=0;row<sonarSize;row++){
        for(int col=row+1;col<sonarSize;col++){
            int temp = *(*(sonarImage+row)+col);
            *(*(sonarImage+row)+col) = *(*(sonarImage+col)+row);
            *(*(sonarImage+col)+row) = temp;
        }
    }
}

void getRowsReversed(int sonarSize,int sonarImage[sonarSize][sonarSize]){
    for(int row=0;row<sonarSize;row++){
        for(int col=0;col<sonarSize/2;col++){
            int temp = *(*(sonarImage + row)+col);
            *(*(sonarImage + row)+col) = *(*(sonarImage + row)+(sonarSize-col-1));
            *(*(sonarImage + row)+(sonarSize-col-1)) = temp;
        }
    }
}


void getRandomisedSonarImage(int sonarSize,int sonarImage[sonarSize][sonarSize]){
    for(int row=0; row < sonarSize; row++){
        for(int col=0; col < sonarSize; col++){
            int randomValue = getRandomValue();
            *(*(sonarImage + row) + col) = randomValue;
        }
    }
}

int getRandomValue(){
    return (rand() % 256);
}

void printSonarImage(int sonarSize,int sonarImage[sonarSize][sonarSize]){
    for(int row=0;row<sonarSize;row++){
        for(int col=0;col<sonarSize;col++){
            int val= *(*(sonarImage+row)+col);
            printf("%4d ",val);
        }
        printf("\n");
    }
}

