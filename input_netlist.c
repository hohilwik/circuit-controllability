/*
INPUTS <number of input lines>
1 2 3 4 5 // numbering for every input line
OUTPUTS <number of output lines>
11 12 13 14 15 // number for every output line
NODES <number of nodes>
1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 // list of every node
GATES <number of gates>
<gatetype> <number of inputs> <list of input lines> <output lines>
AND 3 1 2 3 11
OR 2 2 3 6
FANOUT 3 3 7 8 9 // this has 1 input [3] and 3 outputs [7, 8, 9]
// etc etc
*/


// sigh, the blocks need to numbered, don't they

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>

#define MAXVAL 200000

typedef struct Vector{
	int capacity, size;
	int* arr;
} Vector;

typedef struct Block{
	int type, num_input, num_output;
	Vector inputlines, outputlines;
	Vector inputLoc, outputLoc;

	// type
	// 1 AND
	// 2 OR
	// 3 NOT
	// 4 NAND
	// 5 NOR
	// 6 XOR
	// 7 FANOUT
	// 8 FLIPFLOP
} Block;

typedef struct Node{
	int id;
	int type;
	// 1 for PI
	// 2 for PO
	// 0 for all others
	int drv_out; // block it is the output of
	Vector drv_in; // blocks it is an input for
	int zeroCombiControl, oneCombiControl, CombiObserve;
	int zeroSeqControl, oneSeqControl, SeqObserve;
} Node;


int vector_init(Vector *v)
{
	int Cap = 5;
	v->capacity = Cap;
	v->size = 0;
	v->arr = (int*)malloc(Cap*sizeof(int));
	return v->capacity;
}

int vector_insert(Vector *v, int val)
{
		v->size++;
		int size=v->size;
		if(v->size==v->capacity)
		{
			v->capacity = 2*v->capacity;
			int Cap = v->capacity;
			/*
			int *temp = (int*)malloc(Cap*sizeof(int));
			for(int iter=0; iter<(size-1); iter++)
			{
				temp[iter] = v->arr[iter]; 
			}
			//free(v->arr);
			v->arr = temp;
			*/
			v->arr = (int*)realloc(v->arr, sizeof(int)*(Cap) );
		}
		v->arr[size-1] = val;
		return v->capacity;
}

int sum_vector(Vector *v)
{
	int size=v->size;
	int sum=0;
	for(int i=0; i<size; i++)
	{
		sum=sum+v->arr[i];
	}
	return sum;
}

int min_vector(Vector *v)
{
	int size=v->size;
	if(size==0) 
	{
		return MAXVAL;
	}
	int min=v->arr[0];
	for(int i=0; i<size; i++)
	{
		if(v->arr[i]<min)
		{
			min = v->arr[i];
		}
	}
	return min;
}

int compare_vector(Vector *v1, Vector *v2)
{
	if(v1->size!=v2->size)
	{
		return -1;
	}
	int size=v1->size;
	for(int i=0; i<size; i++)
	{
		if(v1->arr[i]!=v2->arr[i])
		{
			return -1;
		}
	}
	return 0;
}

int block_init(Block *b)
{
	vector_init(&b->inputlines);
	vector_init(&b->outputlines);
	vector_init(&b->inputLoc);
	vector_init(&b->outputLoc);
	return 0;
}

int node_init(Node *n)
{
	vector_init(&n->drv_in);
	n->id = -1;
	n->type = 0;
	n->drv_out = -1;
	n->zeroCombiControl = MAXVAL;
	n->oneCombiControl =  MAXVAL;
	n->CombiObserve = MAXVAL;
	n->zeroSeqControl = MAXVAL;
	n->oneSeqControl = MAXVAL;
	n->SeqObserve = MAXVAL;
	return 0;
}

