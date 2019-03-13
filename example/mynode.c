/*
 * mynode.c
 *
 *  Created on: Nov 29, 2018
 *      Author: pp
 */

#include <stdio.h>
#include <stdlib.h>
#include "mynode.h"

mynode_t* mynode_alloc(int max_actual_num, int reduant_num)
{
	mynode_t* ret = (mynode_t*)calloc(1, sizeof(mynode_t));
	//memset(ret, 0, sizeof(node_t));
	myfec_init(&ret->fec_ctx, 100, reduant_num, 1440, max_actual_num);
	return ret;
}

void mynode_free(mynode_t* node)
{
	if (node)
	{
		free(node);
	}
}
