/*
 * test_parser.cc
 *
 *  Created on: Jan 11, 2020
 *      Author: jb
 * 	
 * 	Further developed by: Linsator & KKSSainz
 * 	
 */

#include <string>
#include <chrono>
#include <iostream>
#include <fstream>
#include "assert.hh"
#include "parser.hh"
#include "test_tools.hh"

#include "arena_alloc.hh"

struct ExprData {
	ExprData(bparser::ArenaAlloc &arena, uint vec_size)
	: vec_size(vec_size)
	{
		v1 = arena.create_array<double>(vec_size * 3);
		fill_seq(v1, 100, 100 + 3 * vec_size);
		v2 = arena.create_array<double>(vec_size * 3);
		fill_seq(v2, 200, 200 + 3 * vec_size);
		
		v3 = arena.create_array<double>(vec_size * 3);
		fill_seq(v3, 100, 100 + 3 * vec_size * 0.5, 0.5);
		v4 = arena.create_array<double>(vec_size * 3);
		fill_seq(v4, 200, 200 + 3 * vec_size * 0.5, 0.5);
		
		vres = arena.create_array<double>(vec_size * 3);
		fill_const(vres, 3 * vec_size, -100);
		subset = arena.create_array<uint>(vec_size);
		for(uint i=0; i<vec_size/4; i++) subset[i] = i;
		cs1 = 4;
		for (int i = 0; i < 3; i++)
		{
			cv1[i] = (i+1)*3;
		}
		
	}

	uint vec_size;
	double *v1, *v2, *v3, *v4, *vres;
	double cs1;
	double cv1[3];
	uint *subset;

};


void test_expr_cpp(std::string expr, void (*f)(ExprData&), std::string expr_id, std::ofstream& file) {
	using namespace bparser;
	uint block_size = 1024; // number of floats
	uint vec_size = 1*block_size;

	// TODO: allow changing variable pointers, between evaluations
	// e.g. p.set_variable could return pointer to that pointer
	// not so easy for vector and tensor variables, there are many pointers to set
	// Rather modify the test to fill the
	uint n_repeats = 1000;

	ArenaAlloc arena_2(32, 16*vec_size *sizeof(double));
	ExprData data2(arena_2, vec_size);

	auto start_time = std::chrono::high_resolution_clock::now();
	for(uint i_rep=0; i_rep < n_repeats; i_rep++) {
		f(data2);
	}
	auto end_time = std::chrono::high_resolution_clock::now();
	double cpp_time  =
			std::chrono::duration_cast<std::chrono::duration<double>>(end_time - start_time).count();


	// check
	double c_sum = 0;
	for(uint dim=0; dim < 3; dim++) {
		for(uint i=0; i<data2.vec_size; i++) {
			double v2 = data2.vres[dim*data2.vec_size + i];
			c_sum += v2;
		}
	}

	std::cout << "Expression " << expr_id << ": " << expr << "\n";
	std::cout << "Result      : " << c_sum << "\n";
	std::cout << "Repeats     : " << n_repeats << "\n";
	std::cout << "c++ time    : " << cpp_time << " s \n";
	std::cout << "c++ avg. time per single execution (with vector size in mind): " << cpp_time/n_repeats/vec_size*1e9 << " ns \n";
	double n_flop = n_repeats * vec_size * 9;
	std::cout << "c++ FLOPS   : " << n_flop / cpp_time << "\n";
	std::cout << "\n";

	file << "C++,"<< expr_id << ","<< expr << "," << c_sum << "," << n_repeats << "," << cpp_time << "," << cpp_time/n_repeats/vec_size*1e9 << "," << n_flop/cpp_time << "\n";
}





///	EXPRESSIONS ///

void expr_test0A(ExprData &data) {
	for(uint i_comp=0; i_comp < 3*data.vec_size; i_comp += data.vec_size) {
		for(uint i=0; i<data.vec_size/4; ++i) {
			uint j = i_comp + 4*data.subset[i];
			for(uint k = 0; k<4; k++) {
				double v1 = data.v1[j+k];
				data.vres[j+k] =  v1 + 1.1 ;
			}
		}
	}
}

