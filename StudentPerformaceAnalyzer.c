#include<stdio.h>

struct Student {
    int rollNo;
    char name[60];
    int marks[3];
};

int getTotal(int *marks);
float getAverage(int totalMarks);
char getGrade(float avgMarks);
void printRollNosByRecursion(struct Student *students,int stIdx,int totalStudents);
int isValidRollNo(struct Student *students,int rollNo,int idx);

int main(){
    
    int totalStudents=0;
    printf("Enter the total number of students : ");
    scanf("%d",&totalStudents);

    struct Student students[totalStudents];

    for(int i=0;i<totalStudents;i++){
        printf("Enter details for Student %d (RollNo Name Marks1 Marks2 Marks3): \n", i + 1);
        scanf("%d %s %d %d %d",
            &students[i].rollNo,
            students[i].name,
            &students[i].marks[0],
            &students[i].marks[1],
            &students[i].marks[2]
        );

        int isRollNoValid = isValidRollNo(students,students[i].rollNo,i);
        int isMarksValid = (students[i].marks[0] >= 0 && students[i].marks[0] <= 100) && (students[i].marks[1] >= 0 && students[i].marks[1] <= 100) && (students[i].marks[2] >= 0 && students[i].marks[2] <= 100);
        
        if( !isRollNoValid || !isMarksValid){
            printf("Invalid input for this student ! Please re-enter details.\n");
            i--;
        }
    }

    printf("-----Student's Performance Analyzer----- \n\n");

    for(int i=0;i<totalStudents;i++){
        printf("Roll : %d \n",students[i].rollNo);
        
        printf("Name : %s \n",students[i].name);
        
        int totalMarks = getTotal(students[i].marks);
        printf("Total : %d \n",totalMarks);
        
        float avgMarks = getAverage(totalMarks);
        printf("Average : %f \n",avgMarks);
        
        char grade = getGrade(avgMarks);
        printf("Grade : %c \n",grade);
        
        if (avgMarks < 35) {
            printf("\n");
            continue;
        }
        printf("Performance : ");
        switch (grade) {
            case 'A':
                printf("*****\n");
                break;
            case 'B':
                printf("****\n");
                break;
            case 'C':
                printf("***\n");
                break;
            case 'D':
                printf("**\n");
                break;
        }
        printf("\n");
    }
    printf("List of Roll Numbers (via recursion) : ");
    printRollNosByRecursion(students,0,totalStudents);
    return 0;
}

int getTotal(int *marks){
    return marks[0]+marks[1]+marks[2];
}

float getAverage(int totalMarks){
    return totalMarks/3.0;
}

char getGrade(float avg){

    if(avg >= 85){
        return 'A';
    }else if(avg >= 70){
        return 'B';
    }else if(avg >= 50){
        return 'C';
    }else if(avg >= 35){
        return 'D';
    }else{
        return 'F';
    }
    // Athrav , I have used if else over here rather Switch case because we cannot compare floating values in switch case in C
}

int isValidRollNo(struct Student *students,int rollNo,int idx){
    if(rollNo <= 0){
        return 0;
    }
    for(int i=0;i<idx;i++){
        if(students[i].rollNo == rollNo){
            return 0;
        }
    }
    return 1;
}

void printRollNosByRecursion(struct Student *students,int stIdx,int totalStudents){
    if(stIdx >= totalStudents){
        return;
    }
    printf("%d ",students[stIdx].rollNo);
    printRollNosByRecursion(students,stIdx+1,totalStudents);
}