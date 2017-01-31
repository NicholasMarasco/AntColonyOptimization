typedef struct edge{
  int idx;
  double dist, pheromone;
} Edge;

typedef struct node{
  int lSize, parent;
  char *name;
  Edge **links;
} Node;

typedef struct ant{
  int pSize, pLen;
  int *path;
} Ant;