void expr_test0B(ExprData &data) {
	for(uint i_comp=0; i_comp < 3*data.vec_size; i_comp += data.vec_size) {
		for(uint i=0; i<data.vec_size/4; ++i) {
			uint j = i_comp + 4*data.subset[i];
			for(uint k = 0; k<4; k++) {
				double v1 = data.v1[j+k];
				double v2 = data.v2[j+k];
				double v3 = data.v3[j+k];
				double v4 = data.v4[j+k];
				data.vres[j+k] = v1 + v2 + v3 + v4;
			}
		}
	}
}

void expr_test1A(ExprData &data) {
	for(uint i_comp=0; i_comp < 3*data.vec_size; i_comp += data.vec_size) {
		for(uint i=0; i<data.vec_size/4; ++i) {
			uint j = i_comp + 4*data.subset[i];
			for(uint k = 0; k<4; k++) {
				double v1 = data.v1[j+k];
				data.vres[j+k] = v1 * v1;
			}
		}
	}
}

void expr_test1B(ExprData &data) {
	for(uint i_comp=0; i_comp < 3*data.vec_size; i_comp += data.vec_size) {
		for(uint i=0; i<data.vec_size/4; ++i) {
			uint j = i_comp + 4*data.subset[i];
			for(uint k = 0; k<4; k++) {
				double v1 = data.v1[j+k];
				data.vres[j+k] = pow(v1, 2);
			}
		}
	}
}

void expr_test2A(ExprData &data) {
	for(uint i_comp=0; i_comp < 3*data.vec_size; i_comp += data.vec_size) {
		for(uint i=0; i<data.vec_size/4; ++i) {
			uint j = i_comp + 4*data.subset[i];
			for(uint k = 0; k<4; k++) {
				double v1 = data.v1[j+k];
				data.vres[j+k] = v1 / 3;
			}
		}
	}
}

void expr_test2B(ExprData &data) {
	for(uint i_comp=0; i_comp < 3*data.vec_size; i_comp += data.vec_size) {
		for(uint i=0; i<data.vec_size/4; ++i) {
			uint j = i_comp + 4*data.subset[i];
			for(uint k = 0; k<4; k++) {
				double v1 = data.v1[j+k];
				data.vres[j+k] = v1 * 3;
			}
		}
	}
}

void expr_test3A(ExprData &data) {
	for(uint i_comp=0; i_comp < 3*data.vec_size; i_comp += data.vec_size) {
		for(uint i=0; i<data.vec_size/4; ++i) {
			uint j = i_comp + 4*data.subset[i];
			for(uint k = 0; k<4; k++) {
				double v1 = data.v1[j+k];
				double v2 = data.v2[j+k];
				data.vres[j+k] = abs(sin(sqrt(pow(v1, 2) + pow(v2, 2))));
			}
		}
	}
}

void expr_test3B(ExprData &data) {
	for(uint i_comp=0; i_comp < 3*data.vec_size; i_comp += data.vec_size) {
		for(uint i=0; i<data.vec_size/4; ++i) {
			uint j = i_comp + 4*data.subset[i];
			for(uint k = 0; k<4; k++) {
				double v1 = data.v1[j+k];
				data.vres[j+k] = abs(sin(sqrt(v1)));
			}
		}
	}
}

void expr_test3C(ExprData &data) {
	for(uint i_comp=0; i_comp < 3*data.vec_size; i_comp += data.vec_size) {
		for(uint i=0; i<data.vec_size/4; ++i) {
			uint j = i_comp + 4*data.subset[i];
			for(uint k = 0; k<4; k++) {
				double v1 = data.v1[j+k];
				double v2 = data.v2[j+k];
				data.vres[j+k] = pow(v1,2) + pow(v2,2);
			}
		}
	}
}

void expr_test4A(ExprData &data) {
	for(uint i_comp=0; i_comp < 3*data.vec_size; i_comp += data.vec_size) {
		for(uint i=0; i<data.vec_size/4; ++i) {
			uint j = i_comp + 4*data.subset[i];
			for(uint k = 0; k<4; k++) {
				double v1 = data.v1[j+k];
				data.vres[j+k] = pow(v1,3);
			}
		}
	}
}

