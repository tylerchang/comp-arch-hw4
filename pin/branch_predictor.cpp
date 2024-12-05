#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int static_prediction(){
    return 1;
}

struct bit_Node{
        int branch;
        int taken;
        int hyteris;
        bit_Node* next;
    } item;

bit_Node* branchListHead = NULL;


void AddBranchInfo(int branch, int taken) {
    bit_Node* newNode = (bit_Node*)malloc(sizeof(bit_Node));
    if (!newNode) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    newNode->branch = branch;
    newNode->taken = taken;
    newNode->next = NULL;
    if (!branchListHead) {
        branchListHead = newNode;
    } else {
        bit_Node* current = branchListHead;
        while (current->next) {
            current = current->next;
        }
        current->next = newNode;
    }
}

void FreeBranchList() {
    bit_Node* current = branchListHead;
    while (current != NULL) {
        bit_Node* temp = current;
        current = current->next;
        free(temp);
    }
    branchListHead = NULL;
}


void PrintAllNodes() {
    bit_Node* current = branchListHead;
    if (!current) {
        printf("The list is empty.\n");
        return;
    }
    
    while (current) {
        printf("Branch: %d, Taken: %d\n", current->branch, current->taken);
        current = current->next;
    }
}


int one_bit_prediction(int branch, int actual_pred){
    //do linear seach, if it fails return the static prediction
    //update the predcition of that node to be actual_pred
    bit_Node *p = branchListHead;
    int prediction = -1;

    while (p != NULL) {
        if (branch == p->branch) {
            prediction = p->taken;
            p->taken = actual_pred;
            break;
        }
        p = p->next;
    }

    if (prediction == -1){
        prediction =  static_prediction();
        AddBranchInfo(branch, actual_pred);
    }
    return prediction;
}


int two_bit_prediction(int branch, int actual_pred){
    //do linear seach, if it fails return the static prediction
    //update the predcition of that node to be actual_pred
    bit_Node *p = branchListHead;
    int prediction = -1;

    while (p != NULL) {
        if (branch == p->branch) {
            prediction = p->taken;
            if (actual_pred != prediction){
                if (p->hyteris == 1){
                    p->hyteris = 0;
                }
                else{
                    p->taken = actual_pred;
                }
            }
            else{
                if (p->hyteris == 0){
                    p->hyteris = 1;
                }
            }
            
            break;
        }
        p = p->next;
    }


    if (prediction == -1){
        prediction =  static_prediction();
        AddBranchInfo(branch, actual_pred);
    }
    return prediction;
}

int main(){

    const int file_len = 1;
    char buffer[256];
    char inst_point[4];
    char target[4];
    char fall_though[4];
    char taken_char[2];
    int taken;

    int total_branches;
    int correct_branches;


    struct filename {
        char filename[100];
    };

    struct filename file_array[file_len] = {
        {"sample_data.txt"},
    };

    FILE *fp;
    for (int i=0; i<file_len; i++){
        total_branches = 0;
        correct_branches = 0;


        char *filename = file_array[i].filename;
        fp = fopen(filename, "r"); 

        while (fgets(buffer, 256, fp) != NULL) {
            if (strlen(buffer) > 90) {
                strncpy(inst_point, &buffer[20], sizeof(inst_point) - 1);
                inst_point[3] = '\0';

                int current_branch = strtol(inst_point, NULL, 16);
              
                strncpy(target, &buffer[45], sizeof(target) - 1); 
                target[3] = '\0';
            
                strncpy(fall_though, &buffer[76], sizeof(fall_though) - 1); 
                fall_though[3] = '\0';

                strncpy(taken_char, &buffer[89], sizeof(taken_char) - 1);
                int taken = atoi(taken_char);

                //int prediction = static_prediction();
                int prediction = two_bit_prediction(current_branch, taken);


                if (prediction == taken){
                    correct_branches ++;
                }
                total_branches ++;

            }
        }

        printf("For file %s, we have %d/%d correct branches", filename, correct_branches, total_branches); 


    }

    fclose(fp);
    FreeBranchList();


}