int findZeroCombiControl(int nodenum, Node* nodeblock, Block* gates)
{
	int val = nodeblock[nodenum-1].zeroCombiControl;
	if(val<MAXVAL)
	{
		return val;
	}
	int gateout = nodeblock[nodenum-1].drv_out;
	if(gateout==-1)
	{
		return MAXVAL;
	}
	int gatetype = gates[gateout].type;
	Vector inlines = gates[gateout].inputlines;
	Vector outlines = gates[gateout].outputlines;
	Vector invals;
	vector_init(&invals);
	Vector invalsOne;
	vector_init(&invalsOne);
	int size = inlines.size;
	for(int i=0; i<size; i++)
	{
		int controlvalZero = findZeroCombiControl(inlines.arr[i], nodeblock, gates);
		nodeblock[inlines.arr[i]-1].zeroCombiControl = controlvalZero;
		vector_insert(&invals, controlvalZero);
		
		int controlvalOne = findOneCombiControl(inlines.arr[i], nodeblock, gates);
		nodeblock[inlines.arr[i]-1].oneCombiControl = controlvalOne;
		vector_insert(&invalsOne, controlvalOne);
	}
	// type
	// 1 AND min(all inputs zeroControl)+1
	// 2 OR sum(all inputs zeroControl)+1
	// 3 NOT (input oneControl)+1
	// 4 NAND sum(all inputs oneControl)+1
	// 5 NOR min(all inputs oneControl)+1
	// 6 XOR min( sum of all inputs zeroControl, sum of all inputs OneControl)+1
	// 7 FANOUT (propagate input control to all outputs)
	int controlOut;
	if(gatetype==1)
	{
		controlOut = min_vector(&invals)+1;
	}
	else if(gatetype==2)
	{
		controlOut = sum_vector(&invals)+1;
	}
	else if(gatetype==3)
	{
		controlOut = min_vector(&invalsOne)+1;
	}
	else if(gatetype==4)
	{
		controlOut = sum_vector(&invalsOne)+1;
	}
	else if(gatetype==5)
	{
		controlOut = min_vector(&invalsOne)+1;
	}
	else if(gatetype==6)
	{
		int v1 = sum_vector(&invals);
		int v2 = sum_vector(&invalsOne);
		int vfinal = (v1<v2)?v1:v2;
		controlOut = vfinal+1;
	}
	else if(gatetype==7)
	{
		controlOut = min_vector(&invals);
		int fanout = outlines.size;
		for(int i=0; i<fanout; i++)
		{
			if(nodeblock[ outlines.arr[i]-1 ].zeroCombiControl > controlOut)
			{
				nodeblock[ outlines.arr[i]-1 ].zeroCombiControl = controlOut;
			}
		}
	}
	else if(gatetype==8)
	{
		controlOut = 0;
	}
	// CLK and RST are assumed to have 0 CC0, CC1
	nodeblock[nodenum-1].zeroCombiControl = controlOut;
	//free(inlines.arr);
	//free(outlines.arr);
	//free(invals.arr);
	//free(invalsOne.arr);
	//printf("%d %d %d\n", nodeblock[nodenum].id, nodeblock[nodenum].zeroCombiControl, nodeblock[nodenum].oneCombiControl);
	return controlOut;
	
}

int findOneCombiControl(int nodenum, Node* nodeblock, Block* gates)
{
	int val = nodeblock[nodenum-1].oneCombiControl;
	if(val<MAXVAL)
	{
		return val;
	}
	int gateout = nodeblock[nodenum-1].drv_out;
	if(gateout==-1)
	{
		return MAXVAL;
	}
	int gatetype = gates[gateout].type;
	Vector inlines = gates[gateout].inputlines;
	Vector outlines = gates[gateout].outputlines;
	Vector invals;
	vector_init(&invals);
	Vector invalsOne;
	vector_init(&invalsOne);
	int size = inlines.size;
	for(int i=0; i<size; i++)
	{
		int controlvalZero = findZeroCombiControl(inlines.arr[i], nodeblock, gates);
		nodeblock[inlines.arr[i]-1].zeroCombiControl = controlvalZero;
		vector_insert(&invals, controlvalZero);
		
		int controlvalOne = findOneCombiControl(inlines.arr[i], nodeblock, gates);
		nodeblock[inlines.arr[i]-1].oneCombiControl = controlvalOne;
		vector_insert(&invalsOne, controlvalOne);
	}
	// type
	// 1 AND sum(all inputs oneControl)+1
	// 2 OR min(all inputs oneControl)+1
	// 3 NOT (input zeroControl)+1
	// 4 NAND min(all inputs zeroControl)+1
	// 5 NOR sum(all inputs zeroControl)+1
	// 6 XOR min( sum of all inputs zeroControl, sum of all inputs oneControl)+1
	// 7 FANOUT (propagate input control to all outputs)
	int controlOut;
	if(gatetype==1)
	{
		controlOut = sum_vector(&invalsOne)+1;
	}
	else if(gatetype==2)
	{
		controlOut = min_vector(&invalsOne)+1;
	}
	else if(gatetype==3)
	{
		controlOut = min_vector(&invals)+1;
	}
	else if(gatetype==4)
	{
		controlOut = min_vector(&invals)+1;
	}
	else if(gatetype==5)
	{
		controlOut = sum_vector(&invals)+1;
	}
	else if(gatetype==6)
	{
		int v1 = sum_vector(&invals);
		int v2 = sum_vector(&invalsOne);
		int vfinal = (v1<v2)?v1:v2;
		controlOut = vfinal+1;
	}
	else if(gatetype==7)
	{
		controlOut = min_vector(&invals);
		int fanout = outlines.size;
		for(int i=0; i<fanout; i++)
		{
			if(nodeblock[ outlines.arr[i]-1 ].oneCombiControl > controlOut)
			{
				nodeblock[ outlines.arr[i]-1 ].oneCombiControl = controlOut;
			}
		}
	}
	else if(gatetype==8)
	{
		controlOut = min_vector(&invalsOne);
	}
	// CLK and RST are assumed to have 0 CC0, CC1
	nodeblock[nodenum-1].oneCombiControl = controlOut;
	//printf("%d %d %d\n", nodeblock[nodenum].id, nodeblock[nodenum].zeroCombiControl, nodeblock[nodenum].oneCombiControl);
	//free(inlines.arr);
	//free(outlines.arr);
	//free(invals.arr);
	//free(invalsOne.arr);
	return controlOut;
	
}