void expr_test4B(ExprData &data) {
	for(uint i_comp=0; i_comp < 3*data.vec_size; i_comp += data.vec_size) {
		for(uint i=0; i<data.vec_size/4; ++i) {
			uint j = i_comp + 4*data.subset[i];
			for(uint k = 0; k<4; k++) {
				double v1 = data.v1[j+k];
				data.vres[j+k] = pow(v1,3.01);
			}
		}
	}
}

void expr_test5A(ExprData &data) {
	for(uint i_comp=0; i_comp < 3*data.vec_size; i_comp += data.vec_size) {
		for(uint i=0; i<data.vec_size/4; ++i) {
			uint j = i_comp + 4*data.subset[i];
			for(uint k = 0; k<4; k++) {
				double v1 = data.v1[j+k];
				double v2 = data.v2[j+k];
				double v3 = data.v3[j+k];
				data.vres[j+k] = pow(v1,2) + pow(v2,2) + pow(v3,2);
			}
		}
	}
}

void expr_test5B(ExprData &data) {
	for(uint i_comp=0; i_comp < 3*data.vec_size; i_comp += data.vec_size) {
		for(uint i=0; i<data.vec_size/4; ++i) {
			uint j = i_comp + 4*data.subset[i];
			for(uint k = 0; k<4; k++) {
				double v1 = data.v1[j+k];
				data.vres[j+k] = sqrt(v1);
			}
		}
	}
}

void expr_test5C(ExprData &data) {
	for(uint i_comp=0; i_comp < 3*data.vec_size; i_comp += data.vec_size) {
		for(uint i=0; i<data.vec_size/4; ++i) {
			uint j = i_comp + 4*data.subset[i];
			for(uint k = 0; k<4; k++) {
				double v1 = data.v1[j+k];
				double v2 = data.v2[j+k];
				double v3 = data.v3[j+k];
				data.vres[j+k] = sqrt(pow(v1,2) + pow(v2,2) + pow(v3,2));
			}
		}
	}
}

void expr_test7A(ExprData &data) {
	for(uint i_comp=0; i_comp < 3*data.vec_size; i_comp += data.vec_size) {
		for(uint i=0; i<data.vec_size/4; ++i) {
			uint j = i_comp + 4*data.subset[i];
			for(uint k = 0; k<4; k++) {
				double v1 = data.v1[j+k];
				double v2 = data.v2[j+k];
				data.vres[j+k] = v1 * v2;
			}
		}
	}
}

void expr_test7B(ExprData &data) {
	for(uint i_comp=0; i_comp < 3*data.vec_size; i_comp += data.vec_size) {
		for(uint i=0; i<data.vec_size/4; ++i) {
			uint j = i_comp + 4*data.subset[i];
			for(uint k = 0; k<4; k++) {
				double v1 = data.v1[j+k];
				double v2 = data.v2[j+k];
				double v3 = data.v3[j+k];
				double v4 = data.v4[j+k];
				data.vres[j+k] = v1 * v2 * v3 * v4;
			}
		}
	}
}

void expr_test7C(ExprData &data) {
	for(uint i_comp=0; i_comp < 3*data.vec_size; i_comp += data.vec_size) {
		for(uint i=0; i<data.vec_size/4; ++i) {
			uint j = i_comp + 4*data.subset[i];
			for(uint k = 0; k<4; k++) {
				double v1 = data.v1[j+k];
				data.vres[j+k] = 3 * v1;
			}
		}
	}
}

void expr_test7D(ExprData &data) {
	for(uint i_comp=0; i_comp < 3*data.vec_size; i_comp += data.vec_size) {
		for(uint i=0; i<data.vec_size/4; ++i) {
			uint j = i_comp + 4*data.subset[i];
			for(uint k = 0; k<4; k++) {
				double v1 = data.v1[j+k];
				data.vres[j+k] = data.cs1 * v1;
			}
		}
	}
}

void expr_test7E(ExprData &data) {
	for(uint i_comp=0; i_comp < 3*data.vec_size; i_comp += data.vec_size) {
		for(uint i=0; i<data.vec_size/4; ++i) {
			uint j = i_comp + 4*data.subset[i];
			for(uint k = 0; k<4; k++) {
				double v1 = data.v1[j+k];
				double v2 = data.v2[j+k];
				data.vres[j+k] = 3 * v1 + v1 * v2;
			}
		}
	}
}

