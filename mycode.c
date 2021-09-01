#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct node{
	int value;
	int count;
	struct node* left;
	struct node* right;
}Node;

typedef struct rle{
	int row;
	int col;
	int max;
	struct node* nodes;
}RLE;

typedef struct pgm{
	int row;
	int col;
	int max;
	int* arr;
}PGM;

Node* newNode(int value){
	Node* newNode = (Node*) malloc(sizeof(Node));
	newNode->count = 1;
	newNode->value = value;
	newNode->left = NULL;
    newNode->right = NULL;
	return newNode;
}

bool RLE_is_valid(RLE* rle){
	Node* iter;
	int count = 0;
	for(iter = rle->nodes; iter != NULL; iter = iter->right){
		if(iter->value > rle->max){
			printf("Invalid RLE: At least one value over the max value\n");
			return false;
		}
		if(iter->right != NULL){
			if(iter->value == iter->right->value){
				printf("Invalid RLE: Consequtive same value run lenghts\n");
				return false;
			}
		}
		count += iter->count;
	}
	if(count != rle->col * rle->row){
		printf("Invalid RLE: Number of pixels do not match with the dimensions\n");
		return false;
	}
	printf("RLE is valid\n");
	return true;
}

RLE* readRLE(FILE* fp){
	RLE* rle = (RLE*) malloc(sizeof(RLE));
	fscanf(fp, "%d", &rle->col);
	fscanf(fp, "%d", &rle->row);
	fscanf(fp, "%d", &rle->max);
	int count, value;
	fscanf(fp, "%d", &count);
	fscanf(fp, "%d", &value);
	rle->nodes = newNode(value);
	rle->nodes->count = count;
	Node* ptr = rle->nodes;
	while(1){
		fscanf(fp, "%d", &count);
		fscanf(fp, "%d", &value);
		if(feof(fp)){
			break;
		}
		ptr->right = newNode(value);
		ptr->right->left = ptr;
		ptr = ptr->right;
		ptr->count = count;
	}
	return rle;
}

PGM* readPGM(FILE* fp){
	char str[5];
	fscanf(fp, "%s", str);
	if(strcmp(str, "P2") != 0){
		printf("Invalid file\n");
		return NULL;
	}
	PGM* pgm = (PGM*) malloc(sizeof(PGM));
    fscanf(fp, "%d", &pgm->col);
    fscanf(fp, "%d", &pgm->row);
    fscanf(fp, "%d", &pgm->max);
    int i, tmp, N = pgm->col * pgm->row;
	pgm->arr = (int*) malloc(N * sizeof(int));
    for(i = 0; i < N; i ++){
		fscanf(fp, "%d", &tmp);
		if(feof(fp)){ 
			break;
		}
		pgm->arr[i] = tmp;
	}
	return pgm;
}

RLE* encode(PGM* pgm){
	int i, N = pgm->col * pgm->row;
	RLE* rle = (RLE*) malloc(sizeof(RLE));
	rle->col = pgm->col;
	rle->row = pgm->row;
	rle->max = pgm->max;
	rle->nodes = newNode(pgm->arr[0]);
	Node* ptr = rle->nodes;
	printf("RLE:\n");
    for(i = 1; i < N; i++){
		if(ptr->value == pgm->arr[i]){
			ptr->count++;
		}else{
			ptr->right = newNode(pgm->arr[i]);
			ptr->right->left = ptr;
			ptr = ptr->right;
		}
	}
	return rle;
}

PGM* decode(RLE* rle){
	if(RLE_is_valid(rle) == false){
		return NULL;
	}
	PGM* pgm = (PGM*) malloc(sizeof(PGM));
	pgm->col = rle->col;
	pgm->row = rle->row;
	pgm->max = rle->max;
	pgm->arr = (int*) malloc(pgm->col * pgm->row * sizeof(int));
	Node* iter;
	int i = 0, j;
	for(iter = rle->nodes; iter != NULL; iter = iter->right){
		for(j = 0; j < iter->count; j++){
			pgm->arr[i++] = iter->value;
		}
	}
	return pgm;
}

void writeRLE(FILE* fp, RLE* rle){
	fprintf(fp, "%d\t%d\n%d\n", rle->col, rle->row, rle->max);
	Node* iter;
    for(iter = rle->nodes; iter != NULL; iter = iter->right){
    	fprintf(fp, "%d %d ", iter->count, iter->value);
	}
}

void writePGM(FILE* fp, PGM* pgm){
	fprintf(fp, "P2\n%d %d\n%d\n", pgm->col, pgm->row, pgm->max);
	int i, j = 0, N = pgm->row * pgm->col;
	for(i = 0; i < N; i++){
		if(j == pgm->col){
			fprintf(fp, "\n");
			j = 0;
		}
		j++;
		fprintf(fp, "%d\t", pgm->arr[i]);
	}
}

void changeValues(RLE* rle, int oldVal, int newVal){
	if(newVal < 0 || newVal > rle->max || oldVal < 0 || oldVal >= rle->max){
		printf("invalid input\n");
		return;
	}
	Node* iter;
	Node* tmp;
	for(iter = rle->nodes; iter != NULL; iter = iter->right){
		if(iter->value == oldVal){
			iter->value = newVal;
			if(iter->left != NULL && iter->left->value == newVal){
				tmp = iter->left;
				iter->count += tmp->count;
				if(tmp->left != NULL){
					tmp->left->right = iter;
				}
				iter->left = tmp->left;
				if(tmp == rle->nodes){
					rle->nodes = iter;
				}
				free(tmp);
			}
			if(iter->right != NULL && iter->right->value == newVal){
				tmp = iter->right;
				iter->count += tmp->count;
				if(tmp->right != NULL){
					tmp->right->left = iter;
				}
				iter->right = tmp->right;
				free(tmp);
			}
		}
	}
}