//sequential
int findZeroSeqControl(int nodenum, Node* nodeblock, Block* gates)
{
	int val = nodeblock[nodenum-1].zeroSeqControl;
	if(val<MAXVAL)
	{
		return val;
	}
	int gateout = nodeblock[nodenum-1].drv_out;
	if(gateout==-1)
	{
		return MAXVAL;
	}
	int gatetype = gates[gateout].type;
	Vector inlines = gates[gateout].inputlines;
	Vector outlines = gates[gateout].outputlines;
	Vector invals;
	vector_init(&invals);
	Vector invalsOne;
	vector_init(&invalsOne);
	int size = inlines.size;
	for(int i=0; i<size; i++)
	{
		int controlvalZero = findZeroSeqControl(inlines.arr[i], nodeblock, gates);
		nodeblock[inlines.arr[i]-1].zeroSeqControl = controlvalZero;
		vector_insert(&invals, controlvalZero);
		
		int controlvalOne = findOneSeqControl(inlines.arr[i], nodeblock, gates);
		nodeblock[inlines.arr[i]-1].oneSeqControl = controlvalOne;
		vector_insert(&invalsOne, controlvalOne);
	}
	// type
	// 1 AND min(all inputs zeroControl)
	// 2 OR sum(all inputs zeroControl)
	// 3 NOT (input oneControl)
	// 4 NAND sum(all inputs oneControl)
	// 5 NOR min(all inputs oneControl)
	// 6 XOR min( sum of all inputs zeroControl, sum of all inputs OneControl)
	// 7 FANOUT (propagate input control to all outputs)
	int controlOut;
	if(gatetype==1)
	{
		controlOut = min_vector(&invals);
	}
	else if(gatetype==2)
	{
		controlOut = sum_vector(&invals);
	}
	else if(gatetype==3)
	{
		controlOut = min_vector(&invalsOne);
	}
	else if(gatetype==4)
	{
		controlOut = sum_vector(&invalsOne);
	}
	else if(gatetype==5)
	{
		controlOut = min_vector(&invalsOne);
	}
	else if(gatetype==6)
	{
		int v1 = sum_vector(&invals);
		int v2 = sum_vector(&invalsOne);
		int vfinal = (v1<v2)?v1:v2;
		controlOut = vfinal;
	}
	else if(gatetype==7)
	{
		controlOut = min_vector(&invals);
		int fanout = outlines.size;
		for(int i=0; i<fanout; i++)
		{
			if(nodeblock[ outlines.arr[i]-1 ].zeroSeqControl > controlOut)
			{
				nodeblock[ outlines.arr[i]-1 ].zeroSeqControl = controlOut;
			}
		}
	}
	else if(gatetype==8)
	{
		controlOut = 0;
	}
	// CLK and RST are assumed to have 0 CC0, CC1
	nodeblock[nodenum-1].zeroSeqControl = controlOut;
	//free(inlines.arr);
	//free(outlines.arr);
	//free(invals.arr);
	//free(invalsOne.arr);
	return controlOut;
	
}