void expr_test7F(ExprData &data) {
	for(uint i_comp=0; i_comp < 3*data.vec_size; i_comp += data.vec_size) {
		for(uint i=0; i<data.vec_size/4; ++i) {
			uint j = i_comp + 4*data.subset[i];
			for(uint k = 0; k<4; k++) {
				double v1 = data.v1[j+k];
				double v2 = data.v2[j+k];
				data.vres[j+k] = 3 * v1 * v1 * v2;
			}
		}
	}
}

void expr_test7G(ExprData &data) {
	for(uint i_comp=0; i_comp < 3*data.vec_size; i_comp += data.vec_size) {
		for(uint i=0; i<data.vec_size/4; ++i) {
			uint j = i_comp + 4*data.subset[i];
			for(uint k = 0; k<4; k++) {
				double v1 = data.v1[j+k];
				double v2 = data.v2[j+k];
				data.vres[j+k] = data.cs1 * v1 + v1 * v2;
			}
		}
	}
}

void expr_test7H(ExprData &data) {
	for(uint i_comp=0; i_comp < 3*data.vec_size; i_comp += data.vec_size) {
		for(uint i=0; i<data.vec_size/4; ++i) {
			uint j = i_comp + 4*data.subset[i];
			for(uint k = 0; k<4; k++) {
				double v3 = data.v3[j+k];
				double cv1 = data.cv1[i_comp/data.vec_size];
				data.vres[j+k] = cv1 * v3;
			}
		}
	}
}

void expr_test7I(ExprData &data) {
	for(uint i_comp=0; i_comp < 3*data.vec_size; i_comp += data.vec_size) {
		for(uint i=0; i<data.vec_size/4; ++i) {
			uint j = i_comp + 4*data.subset[i];
			for(uint k = 0; k<4; k++) {
				double v4 = data.v4[j+k];
				double cv1 = data.cv1[i_comp/data.vec_size];
				data.vres[j+k] = cv1 + v4;
			}
		}
	}
}

void expr_test7J(ExprData &data) {
	for(uint i_comp=0; i_comp < 3*data.vec_size; i_comp += data.vec_size) {
		for(uint i=0; i<data.vec_size/4; ++i) {
			uint j = i_comp + 4*data.subset[i];
			for(uint k = 0; k<4; k++) {
				double v3 = data.v3[j+k];
				double v4 = data.v4[j+k];
				double cv1 = data.cv1[i_comp/data.vec_size];
				data.vres[j+k] = cv1 * v3 + cv1 + v4;
			}
		}
	}
}

void expr_test7K(ExprData &data) {
	for(uint i_comp=0; i_comp < 3*data.vec_size; i_comp += data.vec_size) {
		for(uint i=0; i<data.vec_size/4; ++i) {
			uint j = i_comp + 4*data.subset[i];
			for(uint k = 0; k<4; k++) {
				double v3 = data.v3[j+k];
				double v4 = data.v4[j+k];
				double cv1 = data.cv1[i_comp/data.vec_size];
				data.vres[j+k] = cv1 * v3 * cv1 + v4;
			}
		}
	}
}

void expr_test7L(ExprData &data) {
	for(uint i_comp=0; i_comp < 3*data.vec_size; i_comp += data.vec_size) {
		for(uint i=0; i<data.vec_size/4; ++i) {
			uint j = i_comp + 4*data.subset[i];
			for(uint k = 0; k<4; k++) {
				double v3 = data.v3[j+k];
				double v4 = data.v4[j+k];
				double cv1 = data.cv1[i_comp/data.vec_size];
				data.vres[j+k] = (cv1 * v3 * cv1 + v4) + (cv1 * v3 + cv1 + v4);
			}
		}
	}
}

