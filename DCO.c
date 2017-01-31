/*
 * Nicholas Marasco
 * CIS 421 - AI
 * Assignment 4
 * Due Nov. 10
 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "node.h"

#define DATA_LINE_SIZE 64

#define COLONY_SIZE 10

#define ALPHA 1.0
#define BETA 0.4
#define RHO 0.65
#define Q 100

// prototypes
void search();
Ant* tour(int);
void decay();
void updatePheromone(Ant**);
void buildGraph();
void buildPath(int*,int);
int getLargest(double*,int);
Edge* findEdge(int,int);
double getDist(int,Edge**);

// debug/utility prototypes
int indexOf(int,int*,int);
void removeVal(int,int*,int);
void destroyGraph();
void squashAnts(Ant**);
void printGraph();

Node **graph;

int main(void){
  srand(time(0));
  buildGraph();
//   printGraph();
  search();
  destroyGraph();

  return 0;
}

// perform ACO search on the graph for Iron Hills
// returns: nothing
void search(){

  int i;
  int cycles = 0;
  while(cycles++ < 25){
    Ant **ants = malloc(sizeof(Ant*)*COLONY_SIZE);
    for(i = 0; i < COLONY_SIZE; i++){
      ants[i] = tour(0);
      buildPath(ants[i]->path,ants[i]->pSize);
      printf("Path Length: %d\n",ants[i]->pLen);
    }
    decay();
    updatePheromone(ants);
    squashAnts(ants);
  }

}

// perform a tour for an ant
// parameters:
//  random - 1 to take random edge, 0 for normal choice
// returns: nothing
Ant* tour(int random){

  Ant *ant = malloc(sizeof(Ant));
  ant->path = malloc(sizeof(int)*32);
  ant->pSize = 0;
  ant->pLen = 0;

  int *curVisited = malloc(sizeof(int)*32);
  double *probs = malloc(sizeof(double)*16);

  int curNode = 0;
  int vSize = 0;
  int prevDist = 0;

  curVisited[vSize++] = curNode;
  ant->path[ant->pSize++] = curNode;

  while(curNode != 23){

//     printf("curNode: %s\n",graph[curNode]->name);

    Edge **curLinks = graph[curNode]->links;

    int j = 0;
    double sum = 0;
    int probSize = 0;

    while(curLinks[j] != NULL){
      if(indexOf(curLinks[j]->idx,curVisited,vSize) == -1 &&
         indexOf(curLinks[j]->idx,ant->path,ant->pSize) == -1){
//         printf("pheromone amount: %f\n",curLinks[j]->pheromone);
        double weightedPhero = pow(curLinks[j]->pheromone,ALPHA);
        double weightedDist = pow(curLinks[j]->dist,BETA);
        probs[probSize] = weightedPhero * weightedDist;
        sum += probs[probSize++];
      }
      else{
        probs[probSize++] = -1;
      }
      j++;
    }

    if(sum == 0){ // we visited all the nodes we can see
      ant->pSize--;
      ant->pLen -= getDist(curNode,graph[graph[curNode]->parent]->links);
      curVisited[vSize++] = curNode;
      curNode = ant->path[ant->pSize-1]; // go back up the path
      continue;
    }

    for(j = 0; j < probSize; j++){
      if(probs[j] != -1){
        probs[j] /= sum;
      }
    }

    int chosen;
    if(random){
      do{
        chosen = rand() % graph[curNode]->lSize;
      } while (probs[chosen] < 0);
    }
    else{
      chosen = getChoice(probs,probSize);
    }
    graph[curLinks[chosen]->idx]->parent = curNode;
    curNode = curLinks[chosen]->idx;
    ant->path[ant->pSize++] = curNode;
    ant->pLen += curLinks[chosen]->dist;

  }
//   printf("%d\n",ant->pLen);
  free(curVisited);
  free(probs);
  return ant;
}

// global decay of pheromone
// returns: nothing
void decay(){
  int i = 0;
  while(graph[i] != NULL){
    int j = 0;
    while(graph[i]->links[j] != NULL){
      graph[i]->links[j++]->pheromone *= (1-RHO);
    }
    i++;
  }
}

// update pheromone values on ant path
// parameters:
//  ants - colony of ants
// returns: nothing
void updatePheromone(Ant **ants){
  int i;
  for(i = 0; i < COLONY_SIZE; i++){
    Ant *curAnt = ants[i];
    double toAdd = (double)Q / curAnt->pLen;
    toAdd *= RHO;

    int j;
    for(j = 0; j < curAnt->pSize-1; j++){
      findEdge(curAnt->path[j],curAnt->path[j+1])->pheromone += toAdd;
    }
  }
}

// build the initial graph
// returns: nothing
void buildGraph(){

  char *line = malloc(DATA_LINE_SIZE);
  graph = malloc(sizeof(Node*)*32);
  int pos, idx, dist;
  char *name;
  Edge **links;
  int offset, bytes;

  FILE *input = fopen("data.txt","r");

  while(fgets(line,DATA_LINE_SIZE,input)){
    name = malloc(32);
    links = malloc(sizeof(Edge*)*16);
    sscanf(line,"%d:%[^:]%n",&pos,name,&offset);
    int i = 0;
    while(sscanf(line + offset,":%d,%d%n",&idx,&dist,&bytes)){
      offset += bytes;
      links[i] = malloc(sizeof(Edge));
      links[i]->idx = idx;
      links[i]->pheromone = 0.01;
      links[i++]->dist = dist;
    }
    links[i] = (Edge*)NULL;
    graph[pos] = malloc(sizeof(Node));
    graph[pos]->name = name;
    graph[pos]->lSize = i;
    graph[pos]->links = links;
    graph[pos]->parent = -1;
  }
  graph[++pos] = (Node*)NULL;
  free(line);
  fclose(input);
}

// build the final path
// parameters:
//  path - array of node indices
//  pSize - size of the path
// returns: nothing
void buildPath(int *path,int pSize){
  int i = 0;
  printf("\n%s ",graph[*path++]->name);
  for(i = 1; i < pSize; i++){
    printf("-> %s ",graph[*path++]->name);
  }
  printf("\n");
}

// find the index of the largest value in array
// parameters:
//  arr - array to find greatest value in
//  size - size of the list array
// returns: index of largest value in list
int getLargest(double *arr, int size){
  int max = 0;
  int i;

  for(i = 1; i < size; i++){
    (arr[i] > arr[max]) && (max = i);
  }
  return max;
}

// choose a value in probability array
// parameters:
//  probs - probability array
//  size - size of array
// returns - index of chosen value
int getChoice(double *probs, int size){
    int *sumArray = malloc(sizeof(int)*size);
    int last = 0;

    if(probs[0] == -1){ sumArray[0] = 0; }
    else{
      sumArray[0] = (int)(round(probs[0]*100));
      last = sumArray[0];
    }
    int i;
    for(i = 1; i < size; i++){
      if(probs[i] == -1){ sumArray[i] = 0; }
      else{
        sumArray[i] = (int)(round(probs[i]*100));
        sumArray[i] += last;
        last = sumArray[i];
      }
    }

    int r = getRand(99) + 1;
//     printf("r : %d\n",r);

    int acc = 0;
    for(i = 0; i < size; i++){
      acc += sumArray[i];
      if(acc >= r){
        free(sumArray);
        return i;
      }
    }
}

// find the edge between to nodes
// parameters:
//  parent - index of parent node
//  end - index of end node
// returns: the edge found
Edge* findEdge(int parent,int end){
  Edge **parentLinks = graph[parent]->links;

  int i = 0;
  while(parentLinks[i] != NULL){
    if(parentLinks[i]->idx == end){
      return parentLinks[i];
    }
    i++;
  }
}

// get the dist value between two nodes
// parameters:
//  curNode - end of edge to find
//  parentLinks - list of edges to compare for curNode
// returns: dist value
double getDist(int curNode, Edge **parentLinks){

  int i = 0;
  while(parentLinks[i] != NULL){
    if(parentLinks[i]->idx == curNode){
      return parentLinks[i]->dist;
    }
    i++;
  }

}

// get random number in range
// parameters:
//  max - top bound on number
// returns: number in bounds [0,max)
int getRand(int max){
  unsigned int x = (RAND_MAX + 1u) / max;
  unsigned int y = x * max;
  unsigned r;
  do{
    r = rand();
  } while( r >= y);
  return r / x;
}

// check array for given value
// parameters:
//  val - value to search for
//  arr - array to search through
//  size - size of array
// returns: index if found, -1 otherwise
int indexOf(int val, int* arr, int size){
  int i;
  for(i = 0; i < size; i++)
    if(arr[i] == val){ return i; }
  return -1;
}

// remove value from array
// parameters:
//  val - value to remove
//  arr - array to remove from
//  size - size of array
void removeVal(int val, int* arr, int size){
  int i = indexOf(val,arr,size);
  if(i != -1){
    for(i; i < size-1; i++){
      arr[i] = arr[i+1];
    }
  }
  else{
    printf("Value does not exist\n");
  }
}

// destroy the graph and everything it loves
// returns: nothing
void destroyGraph(){
  int i = 0;
  while(graph[i] != NULL){
    int j = 0;
    while(graph[i]->links[j] != NULL){
      free(graph[i]->links[j++]);
    }
    free(graph[i]->links);
    free(graph[i]->name);
    free(graph[i++]);
  }
  free(graph);
}

// destroy the ants
// parameters:
//  ants - ant array
void squashAnts(Ant **ants){
  int i;

  for(i = 0; i < COLONY_SIZE; i++){
    free(ants[i]->path);
    free(ants[i]);
  }
  free(ants);
}

// print out the graph
void printGraph(){
  int i = 0;
  Edge** conn;
  while(graph[i] != NULL){
    printf("Node Name: %s\n",graph[i]->name);
    printf("  Connections:\n");
    conn = graph[i++]->links;
    int j = 0;
    while(conn[j] != NULL){
      printf("    Name: %s\n",graph[conn[j]->idx]->name);
      printf("      Dist: %lf\n",conn[j]->dist);
      printf("      Phero: %lf\n",conn[j]->pheromone);
      j++;
    }
  }
}