int findOneSeqControl(int nodenum, Node* nodeblock, Block* gates)
{
	int val = nodeblock[nodenum-1].oneSeqControl;
	if(val<MAXVAL)
	{
		return val;
	}
	int gateout = nodeblock[nodenum-1].drv_out;
	if(gateout==-1)
	{
		return MAXVAL;
	}
	int gatetype = gates[gateout].type;
	Vector inlines = gates[gateout].inputlines;
	Vector outlines = gates[gateout].outputlines;
	Vector invals;
	vector_init(&invals);
	Vector invalsOne;
	vector_init(&invalsOne);
	int size = inlines.size;
	for(int i=0; i<size; i++)
	{
		int controlvalZero = findZeroSeqControl(inlines.arr[i], nodeblock, gates);
		nodeblock[inlines.arr[i]-1].zeroSeqControl = controlvalZero;
		vector_insert(&invals, controlvalZero);
		
		int controlvalOne = findOneSeqControl(inlines.arr[i], nodeblock, gates);
		nodeblock[inlines.arr[i]-1].oneSeqControl = controlvalOne;
		vector_insert(&invalsOne, controlvalOne);
	}
	// type
	// 1 AND sum(all inputs oneControl)
	// 2 OR min(all inputs oneControl)
	// 3 NOT (input zeroControl)
	// 4 NAND min(all inputs zeroControl)
	// 5 NOR sum(all inputs zeroControl)
	// 6 XOR min( sum of all inputs zeroControl, sum of all inputs oneControl)
	// 7 FANOUT (propagate input control to all outputs)
	int controlOut;
	if(gatetype==1)
	{
		controlOut = sum_vector(&invalsOne);
	}
	else if(gatetype==2)
	{
		controlOut = min_vector(&invalsOne);
	}
	else if(gatetype==3)
	{
		controlOut = min_vector(&invals);
	}
	else if(gatetype==4)
	{
		controlOut = min_vector(&invals);
	}
	else if(gatetype==5)
	{
		controlOut = sum_vector(&invals);
	}
	else if(gatetype==6)
	{
		int v1 = sum_vector(&invals);
		int v2 = sum_vector(&invalsOne);
		int vfinal = (v1<v2)?v1:v2;
		controlOut = vfinal;
	}
	else if(gatetype==7)
	{
		controlOut = min_vector(&invals);
		int fanout = outlines.size;
		for(int i=0; i<fanout; i++)
		{
			if(nodeblock[ outlines.arr[i]-1 ].zeroSeqControl > controlOut)
			{
				nodeblock[ outlines.arr[i]-1 ].zeroSeqControl = controlOut;
			}
		}
	}
	else if(gatetype==8)
	{
		controlOut = min_vector(&invalsOne);
	}
	// CLK and RST are assumed to have 0 CC0, CC1
	nodeblock[nodenum-1].oneSeqControl = controlOut;
	//free(inlines.arr);
	//free(outlines.arr);
	//free(invals.arr);
	//free(invalsOne.arr);
	return controlOut;
	
}

