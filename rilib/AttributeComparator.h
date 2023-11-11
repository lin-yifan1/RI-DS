/*
 * AttributeComparator.h
 *
 *  Created on: Aug 3, 2012
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

#ifndef ATTRIBUTECOMPARATOR_H_
#define ATTRIBUTECOMPARATOR_H_

#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <fstream>
#include <sstream>

namespace rilib{




class AttributeComparator{
public:
	std::vector<std::vector<bool>> filter;
	int nof_query;
	int nof_ref;
	virtual ~AttributeComparator(){};
	virtual bool compare(void* attr1, void* attr2)=0;
	virtual int compareint(void* attr1, void* attr2)=0;
};





class DefaultAttrComparator: public AttributeComparator{
public:
	DefaultAttrComparator(){};
	virtual bool compare(void* attr1, void* attr2){
		return true;
	};
	virtual int compareint(void* attr1, void* attr2){
		return 0;
	};
};


class StringAttrComparator: public AttributeComparator{
public:
	StringAttrComparator(){};
	virtual bool compare(void* attr1, void* attr2){
		std::string* a=(std::string*)attr1;
		std::string* b=(std::string*)attr2;
		return (a->compare(*b))==0;
	};
	virtual int compareint(void* attr1, void* attr2){
		std::string* a=(std::string*)attr1;
		std::string* b=(std::string*)attr2;
		return a->compare(*b);
	};
};

class IntAttrComparator: public AttributeComparator{
public:
	IntAttrComparator(){};
	virtual bool compare(void* attr1, void* attr2){
		int* a=(int*)attr1;
		int* b=(int*)attr2;
		return ((*a)-(*b))==0;
	};
	virtual int compareint(void* attr1, void* attr2){
		int* a=(int*)attr1;
		int* b=(int*)attr2;
		return (*a)-(*b);
	};
};

class RegNodeAttrComparator: public AttributeComparator{
public:
	RegNodeAttrComparator(char *filtername){
		std::ifstream filterfile(filtername);

		filterfile >> nof_query; // number of query nodes
		filterfile >> nof_ref; // number of nodes in reference graph

		filter.resize(nof_query, std::vector<bool>(nof_ref, false));

		std::string line;
		std::getline(filterfile, line); // move to the next line

		for (int query = 0; query < nof_query; query++)
		{
    		std::getline(filterfile, line);
			std::istringstream iss(line);
			int ref;
			while (iss >> ref) {
				filter[query][ref] = true;
			}
		}
		
	};
	virtual bool compare(void* attr1, void* attr2){
		// attr1 must be pattern/query, attr2 must be target/reference
		int* a = (int*)attr1;
		int* b = (int*)attr2;
		int query = *a;
		int reference = *b;
		return filter[query][reference];
	};
	virtual int compareint(void* attr1, void* attr2){
		return 0;
	}
};

class RegEdgeAttrComparator: public AttributeComparator{
public:
	RegEdgeAttrComparator(){};
	virtual bool compare(void* attr1, void* attr2){
		// attr1 must be query, attr2 must be reference
		int* a = (int*)attr1;
		int* b = (int*)attr2;
		int bw_q = a[0];
		int ltc_q = a[1];
		int bw_r = b[0];
		int ltc_r = b[1];
		return (bw_q <= bw_r) && (ltc_q >= ltc_r);
	};
	virtual int compareint(void* attr1, void* attr2){
		return 0;
	}
};

}


#endif /* ATTRIBUTECOMPARATOR_H_ */