void expr_test7M(ExprData &data) {
	for(uint i_comp=0; i_comp < 3*data.vec_size; i_comp += data.vec_size) {
		for(uint i=0; i<data.vec_size/4; ++i) {
			uint j = i_comp + 4*data.subset[i];
			for(uint k = 0; k<4; k++) {
				double v3 = data.v3[j+k];
				double v4 = data.v4[j+k];
				double cv1 = data.cv1[i_comp/data.vec_size];
				data.vres[j+k] = (cv1 * v3 * cv1 + v4) * (cv1 * v3 + cv1 + v4);
			}
		}
	}
}

/*
void expr_test(ExprData &data) {
	for(uint i_comp=0; i_comp < 3*data.vec_size; i_comp += data.vec_size) {
		for(uint i=0; i<data.vec_size/4; ++i) {
			uint j = i_comp + 4*data.subset[i];
			for(uint k = 0; k<4; k++) {
				double v1 = data.v1[j+k];
				double v2 = data.v2[j+k];
				double v3 = data.v3[j+k];
				double v4 = data.v4[j+k];
				double cv1 = data.cv1[i_comp/data.vec_size];
				data.vres[j+k] = ;
			}
		}
	}
}
*/



void test_expressions(std::string filename) {

	std::ofstream file;
	if(!filename.empty())
	{
		file.open(filename);
		std::cout << "Outputing to " << filename << "\n";
	}
	else
	{
		file.open("vystup.csv");
	}

	//header
	file << "Executor,ID,Expression,Result,Repeats,Time,Avg. time per single execution,FLOPS\n";

	std::cout << "Starting tests with C++.\n";

    test_expr_cpp("v1 + 1.1", expr_test0A, "test0A", file);
	test_expr_cpp("v1 + v2 + v3 + v4", expr_test0B, "test0B", file);

	test_expr_cpp("v1 * v1", expr_test1A, "test1A", file);
    test_expr_cpp("v1**2", expr_test1B, "test1B", file);

    test_expr_cpp("v1 / 3", expr_test2A, "test2A", file);
    test_expr_cpp("v1 * 3", expr_test2B, "test2B", file);

    test_expr_cpp("abs(sin(sqrt(v1**2 + v2**2)))", expr_test3A, "test3A", file);
    test_expr_cpp("abs(sin(sqrt(v1)))", expr_test3B, "test3B", file);
    test_expr_cpp("v1**2 + v2**2", expr_test3C, "test3C", file);

	test_expr_cpp("v1**3", expr_test4A, "test4A", file);
	test_expr_cpp("v1**3.01", expr_test4B,"test4B", file);

	test_expr_cpp("v1**2 + v2**2 + v3**2", expr_test5A, "test5A", file);
	test_expr_cpp("sqrt(v1)", expr_test5B, "test5B", file);
	test_expr_cpp("sqrt(v1**2 + v2**2 + v3**2)", expr_test5C, "test5C", file);

	//matrix

	test_expr_cpp("v1 * v2", expr_test7A, "test7A", file);
	test_expr_cpp("v1 * v2 * v3 * v4", expr_test7B, "test7B", file);

	test_expr_cpp("3 * v1", expr_test7C, "test7C", file);
	test_expr_cpp("cs1 * v1", expr_test7D, "test7D", file);

	test_expr_cpp("3 * v1 + v1 * v2", expr_test7E, "test7E", file);
	test_expr_cpp("3 * v1 * v1 * v2", expr_test7F, "test7F", file);

	test_expr_cpp("cs1 * v1 + v1 * v2", expr_test7G, "test7G", file);

	test_expr_cpp("cv1 * v3", expr_test7H, "test7H", file);
	test_expr_cpp("cv1 + v4", expr_test7I, "test7I", file);
	
	test_expr_cpp("cv1 * v3 + cv1 + v4", expr_test7J, "test7J", file);
	test_expr_cpp("cv1 * v3 * cv1 + v4", expr_test7K, "test7K", file);

	test_expr_cpp("(cv1 * v3 * cv1 + v4) + (cv1 * v3 + cv1 + v4)", expr_test7L, "test7L", file);
	test_expr_cpp("(cv1 * v3 * cv1 + v4) * (cv1 * v3 + cv1 + v4)", expr_test7M, "test7M", file);
}



int main(int argc, char * argv[])
{
	std::string soubor = "";
	if(argc > 1)
	{
		soubor = argv[1]; 
	}
	test_expressions(soubor);
}