// observability
int findCombiObserve(int nodenum, Node* nodeblock, Block* gates)
{
	int val = nodeblock[nodenum-1].CombiObserve;
	if(val<MAXVAL)
	{
		return val;
	}
int obsStart = MAXVAL;
int observeOut = MAXVAL;
int iterlen = nodeblock[nodenum-1].drv_in.size;
for(int iter=0; iter<iterlen; iter++)
{
	int gateout = nodeblock[nodenum-1].drv_in.arr[iter];
	if(gateout==-1)
	{
		return MAXVAL;
	}
	int gatetype = gates[gateout].type;
	Vector inlines = gates[gateout].inputlines;
	Vector outlines = gates[gateout].outputlines;
	Vector invals;
	vector_init(&invals);
	Vector invalsOne;
	vector_init(&invalsOne);
	Vector outObserve;
	vector_init(&outObserve);
	int size = inlines.size;
	for(int i=0; i<size; i++)
	{
		if(inlines.arr[i]==nodenum)
		{
			continue;
		}
		int controlvalZero = findZeroCombiControl(inlines.arr[i], nodeblock, gates);
		nodeblock[inlines.arr[i]-1].zeroCombiControl = controlvalZero;
		vector_insert(&invals, controlvalZero);
		
		int controlvalOne = findOneCombiControl(inlines.arr[i], nodeblock, gates);
		nodeblock[inlines.arr[i]-1].oneCombiControl = controlvalOne;
		vector_insert(&invalsOne, controlvalOne);
	}
	int outnum=outlines.size;
	for(int i=0; i<outnum; i++)
	{
		vector_insert(&outObserve, findCombiObserve(outlines.arr[i], nodeblock, gates) );
	}
	// type
	// 1 AND min(outObserve)+sum(input oneControl)+1
	// 2 OR min(outObserve)+sum(input zeroControl)+1
	// 3 NOT min(outObserve)+1
	// 4 NAND min(outObserve)+sum(input oneControl)+1
	// 5 NOR min(outObserve)+sum(input zeroControl)+1
	// 6 XOR min(outObserve)+min( sum(input oneControl), sum(input zeroControl) )+1
	// 7 FANOUT min(outObserve)
	if(gatetype==1)
	{
		observeOut = min_vector(&outObserve)+sum_vector(&invalsOne)+1;
	}
	else if(gatetype==2)
	{
		observeOut = min_vector(&outObserve)+sum_vector(&invals)+1;
	}
	else if(gatetype==3)
	{
		observeOut = min_vector(&outObserve)+1;
	}
	else if(gatetype==4)
	{
		observeOut = min_vector(&outObserve)+sum_vector(&invalsOne)+1;
	}
	else if(gatetype==5)
	{
		observeOut = min_vector(&outObserve)+sum_vector(&invals)+1;
	}
	else if(gatetype==6)
	{
		int v1 = sum_vector(&invals);
		int v2 = sum_vector(&invalsOne);
		int vfinal = (v1<v2)?v1:v2;
		observeOut = min_vector(&outObserve)+vfinal+1;
	}
	else if(gatetype==7)
	{
		observeOut = min_vector(&outObserve);
	}
	else if(gatetype==8)
	{
		observeOut = min_vector(&outObserve)+findZeroCombiControl(nodenum, nodeblock, gates)+findOneCombiControl(nodenum, nodeblock, gates);
	}
	// CLK and RST are assumed to have 0 CC0, CC1
	obsStart = (obsStart<observeOut)?obsStart:observeOut;
}
	observeOut = obsStart;
	nodeblock[nodenum-1].CombiObserve = observeOut;
	//free(inlines.arr);
	//free(outlines.arr);
	//free(invals.arr);
	//free(invalsOne.arr);
	//printf("%d %d %d\n", nodeblock[nodenum].id, nodeblock[nodenum].zeroCombiControl, nodeblock[nodenum].oneCombiControl);
	return observeOut;
	
}