RLE* createHistogram(RLE* rle){
	RLE* hist = (RLE*) malloc(sizeof(RLE));
	hist->col = rle->col;
	hist->row = rle->row;
	hist->max = rle->max;
	hist->nodes = newNode(rle->nodes->value);
	hist->nodes->count = rle->nodes->count;
	Node* iter;
	Node* ptr;
	for(iter = rle->nodes->right; iter != NULL; iter = iter->right){
		for(ptr = hist->nodes; ptr != NULL; ptr = ptr->right){
			if(ptr->value == iter->value){
				ptr->count += iter->count;
				break;
			}
		}
		if(ptr == NULL){
			ptr = newNode(iter->value);
			ptr->count = iter->count;
			ptr->right = hist->nodes;
			hist->nodes = ptr;
		}
	}
	return hist;
}

void changeCoordinate(RLE* rle, int row, int col, int val){
	if(row < 0 || row >= rle->row || col < 0 || col >= rle->col){
		printf("Coordinate out of bounds\n");
		return;
	}
	int des = row * rle->col + col;
	int ptr = -1;
	Node* iter;
	for(iter = rle->nodes; iter != NULL; iter = iter->right){
		if(ptr + iter->count >= des){
			break;
		}
		ptr += iter->count;
	}
	if(iter->value == val){
		return;
	}
	int intervalL = des - ptr - 1;
	int intervalR = ptr + iter->count - des;
	Node* new = newNode(val);
	Node* tmp;
	//left side
	if(intervalL == 0){
		if(iter->left == NULL){
			new->left = NULL;
			rle->nodes = new;
		}else if(new->value == iter->left->value){
			tmp = iter->left;
			if(tmp->left == NULL){
				new->count += tmp->count;
				rle->nodes = new;
			}else{
				tmp->left->right = new;
				new->left = tmp->left;
				new->count += tmp->count;
			}
			free(tmp);
		}else{
			iter->left->right = new;
			new->left = iter->left;
		}
	}
	else{
		tmp = newNode(iter->value);
		tmp->count = intervalL;
		new->left = tmp;
		tmp->right = new;
		tmp->left = iter->left;
		if(iter->left != NULL){
			iter->left->right = tmp;
		}
		if(iter == rle->nodes){
			rle->nodes = tmp;
		}
	}
	//right side
	if(intervalR == 0){
		if(iter->right == NULL){
			new->right = NULL;
		}else if(new->value == iter->right->value){
			tmp = iter->right;
			if(tmp->right == NULL){
				new->count += tmp->count;
			}else{
				tmp->right->left = new;
				new->right = tmp->right;
				new->count += tmp->count;
			}
			free(tmp);
		}else{
			iter->right->left = new;
			new->right = iter->right;
		}
	}else{
		tmp = newNode(iter->value);
		tmp->count = intervalR;
		new->right = tmp;
		tmp->left = new;
		tmp->right = iter->right;
		if(iter->right != NULL){
			iter->right->left = tmp;
		}
	}
	free(iter);
}

void printRLE(RLE* rle){
	Node* iter;
	printf("RLE:\n");
	for(iter = rle->nodes; iter != NULL; iter = iter->right){
		printf("%d\t%d\n", iter->count, iter->value);
	}
}

int main(){
	char filename[20];
	int a;
	printf("Enter pgm file name:\n");
	scanf("%s", filename);
	
	FILE* fp = fopen(filename, "r");
	if(fp == NULL){
		printf("File does not exist");
		return 0;
	}
	PGM* pgm = readPGM(fp);
	fclose(fp);
	
	RLE* rle = encode(pgm);
	printRLE(rle);
	
	fp = fopen("test_encoded.txt", "w");
	writeRLE(fp, rle);
	fclose(fp);
	printf("test_encoded.txt has been created\n");
	
	int val1, val2, val3;
	while(a != 4){
		fp = fopen("test_encoded.txt", "r");
		rle = readRLE(fp);
		fclose(fp);
		printf("Choose a process:\n");
		printf("1. change a color\n2. change a value with coordinates\n3. create histogram\n4. quit\n");
		scanf("%d", &a);
		switch(a){
			case 1 :
				printf("Enter the color you want to change:\n");
				scanf("%d", &val1);
				printf("Enter the new color:\n");
				scanf("%d", &val2);
				changeValues(rle, val1, val2);
				break;
			case 2 :
				printf("Enter row:\n");
				scanf("%d", &val1);
				printf("Enter column:\n");
				scanf("%d", &val2);
				printf("Enter new value:\n");
				scanf("%d", &val3);
				changeCoordinate(rle, val1, val2, val3);
				break;
			case 3 :
				rle = createHistogram(rle);		
		}
		printRLE(rle);
		fp = fopen("test_encoded.txt", "w");
		writeRLE(fp, rle);
		fclose(fp);
		printf("test_encoded.txt has been updated\n");
	}
	
	fp = fopen("test_encoded.txt", "r");
	RLE* rle1 = readRLE(fp);
	fclose(fp);
	
	PGM* pgm1 = decode(rle1);
	if(pgm1 == NULL){
		return 0;
	}
	
	fp = fopen("test_decoded.pgm", "w");
	writePGM(fp, pgm1);
	fclose(fp);
	printf("test_decoded.pgm has been created\n");
	
}
