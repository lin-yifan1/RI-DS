/*
 * Domains.h
 *
 *  Created on: Oct 31, 2012
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

#ifndef DOMAINS_H_
#define DOMAINS_H_


#include "sbitset.h"
#include "Graph.h"
#include "AttributeComparator.h"

namespace rilib{

bool init_domains(
		Graph&			target,
		Graph& 			pattern,
		AttributeComparator& 	nodeComparator,
		AttributeComparator& 	edgeComparator,
		sbitset *domains,
		bool iso
	){



	if(iso){
		for(int q=0; q<pattern.nof_nodes; q++){
			for(int r=target.nof_nodes-1; r>=0; r--){
				if(		target.out_adj_sizes[r] == pattern.out_adj_sizes[q]
					&&	target.in_adj_sizes[r] == pattern.in_adj_sizes[q]
					&& 	nodeComparator.compare(pattern.nodes_attrs[q], target.nodes_attrs[r])){
					domains[q].set(r, true);
				}
			}
			if(domains[q].is_empty()){
				return false;
			}
		}
	}
	else{
		for(int q=0; q<pattern.nof_nodes; q++){
			for(int r=target.nof_nodes-1; r>=0; r--){
				if(		target.out_adj_sizes[r] >= pattern.out_adj_sizes[q]
					&&	target.in_adj_sizes[r] >= pattern.in_adj_sizes[q]
					&& 	nodeComparator.compare(pattern.nodes_attrs[q], target.nodes_attrs[r])){
					domains[q].set(r, true);
				}
			}
			if(domains[q].is_empty()){
				return false;
			}
		}
	}






	int ra, qb, rb;
	bool notfound;

	//1°-level neighborhood and edges labels
	for(int qa=0; qa<pattern.nof_nodes; qa++){

		for(sbitset::iterator qaIT=domains[qa].first_ones(); qaIT!=domains[qa].end(); qaIT.next_ones()){
			ra = qaIT.first;
			//for each edge qa->qb  check if exists ra->rb
			// 对任意的 query node qa，
			// 如果将其 match 到 ra，
			// 那么对于 qa 的每一个邻居 qb，
			// 必须保证其可以被 match 到 ra 的某个邻居 rb，
			// 否则 qa 不可以被 match 到 ra
			for(int i_qb=0; i_qb<pattern.out_adj_sizes[qa]; i_qb++){
				qb = pattern.out_adj_list[qa][i_qb];
				notfound = true;

				for(int i_rb=0; i_rb<target.out_adj_sizes[ra]; i_rb++){
					rb = target.out_adj_list[ra][i_rb];
					if(domains[qb].get(rb) && edgeComparator.compare(pattern.out_adj_attrs[qa][i_qb], target.out_adj_attrs[ra][i_rb])){
						notfound = false;
						break;
					}
				}

				if(notfound){
					domains[qa].set(ra, false);
					break;
				}
			}
		}

		if(domains[qa].is_empty())
			return false;
	}

	// 我不太明白下面这一段的意义，
	// 看上去只是上一段的另一种实现。
	bool changes = true;
	while(changes){
		changes = false;
		for(int qa=0; qa<pattern.nof_nodes; qa++){
			for(sbitset::iterator qaIT=domains[qa].first_ones(); qaIT!=domains[qa].end(); qaIT.next_ones()){
				ra = qaIT.first;
				//fore each edge qa->qb  check if exists ra->rb
				for(int i_qb=0; i_qb<pattern.out_adj_sizes[qa]; i_qb++){
					qb = pattern.out_adj_list[qa][i_qb];
					notfound = true;
					for(int i_rb=0; i_rb<target.out_adj_sizes[ra]; i_rb++){
						rb = target.out_adj_list[ra][i_rb];
						if(domains[qb].get(rb) && edgeComparator.compare(pattern.out_adj_attrs[qa][i_qb], target.out_adj_attrs[ra][i_rb])){
							notfound = false;
							break;
						}
					}

					if(notfound){
						domains[qa].set(ra, false);
						changes = true;
					}
				}
			}
			if(domains[qa].is_empty())
				return false;
		}
	}


	return true;

};

}


#endif /* DOMAINS_H_ */