int findSeqObserve(int nodenum, Node* nodeblock, Block* gates)
{
	int val = nodeblock[nodenum-1].CombiObserve;
	if(val<MAXVAL)
	{
		return val;
	}
int obsStart = MAXVAL;
int observeOut = MAXVAL;
int iterlen = nodeblock[nodenum-1].drv_in.size;
for(int iter=0; iter<iterlen; iter++)
{
	int gateout = nodeblock[nodenum-1].drv_in.arr[iter];
	if(gateout==-1)
	{
		return MAXVAL;
	}
	int gatetype = gates[gateout].type;
	Vector inlines = gates[gateout].inputlines;
	Vector outlines = gates[gateout].outputlines;
	Vector invals;
	vector_init(&invals);
	Vector invalsOne;
	vector_init(&invalsOne);
	Vector outObserve;
	vector_init(&outObserve);
	int size = inlines.size;
	for(int i=0; i<size; i++)
	{
		if(inlines.arr[i]==nodenum)
		{
			continue;
		}
		int controlvalZero = findZeroCombiControl(inlines.arr[i], nodeblock, gates);
		nodeblock[inlines.arr[i]-1].zeroCombiControl = controlvalZero;
		vector_insert(&invals, controlvalZero);
		
		int controlvalOne = findOneCombiControl(inlines.arr[i], nodeblock, gates);
		nodeblock[inlines.arr[i]-1].oneCombiControl = controlvalOne;
		vector_insert(&invalsOne, controlvalOne);
	}
	int outnum=outlines.size;
	for(int i=0; i<outnum; i++)
	{
		vector_insert(&outObserve, findCombiObserve(outlines.arr[i], nodeblock, gates) );
	}
	// type
	// 1 AND min(outObserve)+sum(input oneControl)+1
	// 2 OR min(outObserve)+sum(input zeroControl)+1
	// 3 NOT min(outObserve)+1
	// 4 NAND min(outObserve)+sum(input oneControl)+1
	// 5 NOR min(outObserve)+sum(input zeroControl)+1
	// 6 XOR min(outObserve)+min( sum(input oneControl), sum(input zeroControl) )+1
	// 7 FANOUT min(outObserve)
	if(gatetype==1)
	{
		observeOut = min_vector(&outObserve)+sum_vector(&invalsOne);
	}
	else if(gatetype==2)
	{
		observeOut = min_vector(&outObserve)+sum_vector(&invals);
	}
	else if(gatetype==3)
	{
		observeOut = min_vector(&outObserve);
	}
	else if(gatetype==4)
	{
		observeOut = min_vector(&outObserve)+sum_vector(&invalsOne);
	}
	else if(gatetype==5)
	{
		observeOut = min_vector(&outObserve)+sum_vector(&invals);
	}
	else if(gatetype==6)
	{
		int v1 = sum_vector(&invals);
		int v2 = sum_vector(&invalsOne);
		int vfinal = (v1<v2)?v1:v2;
		observeOut = min_vector(&outObserve)+vfinal;
	}
	else if(gatetype==7)
	{
		observeOut = min_vector(&outObserve);
	}
	else if(gatetype==8)
	{
		observeOut = min_vector(&outObserve)+findZeroCombiControl(nodenum, nodeblock, gates)+findOneCombiControl(nodenum, nodeblock, gates)+1;
	}
	// CLK and RST are assumed to have 0 CC0, CC1
	obsStart = (obsStart<observeOut)?obsStart:observeOut;
}
	observeOut = obsStart;
	nodeblock[nodenum-1].CombiObserve = observeOut;
	//free(inlines.arr);
	//free(outlines.arr);
	//free(invals.arr);
	//free(invalsOne.arr);
	//printf("%d %d %d\n", nodeblock[nodenum].id, nodeblock[nodenum].zeroCombiControl, nodeblock[nodenum].oneCombiControl);
	return observeOut;
	
}

