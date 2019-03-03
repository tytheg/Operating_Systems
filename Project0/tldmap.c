/*	Tyler Green
//	tgreen2
//	CIS 415 Project 0
//	
//	This is all my own work, except some organization and basics taken from the examples in lectures given by Joe Sventek. The basic organization and layout  of the BST was taken from the basic visualization of a binary search tree on visualgo.net/bn/bst
*/

#include "date.h"
#include "iterator.h"
#include "tldmap.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

typedef struct m_data
{
	TLDNode *root;
	long size;
}M_data;

struct tldnode
{
	TLDNode *left;
	TLDNode *right;
	TLDNode *parent;
	char *tld;
	long value;
};

static void kill_nodes(TLDNode *n)
{
	//set left and right nodes, then free the root node, and recursively call for left and right of each one
	TLDNode *lt = n->left;
	TLDNode *rt = n->right;
	free(n->tld);
	free(n);
	if(lt != NULL)
	{
		kill_nodes(lt);
	}
	if (rt != NULL)
	{
		kill_nodes(rt);
	}
}

static void t_destroy(const TLDMap *tld)
{
	//Destroy Nodes, then Data, then the map itslef
	M_data *tldata = (M_data *)tld->self;
	TLDNode * root = tldata->root;
	kill_nodes(root);
	free((tld->self));
	free((void *)tld);
}

static int insert(TLDNode *n, TLDNode *root)
{
	//check if the root is null, if it is then add it becaues nothing is there
	if (root == NULL)
	{
		root->tld = n->tld;
		return 1;
	}
	//if it is already there, return 0
	if (n->tld == root->tld)
		return  0;
	//compare the two tld's to decide where to insert the node in the BST
	if(strcmp(n->tld, root->tld) < 0)
	{
		//if it is less than the one comparing to, and the left node is not null, redo function with left as root node
		if (root->left != NULL)
			return insert(n, (TLDNode*)(root->left));
		//it's less than comparable, and th eleft is node, so insert it
		else
		{
			root->left = n;
			n->parent = root;
			return 1;
		}
	}
	//Repeat above steps, but to the right
	else
	{
		if (root->right != NULL)
			return insert(n, (TLDNode*)(root->right));
		else
		{
			root->right = n;
			n->parent = root;
			return 1;
		}
	}
}


static int t_insert(const TLDMap *tld, char *theTLD, long v)
{
	M_data *tldata = (M_data *)tld->self;		
	
	TLDNode *node = (TLDNode *)malloc(sizeof(TLDNode));
	if (node == NULL)
		return -1;

	node->left = NULL;
	node->right = NULL;
	node->parent = NULL;
	node->tld = strdup(theTLD);
	node->value = v;
	
	//If the root is null, insert it
	if (tldata->root == NULL)
	{
		tldata->root = node;
		tldata->size++;
		return 1;
	}
	//insert the node using helper function, if it works increment size, then return
	int j = insert(node, tldata->root);
	if (j == 1)
		tldata->size++;
	return j;
}




static int t_reassign(const TLDMap *tld, char *theTLD, long v)
{
	//Search for node and when found, reassign the value
	M_data *tldata = (M_data *)tld->self;
	
	TLDNode *n = tldata->root;
	int i;
	while(n != NULL)
	{
		i = (strcmp(theTLD, n->tld));
		//If the node exists, set the value = passed value
		if (i == 0)
		{
			n->value = v;		
			return 1;
		}
		if (i < 0)
			n = n->left;
		else
			n = n->right;
	}
	return 0;
}

//Search for Node, if it's there return 1, if not return 0
static int t_lookup(const TLDMap *tld, char *theTLD, long *v)
{
	M_data *tldata = (M_data *)tld->self;
	
	TLDNode *n = tldata->root;

	int i;

	while(n != NULL)
	{
		i = (strcmp(theTLD, n->tld));
		//If the node is there, set value
		if (i == 0)
		{
			*v = n->value;			
			return 1;
		}		
		if (i < 0)
			n = n->left;
		else
			n = n->right;
	}
	return 0;
}

static long arr_count = 0L;

static void to_array(TLDNode *n, void **arr)
{

	long i;
	long j;
	TLDNode *cmp = NULL;

	for (i = 0; i < arr_count; ++i)
	{
		//COMPARE THE VALUES SO WE CAN OUTPUT THE ARRAY IN THE CORRECT ORDER
		cmp = (TLDNode *)arr[i];
		j = (n->value - cmp->value);
		if(j == 0)
			j = strcmp(n->tld, cmp->tld);
		if (j < 0)
		{
			//IF WE ARE AT THE PLACE THE TLD SHOULD GO, MOVE THE INDEXES AFTERWARDS
			//FOREWARD THE SIZE OF A VOID*, SO THERE IS SPACE FOR NEW ENTRY
			memmove(&arr[i+1], &arr[i], ((arr_count - i)*sizeof(void*)));
			break;
		}
	}
	//SET THE NODE N AT THE RIGHT PLACE
	arr[i] = (void *)n;
	++arr_count;
}


static void add_node(TLDNode *n, void **arr)
{
	//check set left and right values, then add n to array. Call same function on left and rigtht
	TLDNode *lt = n->left;
	TLDNode *rt = n->right;
	to_array(n, arr);
	if(lt != NULL)
	{
		add_node(lt, arr);
	}
	if (rt != NULL)
	{
		add_node(rt, arr);
	}
}

static const Iterator *t_itCreate(const TLDMap *tld)
{
	M_data *data = (M_data *)tld->self;
	TLDNode *root = data->root;
	long sz = data->size;

	void **tmp = NULL;
	tmp = (void **)malloc(sizeof(void *) * sz);
	//recursively add and order all the nodes to an array
	add_node(root, tmp);

	const Iterator *iter = Iterator_create(sz, tmp);
	return iter;
}


char *TLDNode_tldname(TLDNode *node)
{
	return node->tld;
}

long TLDNode_count(TLDNode *node)
{
	return node->value;
}

static TLDMap template = {NULL, t_destroy, t_insert, t_reassign, t_lookup, t_itCreate};

const TLDMap *TLDMap_create(void)
{
	TLDMap *t = NULL;
	M_data *data = NULL;
	t = (TLDMap *)malloc(sizeof(TLDMap));
	if (t != NULL)
	{
		data = (M_data *)malloc(sizeof(M_data));
		if (data != NULL)
		{
			data->root = NULL;
			data->size = 0;
			*t = template;
			t->self = (void *)data;	
		}
		else
		{
			free(t);
			return NULL;
		}
	}
	return t;
}

