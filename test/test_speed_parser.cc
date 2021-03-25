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

		for (uint i = 0; i < 3; i++)
		{
			cv1[i] = (i+1)*3;
		}

		a = arena.create_array<double>(vec_size * 9);
		//fill_seq(a, 1, 1 + 9 * vec_size);
		for(uint i = 0; i < 9; i++)
		{
			for (uint j = 0; j < vec_size; j++)
			{
				a[i*vec_size+j] = j*9+i+1;
			}
		}
		
	}

	uint vec_size;
	double *v1, *v2, *v3, *v4, *vres;
	double *a;
	double cs1;
	double cv1[3];
	uint *subset;
	//double a[9];
};


void test_expr_parser(std::string expr, std::string expr_id, std::ofstream& file) {
	using namespace bparser;
	uint block_size = 1024; // number of floats
	uint vec_size = 1*block_size;

	// TODO: allow changing variable pointers, between evaluations
	// e.g. p.set_variable could return pointer to that pointer
	// not so easy for vector and tensor variables, there are many pointers to set
	// Rather modify the test to fill the
	uint n_repeats = 1000;

	ArenaAlloc arena_1(32, (25*vec_size) *sizeof(double));
	ExprData data1(arena_1, vec_size);

	Parser p(block_size);
	p.parse(expr);

	p.set_constant("cs1", {}, 	{data1.cs1});
	p.set_constant("cv1", {3}, 	std::vector<double>(data1.cv1, data1.cv1+3));
	p.set_variable("v1", {3}, data1.v1);
	p.set_variable("v2", {3}, data1.v2);
	
	p.set_variable("v3", {3}, data1.v3);
	p.set_variable("v4", {3}, data1.v4);


	//p.set_constant("a", {3,3}, std::vector<double>(data1.a, data1.a+9));
	p.set_variable("a", {3,3}, data1.a);
	
	p.set_variable("_result_", {3}, data1.vres);
	//std::cout << "vres: " << vres << ", " << vres + block_size << ", " << vres + 2*vec_size << "\n";
	//std::cout << "Symbols: " << print_vector(p.symbols()) << "\n";
	//std::cout.flush();
	p.compile();

	std::vector<uint> ss = std::vector<uint>(data1.subset, data1.subset+vec_size/4);
	p.set_subset(ss);
	auto start_time = std::chrono::high_resolution_clock::now();
	for(uint i_rep=0; i_rep < n_repeats; i_rep++) {
		p.run();
	}
	auto end_time = std::chrono::high_resolution_clock::now();
	double parser_time  =
			std::chrono::duration_cast<std::chrono::duration<double>>(end_time - start_time).count();

	for(int i = 0; i<3*1024; i++)
	{
		printf("%f\n", data1.vres[i]);
	}

	// check
	double p_sum = 0;
	for(uint dim=0; dim < 3; dim++) {
		for(uint i=0; i<data1.vec_size; i++) {
			double v1 = data1.vres[dim*data1.vec_size + i];
			p_sum += v1;
		}
	}

	std::cout << "Expression " << expr_id << ": " << expr << "\n";
	std::cout << "Result      : " << p_sum << "\n";
	std::cout << "Repeats     : " << n_repeats << "\n";
	std::cout << "parser time : " << parser_time << " s \n";
	std::cout << "parser avg. time per single execution (with vector size in mind): " << parser_time/n_repeats/vec_size*1e9 << " ns \n";
	double n_flop = n_repeats * vec_size * 9;
	std::cout << "parser FLOPS: " << n_flop / parser_time << "\n";
	std::cout << "\n";

	file << "BParser,"<< expr_id << "," << expr << "," << p_sum << "," << n_repeats << "," << parser_time << "," << parser_time/n_repeats/vec_size*1e9 << "," << n_flop/parser_time << "\n";
   
}






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

	std::cout << "Starting tests with BParser.\n";
	
	test_expr_parser("v1 + 1.1", "1", file);
	test_expr_parser("v1 + v2 + v3 + v4", "2", file);

	test_expr_parser("v1 * v1", "test1", file);
    test_expr_parser("v1**2", "test1A", file);

    test_expr_parser("v1 / 3", "test2", file);
    test_expr_parser("v1 * 3", "test2A", file);

    test_expr_parser("abs(sin(sqrt(v1**2 + v2**2)))", "test3", file);
    test_expr_parser("abs(sin(sqrt(v1)))", "test3A", file);
    test_expr_parser("v1**2 + v2**2", "test3B", file);

	test_expr_parser("v1**3", "test4A", file);
	test_expr_parser("v1**3.01","test4B", file);

	test_expr_parser("v1**2 + v2**2 + v3***2", "test5A", file);
	test_expr_parser("sqrt(v1)", "test5B", file);
	test_expr_parser("sqrt(v1**2 + v2**2 + v3***2)", "test5C", file);
	

	//determinant 3x3 matice
	test_expr_parser("cv1", "test6", file);
	test_expr_parser("a[0,0]", "test6", file);
	test_expr_parser("a[0,1]", "test6", file);
	test_expr_parser("a[0,0]*a[1,1]*a[2,2]", "test6", file);
	test_expr_parser("a[0,0]*a[1,1]*a[2,2] + a[0,1]*a[1,2]*a[2,0] + a[0,2]*a[1,0]*a[2,1] - a[2,0]*a[1,1]*a[0,2]-a[2,1]*a[1,2]*a[0,0]-a[2,2]*a[1,0]*a[0,1]", "test6A", file);
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