int inputGraph(Vector *argInputs, int *inputnum, Vector *argOutputs, int *outputnum, Vector *argNodes, int *nodenum, Block *argBlocks, int *gatenum)
{
//printf("start\n");
	FILE *fp;
	char buff[255];
	fp = fopen("input_cir.txt", "r");

	if(!fp)
	{
		printf("Input file not found. Check name\n");
	}
//printf("test1\n");
	int input_num, output_num, node_num, gate_num;

	fscanf(fp, "%s", buff);

	if( strcmp(buff, "INPUTS")!=0 )
	{
		printf("Incorrect input format. Input header not found\n");
		return -1;
	}

	fscanf(fp, "%d", &input_num);

	if(input_num<=0)
	{
		printf("Input_num error");
		return -1;
	}

	Vector inputs;
	vector_init(&inputs);
//printf("test2\n");
	int in_node;

	for(int i=0; i<input_num; i++)
	{
		fscanf(fp, "%d", &in_node);
		vector_insert(&inputs, in_node);
	}

	fscanf(fp, "%s", buff);

	if( strcmp(buff, "OUTPUTS")!=0 )
	{
		printf("Incorrect input format. Output header not found\n");
		return -1;
	}

	fscanf(fp, "%d", &output_num);

	if(output_num<=0)
	{
		printf("Output_num error");
		return -1;
	}
//printf("test2.1\n");
	Vector outputs;
	vector_init(&outputs);
//printf("test3\n");

	int out_node;

	for(int i=0; i<output_num; i++)
	{
		fscanf(fp, "%d", &out_node);
		vector_insert(&outputs, out_node);
	}
	
//printf("test4\n");
	fscanf(fp, "%s", buff);
	
	if( strcmp(buff, "NODES")!=0 )
	{
		printf("Incorrect input format. Nodes header not found\n");
		return -1;
	}
	
	fscanf(fp, "%d", &node_num);

	if(output_num<=0)
	{
		printf("nodes_num error");
		return -1;
	}
	
	Vector nodes;
	vector_init(&nodes);
	
//printf("test5\n");
	
	int nodeindex;
	
	for(int i=0; i<node_num; i++)
	{
		fscanf(fp, "%d", &nodeindex);
		if(nodeindex>(node_num+1))
		{
			printf("Node index cannot be larger than node_num");
			exit(0);
		}
		vector_insert(&nodes, nodeindex);
	}
	
//printf("test6");
	
	Node *nodeblock = (Node*)malloc(sizeof(Node)*node_num);
	
	for(int z=0; z<node_num; z++)
	{
		node_init(&nodeblock[z]);
	}
	
	for(int z=0; z<node_num; z++)
	{
		int tempval = nodes.arr[z];
		nodeblock[ tempval-1 ].id = tempval;
	}
	
	
//printf("%d\n", nodes.size);

	fscanf(fp, "%s", buff);
	
	if( strcmp(buff, "GATES")!=0 )
	{
		printf("Incorrect input format. Gates header not found\n");
		return -1;
	}
	
	fscanf(fp, "%d", &gate_num);

	if(gate_num<=0)
	{
		printf("gates_num error");
		return -1;
	}
	
	Block *gates;
	gates = (Block*)malloc(gate_num*sizeof(Block));
	
	for(int z=0; z<gate_num; z++)
	{
		block_init(&gates[z]);
	}
	
	for(int z=0; z<gate_num; z++)
	{
		int type=MAXVAL, input_num=-1, output_num=-1;
		fscanf(fp, "%s", buff);
		//printf("%s ", buff); 
		if( strcmp(buff, "AND")==0)
		{
			type = 1;
			output_num = 1;
		}
		else if(strcmp(buff, "OR")==0)
		{
			type = 2;
			output_num = 1;
		}
		else if(strcmp(buff, "NOT")==0)
		{
			type = 3;
			output_num = 1;
		}
		else if(strcmp(buff, "NAND")==0)
		{
			type = 4;
			output_num = 1;
		}
		else if(strcmp(buff, "NOR")==0)
		{
			type = 5;
			output_num = 1;
		}
		else if(strcmp(buff, "XOR")==0)
		{
			type = 6;
			output_num = 1;
		}
		else if(strcmp(buff, "FANOUT")==0)
		{
			type = 7;
			input_num = 1;
		}
		else if(strcmp(buff, "FLIPFLOP")==0)
		{
			type = 8;
			input_num = 1;
			output_num = 1;
		}
		else if(type==MAXVAL)
		{
			printf("Invalid gate type");
			return -1;
		}
		
		gates[z].type = type;
		
		if(type==7)
		{
			fscanf(fp, "%d", &output_num);
		}
		if(type<=6)
		{
			fscanf(fp, "%d", &input_num);
		}
		
		gates[z].num_output = output_num;
		gates[z].num_input = input_num;
		
		for(int in=0; in<input_num; in++)
		{
			int inval;
			fscanf(fp, "%d", &inval);
			vector_insert(&gates[z].inputlines, inval);
			vector_insert(&nodeblock[inval-1].drv_in, z);
		}
			
		for(int out=0; out<output_num; out++)
		{
			int outval;
			fscanf(fp, "%d", &outval);
			vector_insert(&gates[z].outputlines, outval);
			nodeblock[ outval-1 ].drv_out = z;
		}
		
	}
	
//printf("test01\n");
	
	int PInum = inputs.size;
	int POnum = outputs.size;
	int numnodes = nodes.size;
	
	for(int i=0; i<numnodes; i++)
	{
		nodeblock[ nodes.arr[i]-1 ].type = 0;
	}
	for(int i=0; i<PInum; i++)
	{
		nodeblock[ inputs.arr[i]-1 ].type = 1;
		nodeblock[ inputs.arr[i]-1 ].zeroCombiControl = 0;
		nodeblock[ inputs.arr[i]-1 ].oneCombiControl = 0;
		nodeblock[ inputs.arr[i]-1 ].zeroSeqControl = 0;
		nodeblock[ inputs.arr[i]-1 ].oneSeqControl = 0;
		
	}
	for(int i=0; i<POnum; i++)
	{
		nodeblock[ outputs.arr[i]-1 ].type = 2;
		nodeblock[ outputs.arr[i]-1 ].CombiObserve = 0;
		nodeblock[ outputs.arr[i]-1 ].SeqObserve = 0;
	}
	
//printf("test02\n");
	
	Vector frontier = inputs;
	int frontsize = frontier.size;
	while(frontsize!=0)
	{
		Vector newfront, newblocklist;
		vector_init(&newfront);
		for(int i=0; i<frontsize; i++)
		{
			if( nodeblock[frontier.arr[i]-1].type!=2 )
			{
			Vector drvin = nodeblock[ frontier.arr[i]-1 ].drv_in;
			int numblocks = drvin.size;
//printf("test03\n");
			for(int j=0; j<numblocks; j++)
			{
				int addsize = gates[ drvin.arr[j] ].outputlines.size;
				for(int k=0; k<addsize; k++)
				{
					int nodeval = gates[ drvin.arr[j] ].outputlines.arr[k];
//printf("test04\n");
					if(nodeval == 0)
					{
						continue;
					}
					
					if( nodeblock[nodeval-1].type==0 )
					{
						vector_insert(&newfront, nodeval);
//printf("test05\n");
					}
				}
			}
			//free(frontier.arr);
			vector_init(&frontier);
			//free(newfront.arr);
			//free(drvin.arr);
			}
//printf("test06\n");
			frontier = newfront;
			frontsize = frontier.size;
			
			for(int j=0; j<frontsize; j++)
			{
				nodeblock[frontier.arr[j]-1].zeroCombiControl = findZeroCombiControl(frontier.arr[j], nodeblock, gates);
				nodeblock[frontier.arr[j]-1].oneCombiControl = findOneCombiControl(frontier.arr[j], nodeblock, gates);
				nodeblock[frontier.arr[j]-1].zeroSeqControl = findZeroSeqControl(frontier.arr[j], nodeblock, gates);
				nodeblock[frontier.arr[j]-1].oneSeqControl = findOneSeqControl(frontier.arr[j], nodeblock, gates);
			}
		}
	}
	
	printf("\nCombinational Controllability\n");
	for(int i=0; i<numnodes; i++)
	{
		nodeblock[i].zeroCombiControl = findZeroCombiControl(nodeblock[i].id, nodeblock, gates);
		nodeblock[i].oneCombiControl = findOneCombiControl(nodeblock[i].id, nodeblock, gates);
		
		printf("%d %d %d\n", nodeblock[i].id, nodeblock[i].zeroCombiControl, nodeblock[i].oneCombiControl);
	}
	printf("\nSequential Controllability\n");
	for(int i=0; i<numnodes; i++)
	{
		nodeblock[i].zeroSeqControl = findZeroSeqControl(nodeblock[i].id, nodeblock, gates);
		nodeblock[i].oneSeqControl = findOneSeqControl(nodeblock[i].id, nodeblock, gates);
		
		printf("%d %d %d\n", nodeblock[i].id, nodeblock[i].zeroSeqControl, nodeblock[i].oneSeqControl);
	}
	printf("\nCombinational Observability\n");
	for(int i=0; i<numnodes; i++)
	{
		nodeblock[i].CombiObserve = findCombiObserve(nodeblock[i].id, nodeblock, gates);
		
		printf("%d %d\n", nodeblock[i].id, nodeblock[i].CombiObserve);
	}

	Vector observevec, oldvec;
	vector_init(&observevec);
	vector_init(&oldvec);
	for(int i=0; i<numnodes; i++)
	{
		nodeblock[i].SeqObserve = findSeqObserve(nodeblock[i].id, nodeblock, gates);
		vector_insert(&observevec, nodeblock[i].SeqObserve);
	}
	oldvec = observevec;
	vector_init(&observevec);
		
	while(compare_vector(&observevec, &oldvec)!=0)
	{
		oldvec = observevec;
		vector_init(&observevec);
		for(int i=0; i<numnodes; i++)
		{
			nodeblock[i].SeqObserve = findSeqObserve(nodeblock[i].id, nodeblock, gates);
			vector_insert(&observevec, nodeblock[i].SeqObserve);
		}
		
	}
	printf("\nSequential Observability\n");
	for(int i=0; i<numnodes; i++)
	{	
		printf("%d %d\n", nodeblock[i].id, nodeblock[i].SeqObserve);
	}
	
	fclose(fp);
	return 0;

}

int main()
{
	int *inputnum, *outputnum, *nodenum, *gatenum;
	Vector *argInputs, *argOutputs, *argNodes;
	Block *argBlocks;
	for(int i=0; i<1; i++)
	{
	int r=inputGraph(argInputs, inputnum, argOutputs, outputnum, argNodes, nodenum, argBlocks, gatenum);
		if(r==-1)
		{
			exit(0);
		}
	}
	return 0;
}