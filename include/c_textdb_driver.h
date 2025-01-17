/*
 * c_textdb_driver.h
 *
 *  Created on: Nov 11, 2012
 *      Author: vbonnici
 */
/*
Copyright (c) 2014 by Rosalba Giugno

This library contains portions of other open source products covered by separate
licenses. Please see the corresponding source files for specific terms.

RI is provided under the terms of The MIT License (MIT):

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef C_TEXTDB_DRIVER_H_
#define C_TEXTDB_DRIVER_H_

#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Graph.h"

using namespace std;
using namespace rilib;

enum GRAPH_FILE_TYPE
{
	GFT_GFU,
	GFT_GFD,
	GFT_EGFU,
	GFT_EGFD,
	GFT_VFU,
	GFT_LAD,
	GFT_3GO,
	GFT_REG
};

#define STR_READ_LENGTH 256

int read_gfu(const char *fileName, FILE *fd, Graph *graph);
int read_gfd(const char *fileName, FILE *fd, Graph *graph);
int read_vfu(const char *fileName, FILE *fd, Graph *graph);
int read_lad(const char *fileName, FILE *fd, Graph *graph);
int read_egfu(const char *fileName, FILE *fd, Graph *graph);
int read_egfd(const char *fileName, FILE *fd, Graph *graph);
int read_reg(const char *fileName, FILE *fd, Graph *graph);

FILE *open_file(const char *filename, enum GRAPH_FILE_TYPE type)
{
	FILE *fd;
	switch (type)
	{
	case GFT_VFU:
		fd = fopen(filename, "r+b");
		break;
	default:
		fd = fopen(filename, "r");
		break;
	}
	if (fd == NULL)
	{
		printf("ERROR: Cannot open input file %s\n", filename);
		exit(1);
	}
	return fd;
};

int read_dbgraph(const char *filename, FILE *fd, Graph *g, enum GRAPH_FILE_TYPE type)
{
	int ret = 0;
	switch (type)
	{
	case GFT_GFU:
		ret = read_gfu(filename, fd, g);
		break;
	case GFT_GFD:
		ret = read_gfd(filename, fd, g);
		break;
	case GFT_EGFU:
		ret = read_egfu(filename, fd, g);
		break;
	case GFT_EGFD:
		ret = read_egfd(filename, fd, g);
		break;
	case GFT_VFU:
		ret = read_vfu(filename, fd, g);
		break;
	case GFT_LAD:
		ret = read_lad(filename, fd, g);
		break;
	case GFT_REG:
		ret = read_reg(filename, fd, g);
		break;
	}

	return ret;
};

int read_graph(const char *filename, Graph *g, enum GRAPH_FILE_TYPE type)
{
	FILE *fd = open_file(filename, type);
	if (fd == NULL)
	{
		printf("ERROR: Cannot open input file %s\n", filename);
		exit(1);
	}
	int ret = 0;
	switch (type)
	{
	case GFT_GFU:
		ret = read_gfu(filename, fd, g);
		break;
	case GFT_GFD:
		ret = read_gfd(filename, fd, g);
		break;
	case GFT_EGFU:
		ret = read_egfu(filename, fd, g);
		break;
	case GFT_EGFD:
		ret = read_egfd(filename, fd, g);
		break;
	case GFT_VFU:
		ret = read_vfu(filename, fd, g);
		break;
	case GFT_LAD:
		ret = read_lad(filename, fd, g);
		break;
	case GFT_REG:
		ret = read_reg(filename, fd, g);
		break;
	}

	fclose(fd);
	return ret;
};

struct gr_neighs_t
{
public:
	int nid;
	gr_neighs_t *next;
};

int read_gfu(const char *fileName, FILE *fd, Graph *graph)
{
	char str[STR_READ_LENGTH];
	int i, j;

	if (fscanf(fd, "%s", str) != 1)
	{ // #graphname
		return -1;
	}
	if (fscanf(fd, "%d", &(graph->nof_nodes)) != 1)
	{ // nof nodes
		return -1;
	}

	// node labels
	graph->nodes_attrs = (void **)malloc(graph->nof_nodes * sizeof(void *));
	char *label = new char[STR_READ_LENGTH];
	for (i = 0; i < graph->nof_nodes; i++)
	{
		if (fscanf(fd, "%s", label) != 1)
		{
			return -1;
		}
		graph->nodes_attrs[i] = new std::string(label);
	}

	// edges
	graph->out_adj_sizes = (int *)calloc(graph->nof_nodes, sizeof(int));
	graph->in_adj_sizes = (int *)calloc(graph->nof_nodes, sizeof(int));

	gr_neighs_t **ns_o = (gr_neighs_t **)malloc(graph->nof_nodes * sizeof(gr_neighs_t));
	gr_neighs_t **ns_i = (gr_neighs_t **)malloc(graph->nof_nodes * sizeof(gr_neighs_t));
	for (i = 0; i < graph->nof_nodes; i++)
	{
		ns_o[i] = NULL;
		ns_i[i] = NULL;
	}
	int temp = 0;
	if (fscanf(fd, "%d", &temp) != 1)
	{ // number of edges
		return -1;
	}

	int es = 0, et = 0;
	for (i = 0; i < temp; i++)
	{
		if (fscanf(fd, "%d", &es) != 1)
		{ // source node
			return -1;
		}
		if (fscanf(fd, "%d", &et) != 1)
		{ // target node
			return -1;
		}

		graph->out_adj_sizes[es]++;
		graph->in_adj_sizes[et]++;

		if (ns_o[es] == NULL)
		{
			ns_o[es] = (gr_neighs_t *)malloc(sizeof(gr_neighs_t));
			ns_o[es]->nid = et;
			ns_o[es]->next = NULL;
		}
		else
		{
			gr_neighs_t *n = (gr_neighs_t *)malloc(sizeof(gr_neighs_t));
			n->nid = et;
			n->next = (struct gr_neighs_t *)ns_o[es];
			ns_o[es] = n;
		}

		graph->out_adj_sizes[et]++;
		graph->in_adj_sizes[es]++;

		if (ns_o[et] == NULL)
		{
			ns_o[et] = (gr_neighs_t *)malloc(sizeof(gr_neighs_t));
			ns_o[et]->nid = es;
			ns_o[et]->next = NULL;
		}
		else
		{
			gr_neighs_t *n = (gr_neighs_t *)malloc(sizeof(gr_neighs_t));
			n->nid = es;
			n->next = (struct gr_neighs_t *)ns_o[et];
			ns_o[et] = n;
		}
	}

	graph->out_adj_list = (int **)malloc(graph->nof_nodes * sizeof(int *));
	graph->in_adj_list = (int **)malloc(graph->nof_nodes * sizeof(int *));
	graph->out_adj_attrs = (void ***)malloc(graph->nof_nodes * sizeof(void **));

	int *ink = (int *)calloc(graph->nof_nodes, sizeof(int));
	for (i = 0; i < graph->nof_nodes; i++)
	{
		graph->in_adj_list[i] = (int *)calloc(graph->in_adj_sizes[i], sizeof(int));
	}
	for (i = 0; i < graph->nof_nodes; i++)
	{
		// reading degree and successors of vertex i
		graph->out_adj_list[i] = (int *)calloc(graph->out_adj_sizes[i], sizeof(int));
		graph->out_adj_attrs[i] = (void **)malloc(graph->out_adj_sizes[i] * sizeof(void *));

		gr_neighs_t *n = ns_o[i];
		for (j = 0; j < graph->out_adj_sizes[i]; j++)
		{
			graph->out_adj_list[i][j] = n->nid;
			graph->out_adj_attrs[i][j] = NULL;

			graph->in_adj_list[n->nid][ink[n->nid]] = i;

			ink[n->nid]++;

			n = n->next;
		}
	}

	//	graph->sort_edges();

	for (int i = 0; i < graph->nof_nodes; i++)
	{
		if (ns_o[i] != NULL)
		{
			gr_neighs_t *p = NULL;
			gr_neighs_t *n = ns_o[i];
			for (j = 0; j < graph->out_adj_sizes[i]; j++)
			{
				if (p != NULL)
					free(p);
				p = n;
				n = n->next;
			}
			if (p != NULL)
				free(p);
		}

		if (ns_i[i] != NULL)
		{
			gr_neighs_t *p = NULL;
			gr_neighs_t *n = ns_i[i];
			for (j = 0; j < graph->out_adj_sizes[i]; j++)
			{
				if (p != NULL)
					free(p);
				p = n;
				n = n->next;
			}
			if (p != NULL)
				free(p);
		}

		//			free(ns_o);
		//			free(ns_i);
	}

	return 0;
};

int read_gfd(const char *fileName, FILE *fd, Graph *graph)
{
	char str[STR_READ_LENGTH];
	int i, j;

	if (fscanf(fd, "%s", str) != 1)
	{ // #graphname
		return -1;
	}
	if (fscanf(fd, "%d", &(graph->nof_nodes)) != 1)
	{ // nof nodes
		return -1;
	}
	// node labels
	graph->nodes_attrs = (void **)malloc(graph->nof_nodes * sizeof(void *));
	char *label = new char[STR_READ_LENGTH];
	for (i = 0; i < graph->nof_nodes; i++)
	{
		if (fscanf(fd, "%s", label) != 1)
		{
			return -1;
		}
		graph->nodes_attrs[i] = new std::string(label);
	}

	// edges
	graph->out_adj_sizes = (int *)calloc(graph->nof_nodes, sizeof(int));
	graph->in_adj_sizes = (int *)calloc(graph->nof_nodes, sizeof(int));

	gr_neighs_t **ns_o = (gr_neighs_t **)malloc(graph->nof_nodes * sizeof(gr_neighs_t));
	gr_neighs_t **ns_i = (gr_neighs_t **)malloc(graph->nof_nodes * sizeof(gr_neighs_t));
	for (i = 0; i < graph->nof_nodes; i++)
	{
		ns_o[i] = NULL;
		ns_i[i] = NULL;
	}
	int temp = 0;
	if (fscanf(fd, "%d", &temp) != 1)
	{ // number of edges
		return -1;
	}
	int es = 0, et = 0;
	for (i = 0; i < temp; i++)
	{
		if (fscanf(fd, "%d", &es) != 1)
		{ // source node
			return -1;
		}
		if (fscanf(fd, "%d", &et) != 1)
		{ // target node
			return -1;
		}
		graph->out_adj_sizes[es]++;
		graph->in_adj_sizes[et]++;

		if (ns_o[es] == NULL)
		{
			ns_o[es] = (gr_neighs_t *)malloc(sizeof(gr_neighs_t));
			ns_o[es]->nid = et;
			ns_o[es]->next = NULL;
		}
		else
		{
			gr_neighs_t *n = (gr_neighs_t *)malloc(sizeof(gr_neighs_t));
			n->nid = et;
			n->next = (struct gr_neighs_t *)ns_o[es];
			ns_o[es] = n;
		}
	}

	graph->out_adj_list = (int **)malloc(graph->nof_nodes * sizeof(int *));
	graph->in_adj_list = (int **)malloc(graph->nof_nodes * sizeof(int *));
	graph->out_adj_attrs = (void ***)malloc(graph->nof_nodes * sizeof(void **));
	int *ink = (int *)calloc(graph->nof_nodes, sizeof(int));
	for (i = 0; i < graph->nof_nodes; i++)
		graph->in_adj_list[i] = (int *)calloc(graph->in_adj_sizes[i], sizeof(int));
	for (i = 0; i < graph->nof_nodes; i++)
	{
		// reading degree and successors of vertex i
		graph->out_adj_list[i] = (int *)calloc(graph->out_adj_sizes[i], sizeof(int));
		graph->out_adj_attrs[i] = (void **)malloc(graph->out_adj_sizes[i] * sizeof(void *));

		gr_neighs_t *n = ns_o[i];
		for (j = 0; j < graph->out_adj_sizes[i]; j++)
		{
			graph->out_adj_list[i][j] = n->nid;
			graph->out_adj_attrs[i][j] = NULL;

			graph->in_adj_list[n->nid][ink[n->nid]] = i;
			ink[n->nid]++;

			n = n->next;
		}
	}

	//	graph->sort_edges();

	return 0;
};

int read_vfu(const char *fileName, FILE *fd, Graph *graph)
{
	return 0;
};

int read_lad(const char *fileName, FILE *fd, Graph *graph)
{
	return 0;
};

struct egr_neighs_t
{
// a linked list to store labels for the edges
public:
	int nid;
	egr_neighs_t *next;
	std::string *label;
};

int read_egfu(const char *fileName, FILE *fd, Graph *graph)
{
	char str[STR_READ_LENGTH];
	int i, j;

	if (fscanf(fd, "%s", str) != 1)
	{ // #graphname
		return -1;
	}
	if (fscanf(fd, "%d", &(graph->nof_nodes)) != 1)
	{ // nof nodes
		return -1;
	}

	// node labels
	graph->nodes_attrs = (void **)malloc(graph->nof_nodes * sizeof(void *));
	char *label = new char[STR_READ_LENGTH];
	for (i = 0; i < graph->nof_nodes; i++)
	{
		if (fscanf(fd, "%s", label) != 1)
		{
			return -1;
		}
		graph->nodes_attrs[i] = new std::string(label);
	}

	// edges
	graph->out_adj_sizes = (int *)calloc(graph->nof_nodes, sizeof(int));
	graph->in_adj_sizes = (int *)calloc(graph->nof_nodes, sizeof(int));

	egr_neighs_t **ns_o = (egr_neighs_t **)malloc(graph->nof_nodes * sizeof(egr_neighs_t));
	egr_neighs_t **ns_i = (egr_neighs_t **)malloc(graph->nof_nodes * sizeof(egr_neighs_t));
	for (i = 0; i < graph->nof_nodes; i++)
	{
		ns_o[i] = NULL;
		ns_i[i] = NULL;
	}
	int temp = 0;
	if (fscanf(fd, "%d", &temp) != 1)
	{ // number of edges
		return -1;
	}

	int es = 0, et = 0;
	for (i = 0; i < temp; i++)
	{
		if (fscanf(fd, "%d", &es) != 1)
		{ // source node
			return -1;
		}
		if (fscanf(fd, "%d", &et) != 1)
		{ // target node
			return -1;
		}
		if (fscanf(fd, "%s", label) != 1)
		{
			return -1;
		}

		graph->out_adj_sizes[es]++;
		graph->in_adj_sizes[et]++;

		if (ns_o[es] == NULL)
		{
			ns_o[es] = (egr_neighs_t *)malloc(sizeof(egr_neighs_t));
			ns_o[es]->nid = et;
			ns_o[es]->next = NULL;
			ns_o[es]->label = new std::string(label);
		}
		else
		{
			egr_neighs_t *n = (egr_neighs_t *)malloc(sizeof(egr_neighs_t));
			n->nid = et;
			n->next = (struct egr_neighs_t *)ns_o[es];
			n->label = new std::string(label);
			ns_o[es] = n;
		}

		graph->out_adj_sizes[et]++;
		graph->in_adj_sizes[es]++;

		if (ns_o[et] == NULL)
		{
			ns_o[et] = (egr_neighs_t *)malloc(sizeof(egr_neighs_t));
			ns_o[et]->nid = es;
			ns_o[et]->next = NULL;
			ns_o[et]->label = new std::string(label);
		}
		else
		{
			egr_neighs_t *n = (egr_neighs_t *)malloc(sizeof(egr_neighs_t));
			n->nid = es;
			n->next = (struct egr_neighs_t *)ns_o[et];
			n->label = new std::string(label);
			ns_o[et] = n;
		}
	}

	graph->out_adj_list = (int **)malloc(graph->nof_nodes * sizeof(int *));
	graph->in_adj_list = (int **)malloc(graph->nof_nodes * sizeof(int *));
	graph->out_adj_attrs = (void ***)malloc(graph->nof_nodes * sizeof(void **));

	int *ink = (int *)calloc(graph->nof_nodes, sizeof(int));
	for (i = 0; i < graph->nof_nodes; i++)
	{
		graph->in_adj_list[i] = (int *)calloc(graph->in_adj_sizes[i], sizeof(int));
	}
	for (i = 0; i < graph->nof_nodes; i++)
	{
		// reading degree and successors of vertex i
		graph->out_adj_list[i] = (int *)calloc(graph->out_adj_sizes[i], sizeof(int));
		graph->out_adj_attrs[i] = (void **)malloc(graph->out_adj_sizes[i] * sizeof(void *));

		egr_neighs_t *n = ns_o[i];
		for (j = 0; j < graph->out_adj_sizes[i]; j++)
		{
			graph->out_adj_list[i][j] = n->nid;
			graph->out_adj_attrs[i][j] = n->label;

			graph->in_adj_list[n->nid][ink[n->nid]] = i;

			ink[n->nid]++;

			n = n->next;
		}
	}

	//	graph->sort_edges();

	for (int i = 0; i < graph->nof_nodes; i++)
	{
		if (ns_o[i] != NULL)
		{
			egr_neighs_t *p = NULL;
			egr_neighs_t *n = ns_o[i];
			for (j = 0; j < graph->out_adj_sizes[i]; j++)
			{
				if (p != NULL)
					free(p);
				p = n;
				n = n->next;
			}
			if (p != NULL)
				free(p);
		}

		if (ns_i[i] != NULL)
		{
			egr_neighs_t *p = NULL;
			egr_neighs_t *n = ns_i[i];
			for (j = 0; j < graph->out_adj_sizes[i]; j++)
			{
				if (p != NULL)
					free(p);
				p = n;
				n = n->next;
			}
			if (p != NULL)
				free(p);
		}

		//			free(ns_o);
		//			free(ns_i);
	}

	return 0;
};

int read_egfd(const char *fileName, FILE *fd, Graph *graph)
{
	char str[STR_READ_LENGTH];
	int i, j;

	if (fscanf(fd, "%s", str) != 1)
	{ // #graphname
		return -1;
	}
	if (fscanf(fd, "%d", &(graph->nof_nodes)) != 1)
	{ // nof nodes
		return -1;
	}
	// node labels
	graph->nodes_attrs = (void **)malloc(graph->nof_nodes * sizeof(void *));
	char *label = new char[STR_READ_LENGTH];
	for (i = 0; i < graph->nof_nodes; i++)
	{
		if (fscanf(fd, "%s", label) != 1)
		{
			return -1;
		}
		graph->nodes_attrs[i] = new std::string(label);
	}

	// edges
	graph->out_adj_sizes = (int *)calloc(graph->nof_nodes, sizeof(int));
	graph->in_adj_sizes = (int *)calloc(graph->nof_nodes, sizeof(int));

	egr_neighs_t **ns_o = (egr_neighs_t **)malloc(graph->nof_nodes * sizeof(egr_neighs_t));
	egr_neighs_t **ns_i = (egr_neighs_t **)malloc(graph->nof_nodes * sizeof(egr_neighs_t));
	for (i = 0; i < graph->nof_nodes; i++)
	{
		ns_o[i] = NULL;
		ns_i[i] = NULL;
	}
	int temp = 0;
	if (fscanf(fd, "%d", &temp) != 1)
	{ // number of edges
		return -1;
	}
	int es = 0, et = 0;
	for (i = 0; i < temp; i++)
	{
		if (fscanf(fd, "%d", &es) != 1)
		{ // source node
			return -1;
		}
		if (fscanf(fd, "%d", &et) != 1)
		{ // target node
			return -1;
		}
		if (fscanf(fd, "%s", label) != 1)
		{
			return -1;
		}

		graph->out_adj_sizes[es]++;
		graph->in_adj_sizes[et]++;

		if (ns_o[es] == NULL)
		{
			ns_o[es] = (egr_neighs_t *)malloc(sizeof(egr_neighs_t));
			ns_o[es]->nid = et;
			ns_o[es]->next = NULL;
			ns_o[es]->label = new std::string(label);
		}
		else
		{
			egr_neighs_t *n = (egr_neighs_t *)malloc(sizeof(egr_neighs_t));
			n->nid = et;
			n->next = (struct egr_neighs_t *)ns_o[es];
			n->label = new std::string(label);
			ns_o[es] = n;
		}
	}

	graph->out_adj_list = (int **)malloc(graph->nof_nodes * sizeof(int *));
	graph->in_adj_list = (int **)malloc(graph->nof_nodes * sizeof(int *));
	graph->out_adj_attrs = (void ***)malloc(graph->nof_nodes * sizeof(void **));

	int *ink = (int *)calloc(graph->nof_nodes, sizeof(int));
	for (i = 0; i < graph->nof_nodes; i++)
	{
		graph->in_adj_list[i] = (int *)calloc(graph->in_adj_sizes[i], sizeof(int));
	}
	for (i = 0; i < graph->nof_nodes; i++)
	{
		// reading degree and successors of vertex i
		graph->out_adj_list[i] = (int *)calloc(graph->out_adj_sizes[i], sizeof(int));
		graph->out_adj_attrs[i] = (void **)malloc(graph->out_adj_sizes[i] * sizeof(void *));

		egr_neighs_t *n = ns_o[i];
		for (j = 0; j < graph->out_adj_sizes[i]; j++)
		{
			graph->out_adj_list[i][j] = n->nid;
			graph->out_adj_attrs[i][j] = n->label;
			graph->in_adj_list[n->nid][ink[n->nid]] = i;
			ink[n->nid]++;

			n = n->next;
		}
	}

	//	graph->sort_edges();

	return 0;
};

struct reg_bw_ltc_t
{
// a linked list to store bandwidths and latencies for the edges
public:
	int nid;
	int bw;
	int ltc;
	reg_bw_ltc_t *next;
};

int read_reg(const char *fileName, FILE *fd, Graph *graph)
{
	char str[STR_READ_LENGTH]; // used to store graph name
	int i, j; // iterate variables

	if (fscanf(fd, "%s", str) != 1)
	{ // #graphname
		return -1;
	}
	if (fscanf(fd, "%d", &(graph->nof_nodes)) != 1)
	{ // nof nodes
		return -1;
	}
	// node labels
	graph->nodes_attrs = (void **)malloc(graph->nof_nodes * sizeof(void *));
	char *label = new char[STR_READ_LENGTH];
	for (i = 0; i < graph->nof_nodes; i++)
	{
		if (fscanf(fd, "%s", label) != 1)
		{
			return -1;
		}
		graph->nodes_attrs[i] = new std::string(label);
	}


	// edges
	graph->out_adj_sizes = (int *)calloc(graph->nof_nodes, sizeof(int));
	graph->in_adj_sizes = (int *)calloc(graph->nof_nodes, sizeof(int));

	reg_bw_ltc_t **ns_o = (reg_bw_ltc_t **)malloc(graph->nof_nodes * sizeof(reg_bw_ltc_t));
	reg_bw_ltc_t **ns_i = (reg_bw_ltc_t **)malloc(graph->nof_nodes * sizeof(reg_bw_ltc_t));
	for (i = 0; i < graph->nof_nodes; i++)
	{
		// initialize
		ns_o[i] = NULL;
		ns_i[i] = NULL;
	}
	int temp = 0;
	if (fscanf(fd, "%d", &temp) != 1)
	{ // number of edges
		return -1;
	}

	int es = 0, et = 0;
	int bw, ltc; // bandwidth and latency, both integers
	for (i = 0; i < temp; i++)
	{
		if (fscanf(fd, "%d", &es) != 1)
		{ // source node
			return -1;
		}
		if (fscanf(fd, "%d", &et) != 1)
		{ // target node
			return -1;
		}
		if (fscanf(fd, "%d", &bw) != 1)
		{
			return -1;
		}
		if (fscanf(fd, "%d", &ltc) != 1)
		{
			return -1;
		}

		graph->out_adj_sizes[es] ++;
		graph->in_adj_sizes[es]++;
		graph->out_adj_sizes[et]++;
		graph->in_adj_sizes[et] ++;
		

		// store in linked list
		// since undirected, only store in ns_o
		// leave ns_i empty
		if (ns_o[es] == NULL)
		{
			// insert the head of the linked list
			ns_o[es] = (reg_bw_ltc_t *)malloc(sizeof(reg_bw_ltc_t));
			ns_o[es]->nid = et;
			ns_o[es]->bw = bw;
			ns_o[es]->ltc = ltc;
			ns_o[es]->next = NULL;
		}
		else
		{
			// append to the head of linked list
			reg_bw_ltc_t *n = (reg_bw_ltc_t *)malloc(sizeof(reg_bw_ltc_t));
			n->nid = et;
			n->next = (struct reg_bw_ltc_t *)ns_o[es];
			n->bw = bw;
			n->ltc = ltc;
			ns_o[es] = n;
		}

		if (ns_o[et] == NULL)
		{
			ns_o[et] = (reg_bw_ltc_t *)malloc(sizeof(reg_bw_ltc_t));
			ns_o[et]->nid = es;
			ns_o[et]->next = NULL;
			ns_o[et]->bw = bw;
			ns_o[et]->ltc = ltc;
		}
		else
		{
			reg_bw_ltc_t *n = (reg_bw_ltc_t *)malloc(sizeof(reg_bw_ltc_t));
			n->nid = es;
			n->next = (struct reg_bw_ltc_t *)ns_o[et];
			n->bw = bw;
			n->ltc = ltc;
			ns_o[et] = n;
		}
	}

	// store the content of linked list to following attributes
	graph->out_adj_list = (int **)malloc(graph->nof_nodes * sizeof(int *));
	graph->in_adj_list = (int **)malloc(graph->nof_nodes * sizeof(int *));
	graph->out_adj_attrs = (void ***)malloc(graph->nof_nodes * sizeof(void **));

	int *ink = (int *)calloc(graph->nof_nodes, sizeof(int));
	for (i = 0; i < graph->nof_nodes; i++)
	{
		graph->in_adj_list[i] = (int *)calloc(graph->in_adj_sizes[i], sizeof(int));
	}
	for (i = 0; i < graph->nof_nodes; i++)
	{
		// reading degree and successors of vertex i
		graph->out_adj_list[i] = (int *)calloc(graph->out_adj_sizes[i], sizeof(int));
		graph->out_adj_attrs[i] = (void **)malloc(graph->out_adj_sizes[i] * sizeof(void *));

		reg_bw_ltc_t *n = ns_o[i];
		for (j = 0; j < graph->out_adj_sizes[i]; j++)
		{
			graph->out_adj_list[i][j] = n->nid;
			
			// create a 2-element int array to store bw and ltc
			// TODO figure out the void ** 
			int* attr = (int *)malloc(2 * sizeof(int));
			attr[0] = n->bw;
			attr[1] = n->ltc;
			graph->out_adj_attrs[i][j] = (void *)attr;

			graph->in_adj_list[n->nid][ink[n->nid]] = i;

			ink[n->nid]++;

			n = n->next;
		}
	}

	// TODO: figure out what sort_edges do
	//	graph->sort_edges();

	// free the linked lists ns_o and ns_i
	for (int i = 0; i < graph->nof_nodes; i++)
	{
		if (ns_o[i] != NULL)
		{
			reg_bw_ltc_t *p = NULL;
			reg_bw_ltc_t *n = ns_o[i];
			for (j = 0; j < graph->out_adj_sizes[i]; j++)
			{
				if (p != NULL)
					free(p);
				p = n;
				n = n->next;
			}
			if (p != NULL)
				free(p);
		}

		if (ns_i[i] != NULL)
		{
			reg_bw_ltc_t *p = NULL;
			reg_bw_ltc_t *n = ns_i[i];
			for (j = 0; j < graph->out_adj_sizes[i]; j++)
			{
				if (p != NULL)
					free(p);
				p = n;
				n = n->next;
			}
			if (p != NULL)
				free(p);
		}

		//			free(ns_o);
		//			free(ns_i);
	}

	return 0;
}

#endif /* C_TEXTDB_DRIVER_H_ */
